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

add_subdirectory(3rdParty)

add_subdirectory(libs)

