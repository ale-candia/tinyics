include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/python/
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/external/ns-3/build/include/
    ${CMAKE_SOURCE_DIR}/external/pybind11/include
)

link_libraries(icssim)

pybind11_add_module(industrial_networks python/ics.cc)

