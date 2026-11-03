/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Audio.h"

#include <Arduino.h>
#include <driver/i2s_std.h>
#include <math.h>

#include "Log.h"
#include "config.h"

namespace {
    constexpr uint32_t SAMPLE_RATE = 16000;
    constexpr size_t CHUNK_SAMPLES = 256;
    constexpr uint8_t DEFAULT_GAIN_DB = 9;

    // The driver default is 6 descriptors of 240 frames. Doubling the
    // descriptor count doubles the DMA buffer to about 180 ms, so a slow
    // main-loop pass does not drain the buffer.
    constexpr uint32_t DMA_DESC_NUM = 12;
    constexpr uint32_t DMA_FRAME_NUM = 240;

    const char *tag = "Audio";

    i2s_chan_handle_t txChan = nullptr;

    enum class PinState : uint8_t {
        Input,
        OutputHigh,
        OutputLow,
    };

    bool audioReady = false;
    bool audioEnabled = false;
    uint8_t audioGainDb = DEFAULT_GAIN_DB;
    int audioLastError = 0;

    PinState gain0State = PinState::Input;
    PinState gain1State = PinState::Input;
    PinState sdState = PinState::OutputLow;

    bool toneActive = false;
    uint32_t toneSamplesRemaining = 0;
    float tonePhase = 0.0f;
    float tonePhaseIncrement = 0.0f;
    float toneAmplitude = 0.0f;

    int16_t pendingSamples[CHUNK_SAMPLES];
    int16_t stereoBuffer[CHUNK_SAMPLES * 2];
    size_t pendingCount = 0;
    size_t pendingOffset = 0;

    uint32_t bytesWritten = 0;
    uint32_t stalls = 0;

    // Drive one pin and return the state it now has.
    PinState applyPin(uint8_t pin, PinState state) {
        if (state == PinState::Input) {
            pinMode(pin, INPUT);
        } else {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, state == PinState::OutputHigh ? HIGH : LOW);
        }
        return state;
    }

    // Text for a pin state.
    const char *stateText(PinState state) {
        switch (state) {
            case PinState::OutputHigh:
                return "output high";
            case PinState::OutputLow:
                return "output low";
            default:
                return "input";
        }
    }

    // Fill the pending buffer with the next chunk of the active tone.
    void fillChunk(void) {
        size_t count = toneSamplesRemaining < CHUNK_SAMPLES ? toneSamplesRemaining : CHUNK_SAMPLES;
        for (size_t i = 0; i < count; i++) {
            pendingSamples[i] = static_cast<int16_t>(toneAmplitude * sinf(tonePhase));
            tonePhase += tonePhaseIncrement;
            if (tonePhase >= 2.0f * PI) {
                tonePhase -= 2.0f * PI;
            }
        }
        pendingCount = count;
        pendingOffset = 0;
        toneSamplesRemaining -= count;
    }

    // Write the pending samples to the DMA buffer; returns false when the buffer is full.
    bool writePending(void) {
        size_t monoRemaining = pendingCount - pendingOffset;
        for (size_t i = 0; i < monoRemaining; i++) {
            stereoBuffer[i * 2] = pendingSamples[pendingOffset + i];
            stereoBuffer[i * 2 + 1] = pendingSamples[pendingOffset + i];
        }

        size_t bytesToWrite = monoRemaining * 2 * sizeof(int16_t);
        size_t written = 0;
        esp_err_t err = i2s_channel_write(txChan, stereoBuffer, bytesToWrite, &written, 0);

        if (written == 0) {
            stalls++;
            audioLastError = err;
            return false;
        }

        pendingOffset += written / (2 * sizeof(int16_t));
        bytesWritten += written / 2;
        audioLastError = err;
        return true;
    }

    // Fill the DMA buffer with the first chunks of the tone before the main loop resumes.
    void prefill(void) {
        while (toneActive) {
            if (pendingOffset >= pendingCount) {
                if (toneSamplesRemaining == 0) {
                    toneActive = false;
                    return;
                }
                fillChunk();
            }

            if (!writePending()) {
                return;
            }

            if (pendingOffset >= pendingCount && toneSamplesRemaining == 0) {
                toneActive = false;
                return;
            }
        }
    }
}

