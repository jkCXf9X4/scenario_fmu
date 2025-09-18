# Scenario FMU

## Note

This builds an FMU (https://fmi-standard.org/) that outputs the equivalent of a csv table. 
The input is coordinates and the interpolation method that is to be applied

The reason is to enable the inclusion of synthetic data within an SSP (https://ssp-standard.org/) without relying on the importing tool.
This will provide higher repeatability when migrating between tools 

Use case:
- build scenario fmu with custom connectors for a specific model or use a generic model with generic connections y1, y2,...
- attach it to an ssp
- create parameter sets to capture alterations in the scenario or use the default if applicable

Easiest utilized with the python package

## Input 
The input is a string of values that corresponds to a csv or equivalent, this is specified as an parameter for the fmu (scenario_input). 

- All values will be parsed as doubles

An fmu parameter value is used to handle the input to the model.
Input, where t is time and v is the variables. Enter (\n) is used as separator between variables
```
name_1;interpolation_method_1;t_0,var_1.0;t_1,var_1.2
name_2;interpolation_method_2;t_0,var_2.0;t_3,var_2.2
...
```

for example:
```
var1;L;1,0;3,0.5;5,4;9,2
var2;ZOH;2,0;3,0.5;5,4;9,2
var2;NN;0,0;1,0.5;2,4;3,2
```

### Interpolation methods

Each one corresponds to the same position in the list of parameters
Parameter value specifying interpolation

- L: Linear
- C: Cubic (Not supported yet)
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
const fmi2ValueReference vr_in[1] = {0};
const fmi2String values[1] = {"var1;L;1,0;3,0.5;5,4;9,2\nvar2;ZOH;2,0;3,0.5;5,4;9,2\nvar2;NN;0,0;1,0.5;2,4;3,2"};

fmi2SetString(comp, vr_in, 1, values)

fmi2EnterInitializationMode(comp)
fmi2ExitInitializationMode(comp)

// Take a step to the time and get the values
const fmi2ValueReference vr_out[3] = {1, 2, 3};
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


## Python Tools (Packaging, SSV generation)

You can:
- package a complete FMI 2.0 Co‑Simulation FMU containing `modelDescription.xml` and the shared library via the installed CLI
- Generate custom parameter sets for the fmus

More info in the [python package readme](./python/README.md)

The Python utilities live in `python/` and can be installed locally:
```
# one‑time local install of Python tools
`pip install -e ./python`
```

## Run with FMPy

Simple script to run the FMU with FMPy, capture a CSV, and optionally a plot if matplotlib is availible.
build and package before executing

```
./scripts/run_fmu.py
```

- Outputs:
  - CSV: `reference_results/results.csv` 
  - Plot: `reference_results/results.png`


