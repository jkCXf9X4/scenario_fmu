
git submodule add git@github.com:modelica/Reference-FMUs.git 3rdParty/reference_fmu


View th sysmbols:
- objdump -TC build/libs/scenario_fmu/libscenario.so


verify debug info:
- readelf -S build/libs/scenario_fmu/libscenario.so | rg '\.debug_'

