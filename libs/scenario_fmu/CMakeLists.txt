

file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS "*.h" "*.hpp" "*.cpp")

# message(ALL_SRC schema_parser ="${ALL_SRC}")

add_library(scenario SHARED ${ALL_SRC})

target_include_directories(scenario
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include_private
 
)
