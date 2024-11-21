

file(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS "*.hpp" "*.cpp")

# message(ALL_SRC schema_parser ="${ALL_SRC}")

add_library(scenario ${ALL_SRC})

target_include_directories(scenario
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    
    PRIVATE
    ${REFERENCE_FMU_DIR}/src
    ${REFERENCE_FMU_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include_private
 
)

# target_compile_options(scenario PRIVATE -Wfatal-errors)

find_package(boost_log CONFIG REQUIRED)
target_link_libraries(scenario PRIVATE Boost::log)

find_package(libzip CONFIG REQUIRED)
target_link_libraries(scenario PRIVATE libzip::zip)

find_package(pugixml CONFIG REQUIRED)
target_link_libraries(scenario PRIVATE pugixml::static pugixml::pugixml)

