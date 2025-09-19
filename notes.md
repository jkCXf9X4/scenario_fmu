scenario



nm -D --defined-only libyourfmu.so



objdump -TC ./0004_Atmos/binaries/linux64/atmos.so

objdump -TC ./scenario/binaries/linux64/scenario.so


nm -D --defined-only ./0004_Atmos/binaries/linux64/atmos.so

nm -D --defined-only ./scenario/binaries/linux64/scenario.so


ldd ./0004_Atmos/binaries/linux64/atmos.so
ldd ./scenario/binaries/linux64/scenario.so


readelf --version-info ./0004_Atmos/binaries/linux64/atmos.so
readelf --version-info ./scenario/binaries/linux64/scenario.so


objdump -p ./scenario/binaries/linux64/scenario.so | grep SONAME