void AudioSetup(void) {
    AudioSetEnabled(false);
    AudioSetGain(DEFAULT_GAIN_DB);

    i2s_chan_config_t chanConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chanConfig.dma_desc_num = DMA_DESC_NUM;
    chanConfig.dma_frame_num = DMA_FRAME_NUM;
    chanConfig.auto_clear = true;

    esp_err_t err = i2s_new_channel(&chanConfig, &txChan, nullptr);
    if (err != ESP_OK) {
        audioLastError = err;
        LOG_E(tag, "I2S channel failed: %d", err);
        return;
    }

    i2s_std_config_t stdConfig = {};
    stdConfig.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE);
    stdConfig.slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    stdConfig.gpio_cfg.mclk = I2S_GPIO_UNUSED;
    stdConfig.gpio_cfg.bclk = static_cast<gpio_num_t>(PIN_I2S_CLK);
    stdConfig.gpio_cfg.ws = static_cast<gpio_num_t>(PIN_I2S_WS);
    stdConfig.gpio_cfg.dout = static_cast<gpio_num_t>(PIN_I2S_DATA);
    stdConfig.gpio_cfg.din = I2S_GPIO_UNUSED;
    stdConfig.gpio_cfg.invert_flags.mclk_inv = false;
    stdConfig.gpio_cfg.invert_flags.bclk_inv = false;
    stdConfig.gpio_cfg.invert_flags.ws_inv = false;

    err = i2s_channel_init_std_mode(txChan, &stdConfig);
    if (err != ESP_OK) {
        audioLastError = err;
        LOG_E(tag, "I2S std mode failed: %d", err);
        i2s_del_channel(txChan);
        txChan = nullptr;
        return;
    }

    err = i2s_channel_enable(txChan);
    if (err != ESP_OK) {
        audioLastError = err;
        LOG_E(tag, "I2S enable failed: %d", err);
        i2s_del_channel(txChan);
        txChan = nullptr;
        return;
    }

    audioReady = true;
}

void AudioLoop(void) {
    if (!toneActive) {
        return;
    }

    if (pendingOffset >= pendingCount) {
        if (toneSamplesRemaining == 0) {
            toneActive = false;
            return;
        }
        fillChunk();
    }

    writePending();

    if (pendingOffset >= pendingCount && toneSamplesRemaining == 0) {
        toneActive = false;
    }
}

bool AudioReady(void) {
    return audioReady;
}

bool AudioEnabled(void) {
    return audioEnabled;
}

void AudioSetEnabled(bool enabled) {
    sdState = applyPin(PIN_I2S_SD, enabled ? PinState::Input : PinState::OutputLow);
    audioEnabled = enabled;
}

uint8_t AudioGainDb(void) {
    return audioGainDb;
}

bool AudioSetGain(uint8_t db) {
    switch (db) {
        case 6:
            gain0State = applyPin(PIN_I2S_GAIN0, PinState::OutputHigh);
            gain1State = applyPin(PIN_I2S_GAIN1, PinState::Input);
            break;
        case 9:
            gain0State = applyPin(PIN_I2S_GAIN0, PinState::Input);
            gain1State = applyPin(PIN_I2S_GAIN1, PinState::Input);
            break;
        case 12:
            gain0State = applyPin(PIN_I2S_GAIN0, PinState::OutputLow);
            gain1State = applyPin(PIN_I2S_GAIN1, PinState::Input);
            break;
        case 15:
            gain0State = applyPin(PIN_I2S_GAIN0, PinState::Input);
            gain1State = applyPin(PIN_I2S_GAIN1, PinState::OutputLow);
            break;
        default:
            return false;
    }

    audioGainDb = db;
    return true;
}

bool AudioPlaying(void) {
    return toneActive;
}

uint32_t AudioRemainingMs(void) {
    if (!toneActive) {
        return 0;
    }

    uint32_t samples = toneSamplesRemaining + (pendingCount - pendingOffset);
    return samples * 1000U / SAMPLE_RATE;
}

uint32_t AudioBytesWritten(void) {
    return bytesWritten;
}

uint32_t AudioStalls(void) {
    return stalls;
}

int AudioLastError(void) {
    return audioLastError;
}

bool AudioPlayTone(uint16_t freqHz, uint16_t durationMs, uint8_t amplitudePercent) {
    if (!audioReady) {
        return false;
    }

    if (freqHz < 20 || freqHz > 20000 || durationMs < 10 || durationMs > 5000 || amplitudePercent < 1 || amplitudePercent > 100) {
        return false;
    }

    tonePhase = 0.0f;
    pendingCount = 0;
    pendingOffset = 0;
    stalls = 0;
    toneSamplesRemaining = static_cast<uint32_t>(durationMs) * SAMPLE_RATE / 1000U;
    tonePhaseIncrement = 2.0f * PI * static_cast<float>(freqHz) / static_cast<float>(SAMPLE_RATE);
    toneAmplitude = 32767.0f * static_cast<float>(amplitudePercent) / 100.0f;
    toneActive = true;
    prefill();
    return true;
}

const char *AudioGain0State(void) {
    return stateText(gain0State);
}

const char *AudioGain1State(void) {
    return stateText(gain1State);
}

const char *AudioSdState(void) {
    return stateText(sdState);
}
