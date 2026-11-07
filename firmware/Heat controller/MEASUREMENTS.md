# Handle measurements

The heat controller publishes the handle measurements over I2C. This document records the measured values for each physical situation and the rules that classify the situation.

## Method

The operator creates one situation at a time. The main processor `sensor` command reads the values over I2C. Each situation is one row in the measurement log.

## Measurement log

| Situation | Heater | 24V | TIP_TEMP_C | NTC_TEMP_C | NTC_V | THERMOCOUPLE_MV | VIN_OFF_V | VIN_ON_V | IRON_CURRENT_OFF_A | IRON_CURRENT_ON_A | RTIP_OHM | RSUPPLY_OHM |
| --------- | ------ | --- | ---------- | ---------- | ----- | --------------- | --------- | -------- | ------------------ | ----------------- | -------- | ----------- |
| No handle | off    | off | nan        | nan        | 3.32275 | 0.708         | -0.01     | 0.00     | 0.021              | 0.000             | nan      | nan         |
| No handle | off    | on  | nan        | nan        | 3.32262 | 29.367        | 24.13     | 0.00     | 0.021              | 0.000             | nan      | nan         |
| Handle with tip | off | on | 21.65 | 21.07 | 1.79562 | 0.009 | 24.13 | 0.00 | 0.022 | 0.000 | nan | nan |
| Handle with tip | off | off | 21.68 | 21.10 | 1.79450 | 0.009 | -0.01 | 0.00 | 0.022 | 0.000 | nan | nan |
| Handle without tip | off | off | 102.55 | 21.20 | 1.79062 | 1.439 | -0.01 | 0.00 | 0.021 | 0.000 | nan | nan |
| Handle without tip | off | on | 1402.45 | 21.37 | 1.78437 | 29.459 | 24.13 | 0.00 | 0.021 | 0.000 | nan | nan |
| Handle without tip | on | on | 1392.56 | 21.66 | 1.77363 | 29.279 | 24.13 | 0.00 | 0.021 | 0.000 | nan | nan |
| Handle with tip | on | on | 100.37 | 22.08 | 1.75800 | 1.381 | 24.12 | 23.18 | 0.021 | 2.939 | 7.89 | 0.32 |
| Handle with tip | on | off | 57.88 | 22.40 | 1.74588 | 0.601 | -0.01 | -0.00 | 0.021 | 0.021 | -0.13 | 0.00 |
| Handle with tip | on | on | 350.12 | 23.05 | 1.72187 | 6.710 | 24.13 | 23.16 | 0.021 | 2.899 | 7.99 | 0.34 |
| Tip not seated | off | off | 93.14 | 19.53 | 1.85337 | 1.292 | -0.01 | 0.00 | 0.021 | 0.000 | nan | nan |
| Tip not seated | off | on | 1398.92 | 19.37 | 1.85925 | 29.432 | 24.12 | 0.00 | 0.021 | 0.000 | nan | nan |
| Tip not seated | on | on | 1393.31 | 19.35 | 1.86012 | 29.333 | 24.11 | 0.00 | 0.021 | 0.000 | nan | nan |
| Handle with tip (seated) | off | on | 24.72 | 21.79 | 1.76888 | 0.048 | 24.12 | 0.00 | 0.021 | 0.000 | nan | nan |
| Handle with tip (seated) | on | on | 100.03 | 21.49 | 1.77988 | 1.386 | 24.12 | 23.18 | 0.021 | 2.903 | 7.99 | 0.32 |
| Handle with tip | on | on | 30.01 | 21.92 | 1.76375 | 0.133 | 24.13 | 0.00 | 0.021 | 0.000 | nan | nan |
| Heat controller absent | - | - | -273.15 | -272.40 | 0.75275 | 752.748 | 0.75 | 0.75 | 0.753 | 0.753 | 0.75 | 0.75 |

## Notes

Without a tip the thermocouple input floats, so `THERMOCOUPLE_MV` is not a fault indicator in that state.

A tip that is not seated shows a slowly rising and falling `TIP_TEMP_C` with a cold tip while 24V and the heater are off. The thermocouple signal is unstable. Pressing the tip well restores a stable reading.

When the heat controller is absent, the main processor reads 0 K, so `TIP_TEMP_C` and `NTC_TEMP_C` show `-273.15` C. This is a link-loss state.

## Classification

The main processor evaluates the states in this order. The first match wins.

| Order | State | Condition | Line 1 | Line 2 |
| ----- | ----- | --------- | ------ | ------ |
| 1 | `LOSS` | `linkUp()` is false | `HW fault` | `Heat ctrl absent` |
| 2 | `NO_HANDLE` | `NTC_V > 2.5` | `No handle` | `Connect T12 handle` |
| 3 | `24V-off` | `VIN_OFF_V < 5` | `24V is off` | `Switch 24V on` |
| 4 | `NO_TIP` | `THERMOCOUPLE_MV / 1000 * gain > 1.5`, or heater on for 1 s AND `duty > 0.2` AND `IRON_CURRENT_ON_A < 0.1` | `No T12 tip` | `Press tip in firmly` |
| 5 | `OK` | none of the above | tip temperature | `SET`/`PWM` |

The 30 C row shows why the current check needs `duty > 0.2`: the duty is 0.171, so the heat controller does not refresh `IRON_CURRENT_ON_A`, and it reads 0.000 on a good tip.

The thermocouple check does not need the heater. The "Handle without tip, off, on" row reads 3.13 V raw, so `NO_TIP` appears before the user starts heating. The current check still needs the heater, because the heat controller refreshes `IRON_CURRENT_ON_A` only while it drives the tip. The check waits 1 s after the heater turns on, because the first frame still holds the 0.000 from the off state.

## Verification

The main processor `sensor` command reports these states for the tested situations.

| Situation | `HANDLE_STATE` | Key value |
| --- | --- | --- |
| 24V off, heater on, tip seated | `SUPPLY_OFF` | `VIN_OFF_V: -0.01` |
| 24V on, heater on, tip seated | `OK` | `RTIP_OHM: 7.97` |
| Handle unplugged | `NO_HANDLE` | `NTC_V: 3.32175` |
| Handle without tip, heater on | `NO_TIP` | `THERMOCOUPLE_MV: 29.495` |
| Heat controller in reset | `LOSS` | `LINK: LOSS` |

The `NO_TIP` state comes from the thermocouple rule. The PID sees 1403 C and sets the duty to 0, so the heat controller does not refresh `IRON_CURRENT_ON_A` and the current rule does not fire.
