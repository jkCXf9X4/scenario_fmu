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

- `scenario-fmu-package --help`

