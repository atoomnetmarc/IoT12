/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "AudioCommands.h"

#include <cstdlib>
#include <cstring>

#include "Audio.h"
#include "CliFormat.h"

namespace {
    constexpr uint32_t SAMPLE_RATE = 16000;

    // Print the amplifier state.
    void printAudio(cli::SerialCli &cli) {
        cli_format::keyText(cli, "I2S", AudioReady() ? "ready" : "error");
        cli_format::keyText(cli, "ENABLED", AudioEnabled() ? "yes" : "no");
        cli_format::keyUInt(cli, "GAIN_DB", AudioGainDb());
        cli_format::keyText(cli, "GAIN0", AudioGain0State());
        cli_format::keyText(cli, "GAIN1", AudioGain1State());
        cli_format::keyText(cli, "SD", AudioSdState());
        cli_format::keyUInt(cli, "SAMPLE_RATE", SAMPLE_RATE);
        cli_format::keyText(cli, "PLAYING", AudioPlaying() ? "yes" : "no");
        cli_format::keyUInt(cli, "REMAINING_MS", AudioRemainingMs());
        cli_format::keyUInt(cli, "BYTES_WRITTEN", AudioBytesWritten());
        cli_format::keyUInt(cli, "STALLS", AudioStalls());
        if (AudioLastError() == 0) {
            cli_format::keyText(cli, "LAST_ERROR", "none");
        } else {
            cli_format::keyInt(cli, "LAST_ERROR", AudioLastError());
        }
    }

    // Print the gain pin states.
    void printPins(cli::SerialCli &cli) {
        cli_format::keyText(cli, "GAIN0", AudioGain0State());
        cli_format::keyText(cli, "GAIN1", AudioGain1State());
        cli_format::keyText(cli, "SD", AudioSdState());
        cli_format::keyUInt(cli, "GAIN_DB", AudioGainDb());
        cli_format::keyText(cli, "ENABLED", AudioEnabled() ? "yes" : "no");
    }

    // Check the mute guard; returns true when playback may start.
    bool playbackAllowed(cli::SerialCli &cli, const char *command) {
        if (!AudioEnabled()) {
            cli.writef("%s: amplifier is muted, use 'audio unmute'\r\n", command);
            return false;
        }

        return true;
    }

    // Start a tone and print its parameters.
    void playTone(cli::SerialCli &cli, uint16_t freqHz, uint16_t durationMs, uint8_t amplitudePercent) {
        if (!AudioPlayTone(freqHz, durationMs, amplitudePercent)) {
            cli.write("audio tone: not available (I2S not ready)\r\n");
            return;
        }

        cli.writef("TONE: %u Hz, %u ms, %u%%\r\n", freqHz, durationMs, amplitudePercent);
        cli_format::keyUInt(cli, "GAIN_DB", AudioGainDb());
    }

    // Handle `audio tone <freq> <ms> [<amplitude>]`.
    void cmdTone(cli::SerialCli &cli, int argc, char **argv) {
        if (argc < 4) {
            cli.write("Usage: audio tone <freq 20-7000> <ms 10-5000> [<amplitude 1-100>]\r\n");
            return;
        }

        int freq = atoi(argv[2]);
        int duration = atoi(argv[3]);
        int amplitude = 10;
        if (argc >= 5) {
            amplitude = atoi(argv[4]);
        }

        if (freq < 20 || freq > 7000 || duration < 10 || duration > 5000 || amplitude < 1 || amplitude > 100) {
            cli.write("Usage: audio tone <freq 20-7000> <ms 10-5000> [<amplitude 1-100>]\r\n");
            return;
        }

        if (!playbackAllowed(cli, "audio tone")) {
            return;
        }

        playTone(cli, static_cast<uint16_t>(freq), static_cast<uint16_t>(duration), static_cast<uint8_t>(amplitude));
    }

    // Handle the `audio` command and its subcommands.
    void cmdAudio(cli::SerialCli &cli, int argc, char **argv) {
        if (argc >= 2 && strcmp(argv[1], "gain") == 0) {
            if (argc < 3) {
                cli.write("Usage: audio gain <6|9|12|15>\r\n");
                return;
            }

            int db = atoi(argv[2]);
            if (db < 0 || db > 255 || !AudioSetGain(static_cast<uint8_t>(db))) {
                cli.write("Usage: audio gain <6|9|12|15>\r\n");
                return;
            }

            cli_format::keyUInt(cli, "GAIN_DB", AudioGainDb());
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "mute") == 0) {
            AudioSetEnabled(false);
            cli_format::keyText(cli, "ENABLED", "no");
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "unmute") == 0) {
            AudioSetEnabled(true);
            cli_format::keyText(cli, "ENABLED", "yes");
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "test") == 0) {
            if (!playbackAllowed(cli, "audio test")) {
                return;
            }

            playTone(cli, 1000, 200, 10);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "tone") == 0) {
            cmdTone(cli, argc, argv);
            return;
        }

        if (argc >= 2 && strcmp(argv[1], "pins") == 0) {
            printPins(cli);
            return;
        }

        if (argc >= 2) {
            cli.write("Usage: audio [gain <6|9|12|15>|mute|unmute|test|tone <freq> <ms> [<amplitude>]|pins]\r\n");
            return;
        }

        printAudio(cli);
    }
}

namespace audio_commands {
    void registerCommands(cli::SerialCli &cli) {
        cli.registerCommand({"audio", [&cli](int argc, char **argv) { cmdAudio(cli, argc, argv); }, "audio amplifier: state, gain <6|9|12|15>, mute|unmute, test, tone <freq> <ms> [<amplitude>], pins"});
    }
}
