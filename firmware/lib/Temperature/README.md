# Temperature

Temperature scales that convert to and from Kelvin. The library is Arduino-free and has no dependencies.

## Usage

```cpp
#include "Temperature.h"

const temperature::Unit *unit = temperature::find("fahrenheit");
float kelvin = unit->toKelvin(212.0f);
float fahrenheit = unit->fromKelvin(373.15f);
```

`temperature::units()` returns the table and `temperature::unitCount()` its length. `temperature::celsius()` returns the default unit. An unknown id returns `nullptr`.

## Units

| Id           | Name       | Symbol | Step | Direction | Decimals | Common |
| ------------ | ---------- | ------ | ---- | --------- | -------- | ------ |
| `celsius`    | Celsius    | °C     | 10   | 1         | 1        | yes    |
| `fahrenheit` | Fahrenheit | °F     | 18   | 1         | 1        | yes    |
| `kelvin`     | Kelvin     | K      | 10   | 1         | 1        | yes    |
| `rankine`    | Rankine    | °Ra    | 18   | 1         | 1        | no     |
| `reaumur`    | Réaumur    | °Ré    | 8    | 1         | 1        | no     |
| `romer`      | Rømer      | °Rø    | 5.25 | 1         | 1        | no     |
| `delisle`    | Delisle    | °De    | 15   | -1        | 1        | no     |
| `newton`     | Newton     | °N     | 3.3  | 1         | 1        | no     |
| `leiden`     | Leiden     | °L     | 10   | 1         | 1        | no     |
| `wedgwood`   | Wedgwood   | °W     | 0.14 | 1         | 2        | no     |
| `gasmark`    | Gas mark   | GM     | 0.5  | 1         | 2        | no     |
| `thermostat` | Thermostat | Th     | 0.33 | 1         | 1        | no     |
| `stufe`      | Stufe      | St     | 0.5  | 1         | 1        | no     |

`step` is the size of a 10 C step in the unit. `direction` is 1 when the value rises with temperature and -1 for Delisle. `decimals` is the display precision. `common` marks the units that a UI shows first.

Gas mark is a discrete oven dial. The library interpolates linearly between the marks of the published conversion table, and extrapolates with the first or last segment outside the table.

## Adding a unit

1. Add a `toKelvin` and `fromKelvin` function pair with the source URL above it.
2. Add one row to `UNITS` in `src/Temperature.cpp`.
3. Add the reference points to `test/test_temperature.cpp`.

## Test

```
g++ -std=c++17 -Wall -Wextra -I src test/test_temperature.cpp src/Temperature.cpp -o /tmp/test_temperature && /tmp/test_temperature
```

## License

Apache-2.0. See `LICENSE`.
