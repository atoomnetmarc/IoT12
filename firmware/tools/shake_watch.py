#!/usr/bin/env python3
"""Watch the Heat controller SHAKE input over the serial CLI."""

import argparse
import time

import serial


def read_values(port, interval):
    port.write(b"shake\r\n")
    port.flush()
    deadline = time.time() + interval
    values = {}
    while time.time() < deadline:
        line = port.readline().decode("ascii", errors="replace").strip()
        if ":" not in line:
            continue
        key, _, value = line.partition(":")
        values[key.strip()] = value.strip()
        if key.strip() == "OFF_MS":
            break
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/ttyACM0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--interval", type=float, default=0.5)
    args = parser.parse_args()

    with serial.Serial(args.port, args.baud, timeout=0.1) as port:
        port.dtr = True
        time.sleep(0.2)
        port.reset_input_buffer()

        last = None
        while True:
            values = read_values(port, args.interval)
            if not values:
                print("no response", flush=True)
                continue
            current = (
                values.get("RAW"),
                values.get("DEBOUNCED"),
                values.get("PHASE"),
                values.get("LAST_MOTION_MS"),
            )
            if current != last:
                print(
                    "RAW={} DEBOUNCED={} PHASE={} LAST_MOTION_MS={} TARGET_C={}".format(
                        current[0], current[1], current[2], current[3], values.get("TARGET_C")
                    ),
                    flush=True,
                )
                last = current


if __name__ == "__main__":
    main()
