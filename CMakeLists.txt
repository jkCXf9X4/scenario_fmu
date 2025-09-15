cmake_minimum_required(VERSION 3.10)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" projectVersion)
project(scenario_project VERSION ${projectVersion})

message("Current scenario_fmu version: ${projectVersion}\n")

message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}\n")
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}\n")

message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}\n")
message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}\n")
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}\n")

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs)

# Tests
include(CTest)

add_subdirectory(test)

# FMU packaging target
find_package(Python3 COMPONENTS Interpreter)

# Number of outputs to generate in modelDescription.xml (integer)
set(SCENARIO_FMU_OUTPUTS 1000 CACHE STRING "Number of real outputs in modelDescription.xml")

if(Python3_Interpreter_FOUND)
  add_custom_target(package_fmu
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/package_fmu.py
            --build-dir ${CMAKE_BINARY_DIR}
            --out ${CMAKE_BINARY_DIR}/scenario.fmu
            --model-id scenario
            --model-name ScenarioFMU
            --num-outputs ${SCENARIO_FMU_OUTPUTS}
    DEPENDS scenario
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packaging Scenario FMU into ${CMAKE_BINARY_DIR}/scenario.fmu"
    VERBATIM
  )
else()
  message(STATUS "Python3 not found: 'package_fmu' target will be unavailable")
endif()
