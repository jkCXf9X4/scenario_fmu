# Scenario FMU

## Note

This builds an FMU (https://fmi-standard.org/) that outputs the equivalent of a csv table

The reason is to enable the inclusion of synthetic data within an SSP (https://ssp-standard.org/) without relying on the importing tool.
This will provide higher repeatability when migrating between tools 


## Input 
The input is a string of values that corresponds to a csv or equivalent, this is specified as an parameter for the fmu (scenario_input). 

The first one should always specify time.
All values will be parsed as doubles
It is acceptable to leave fields empty, the last known value will then be utilized

Example parameter value specifying input
```
"[0;0;0][1;4;5][2;3;3][2.01;;4][3;3;3]"
```

## Interpolation possibilities
```
interpolation  = "[;L;]"
```
L: Linear
C: Cubic
ZOH: Zero order hold
NN: Nearest Neighbor

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

# Build

Uses vcpkg for dependencies

```
git submodule init && git submodule update

cmake --preset=vcpkg

cmake --build build
cmake --build build &> build.log
```

Build and run tests
```
cmake --build build && ./build/test/scenario_tests
```

Build and inspect .so (tested on ubuntu 22)
```
cmake --build build && objdump -TC ./build/libs/scenario_fmu/libscenario.so | grep " g    DF"
```

# Credits
