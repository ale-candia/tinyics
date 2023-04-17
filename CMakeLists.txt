cmake_minimum_required(VERSION 3.25.1)
project("iotsim")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SourceFiles ${PROJECT_SOURCE_DIR}/src/*.cc)
add_executable(${PROJECT_NAME} ${SourceFiles})

target_include_directories(${PROJECT_NAME} PRIVATE /Users/ale/Dev/ns-3/build/include/ ${PROJECT_SOURCE_DIR}/src/)
target_link_directories(${PROJECT_NAME} PRIVATE /Users/ale/Dev/ns-3/build/lib)

target_link_libraries(${PROJECT_NAME}
    ns3-dev-applications-debug
    ns3-dev-core-debug
    ns3-dev-csma-debug
    ns3-dev-csma-layout-debug
    ns3-dev-internet-debug
    ns3-dev-network-debug
    ns3-dev-nix-vector-routing-debug
    ns3-dev-topology-read-debug
    ns3-dev-traffic-control-debug
)
