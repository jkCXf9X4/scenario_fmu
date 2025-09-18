

file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS "*.h" "*.hpp" "*.cpp")

# message(ALL_SRC schema_parser ="${ALL_SRC}")

add_library(scenario SHARED ${ALL_SRC})

target_include_directories(scenario
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include_private
 
)

# Copy the built shared library into the Python package's binaries folder
# to make it available for packaging into wheels.

# Detect platform folder similar to Python tools
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_ "win64")
  else()
    set(PLATFORM_ "win32")
  endif()
elseif(APPLE)
  set(PLATFORM_ "darwin64")
elseif(UNIX)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_ "linux64")
  else()
    set(PLATFORM_ "linux32")
  endif()
else()
  set(PLATFORM_ "linux64")
endif()

set(PY_BINARY_FOLDER "${CMAKE_SOURCE_DIR}/python/src/scenario_fmu_generator/_binaries/${PLATFORM_}")

add_custom_command(TARGET scenario POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PY_BINARY_FOLDER}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scenario> "${PY_BINARY_FOLDER}/$<TARGET_FILE_NAME:scenario>"
  COMMENT "Copying scenario library to ${PY_BINARY_FOLDER}"
)
