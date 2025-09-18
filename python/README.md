# scenario-fmu-generator

Utilities to package and run Scenario FMUs (FMI 2.0 Co-Simulation).

- `scenario-fmu-package`: Create a valid `.fmu` archive from the built shared library.

Use case:
- build scenario fmu with custom connectors for a specific model or use a generic model with generic connections y1, y2,...
- attach it to an ssp
- create parameter sets to capture alterations in the scenario

## Install

```
pip install scenario-fmu-generator
```

## CLI

### package FMU

This generates `modelDescription.xml`, detect platform (`binaries/linux64`, `darwin64`,
`win32`/`win64`) and copy the built shared library (`libscenario.so`/`libscenario.dylib`/
`scenario.dll`).

```
# Default generic fmu
scenario-fmu-package --out scenario.fmu

# Custom fmu with default behavior
scenario-fmu-package --out ./build/scenario.fmu -s "var1; L; 1,0; 3,0.5; 5,4; 9,2
var2; ZOH; 2,0; 3,0.5; 5,4; 9,2
var3; NN; 0,0; 1,0.5; 2,4; 3,2"
```

### Build the ssv

Create an SSP parameter set to be used with the scenario fmu:

```
scenario-fmu-ssv --out ./build/scenario.ssv -s "var1; L; 1,2; 3,0.5; 4,3; 7,2
var2; ZOH; 2,2; 3,0.5; 5,5; 9,2
var3; NN; 0,0; 1,0.5; 2,4; 3,2"
```

## Python Tools (Packaging & CLI)

To build distributable (wheel/sdist) for publishing:

```
cd python
python -m build
```

By default, CMake copies the built shared library into the Python package ensuring its available for the build.
