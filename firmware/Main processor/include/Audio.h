/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include <stdint.h>

// Initialize the I2S amplifier, mute it and select the default gain.
void AudioSetup(void);

// Write the next chunk of the active tone, if any.
void AudioLoop(void);

// True when the I2S peripheral started.
bool AudioReady(void);

// True when the amplifier is enabled.
bool AudioEnabled(void);

// Enable or disable the amplifier through the SD pin.
void AudioSetEnabled(bool enabled);

// Selected gain in dB.
uint8_t AudioGainDb(void);

// Select a gain of 6, 9, 12 or 15 dB; returns false for any other value.
bool AudioSetGain(uint8_t db);

// True while a tone is playing.
bool AudioPlaying(void);

// Milliseconds of audio left in the active tone.
uint32_t AudioRemainingMs(void);

// Total bytes written to the I2S peripheral.
uint32_t AudioBytesWritten(void);

// Number of writes that returned zero bytes.
uint32_t AudioStalls(void);

// Last I2S error, or zero when the last write succeeded.
int AudioLastError(void);

// Start a sine tone; returns false when the arguments are out of range or I2S is not ready.
bool AudioPlayTone(uint16_t freqHz, uint16_t durationMs, uint8_t amplitudePercent);

// State of the GAIN0 pin as "input", "output high" or "output low".
const char *AudioGain0State(void);

// State of the GAIN1 pin as "input", "output high" or "output low".
const char *AudioGain1State(void);

// State of the SD pin as "input" or "output low".
const char *AudioSdState(void);
