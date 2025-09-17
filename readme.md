# Scenario FMU

## Note

This builds an FMU (https://fmi-standard.org/) that outputs the equivalent of a csv table

The reason is to enable the inclusion of synthetic data within an SSP (https://ssp-standard.org/) without relying on the importing tool.
This will provide higher repeatability when migrating between tools 


## Input 
The input is a string of values that corresponds to a csv or equivalent, this is specified as an parameter for the fmu (scenario_input). 

- The first one should always specify time.
- All values will be parsed as doubles
- It is acceptable to leave fields empty

Parameter value specifying input
```
Input, where t is time and v is the variables
[t0, v1.1, v2.1, ...][t1, v1.2, v2.2, ...][...]

for example:
[0;0;0][1;4;5][2;3;3][2.5;;4][3;3;3]
```

## Interpolation possibilities

List of interpolation methods, each one corresponds to the same position in the list of parameters
Parameter value specifying interpolation
```
[L;L;ZOH]
```
- L: Linear
- C: Cubic
- ZOH: Zero order hold
- NN: Nearest Neighbor

### TODO: add support for alternative representation

```
"[[t1.1;v1.1;d1.1;l1.1][t1.2;v1.2;d1.2;l1.2]][[t2.1;v2.1;d2.1;l2.1][t2.2;v2.2;d2.2;l2.2]]"
```

t: time, start
v: value, start
d: derivative
l: length

x,y = x:variable y:timestep

## Execution


Value references will be the sequential number in the input order. 1 and 2 are used for input
```
// Setup
const fmi2ValueReference vr_in[2] = {0, 1};
const fmi2String values[2] = {"[0;0;0][1;4;5][2;3;3][2.5;;4][3;3;3]", "[L;L;ZOH]"};

fmi2SetString(comp, vr_in, 2, values)

fmi2EnterInitializationMode(comp)
fmi2ExitInitializationMode(comp)

// Take a step to the time and get the values
const fmi2ValueReference vr_out[3] = {2, 3, 4};
fmi2Real out_vals[3] = {0.0, 0.0, 0.0};
fmi2Status fmi2DoStep(fmi2Component c, ...)
fmiGetReal(fmi2Component c, fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
```

# Build

## Setup

Uses vcpkg for dependencies, https://vcpkg.io/en/

```
# Initialize submodules
git submodule init && git submodule update

# Set build config
cmake --preset=vcpkg

# Define build type
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug'
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Clear everything and start from the beginning
rm -rf build
```


## Build library

```
cmake --build build -j
```

## FMU Packaging

You can package a complete FMI 2.0 Co‑Simulation FMU containing `modelDescription.xml` and the
shared library via the Python helper script.

Package FMU into `build/scenario.fmu`:
```
./scripts/package_fmu.py
```
see --help for input options for the scripts

The script generates `modelDescription.xml`, detects platform (`binaries/linux64`, `darwin64`,
`win32`/`win64`) and copies the built shared library (`libscenario.so`/`libscenario.dylib`/
`scenario.dll`). Use `--guid <uuid>` to fix the GUID (random by default).


## Tests

Build and run tests
```
cmake --build build && ./build/test/scenario_tests
cmake --build build && ctest --test-dir build -V
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

# build, package and run
cmake --build build && ./scripts/package_fmu.py && ./scripts/run_fmu.py
```
see --help for input variations, default is to run the default build

- Outputs:
  - CSV: `build/run/results.csv`
  - Plot: `build/run/results.png` (if matplotlib available)
