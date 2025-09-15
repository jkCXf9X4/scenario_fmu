# Scenario FMU

## Note

This builds an FMU (https://fmi-standard.org/) that outputs the equivalent of a csv table

The reason is to enable the inclusion of synthetic data within an SSP (https://ssp-standard.org/) without relying on the importing tool.
This will provide higher repeatability when migrating between tools 


## Input 
The input is a string of values that corresponds to a csv or equivalent, this is specified as an parameter for the fmu (scenario_input). 

- The first one should always specify time.
- All values will be parsed as doubles
- It is acceptable to leave fields empty, the last known value will then be utilized


Example parameter value specifying input
```
"[0;0;0][1;4;5][2;3;3][2.01;;4][3;3;3]"
```

TODO: add support for alternative representation

```
"[[t1.1;v1.1;d1.1;l1.1][t1.2;v1.2;d1.2;l1.2]][[t2.1;v2.1;d2.1;l2.1][t2.2;v2.2;d2.2;l2.2]]"
```

t: time, start
v: value, start
d: derivative
l: length

x,y = x:variable y:timestep

## Interpolation possibilities
```
interpolation  = "[L;L;ZOH]"
```
- L: Linear
- C: Cubic
- ZOH: Zero order hold
- NN: Nearest Neighbor

## Time

Take a step to the time where the output values should be valid

```
fmi2Status fmi2DoStep(fmi2Component c, ...)
```

## Output

parse all the outputs with the fmiGetReal function. 


Value references will be the sequential number in the input order.

```
fmiGetReal(fmi2Component c, fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
```

The model description will contain 1000 outputs but this is not a hard limit, any number of outputs can be used

TODO: Add a graph of the result

# Build

## Setup

Uses vcpkg for dependencies

```
git submodule init && git submodule update
cmake --preset=vcpkg
```

## FMU Packaging

You can package a complete FMI 2.0 Co‑Simulation FMU containing `modelDescription.xml` and the
shared library, either via the provided CMake target or the Python helper script.

- Using CMake target (recommended)
  - Configure and build:
    - `cmake --preset=vcpkg`
    - `cmake --build build -j`
  - Package FMU into `build/scenario.fmu`:
    - `cmake --build build --target package_fmu`
  - Control number of outputs in `modelDescription.xml` (default 1000):
    - `cmake -S . -B build -DSCENARIO_FMU_OUTPUTS=250`
    - then `cmake --build build --target package_fmu`

- Using the Python script directly
  - Command:
    ```
    ./scripts/package_fmu.py
    ```
see --help for input variations, 

  - The script generates `modelDescription.xml`, detects platform (`binaries/linux64`, `darwin64`,
    `win32`/`win64`) and copies the built shared library (`libscenario.so`/`libscenario.dylib`/
    `scenario.dll`). Use `--guid <uuid>` to fix the GUID (random by default).

## Tests


Build and run tests
```
cmake --build build && ./build/test/scenario_tests
```

Build and inspect .so (tested on ubuntu 22)
```
cmake --build build && objdump -TC ./build/libs/scenario_fmu/libscenario.so | grep " g    DF"
```

## Run with FMPy

Use the helper to run the FMU with FMPy, capture a CSV, and optionally a plot.

- Requirements: `pip install fmpy matplotlib`
- Example:
  ```
  ./scripts/run_fmu.py
  ```
see --help for input variations, default is to run the default build

- Outputs:
  - CSV: `build/run/results.csv`
  - Plot: `build/run/results.png` (if matplotlib available)
