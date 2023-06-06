include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/python/
    ${CMAKE_SOURCE_DIR}/src/icsim
    ${CMAKE_SOURCE_DIR}/external/ns-3/build/include/
    ${CMAKE_SOURCE_DIR}/external/pybind11/include
)

link_libraries(icssim)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/bindings)

pybind11_add_module(industrial_networks python/ics.cc)

# Set the output directory for the specific target
set_target_properties(industrial_networks PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/bindings
)

