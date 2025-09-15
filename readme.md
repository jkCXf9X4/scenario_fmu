# Scenario FMU

## Note




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

Build and inspect .so
```
cmake --build build && objdump -TC ./build/libs/scenario_fmu/libscenario.so | grep " g    DF"
```

# Credits
