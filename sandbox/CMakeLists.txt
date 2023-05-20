add_executable(example main.cc)

target_link_libraries(example PRIVATE icssim)

target_link_directories(example PRIVATE ${CMAKE_SOURCE_DIR}/build)

target_include_directories(example PRIVATE
    ${CMAKE_SOURCE_DIR}/external/ns-3/build/include
    ${CMAKE_SOURCE_DIR}/src
)

#### Cleaning up pcap files ####

add_custom_target(clean-pcap COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/sandbox/*.pcap)

