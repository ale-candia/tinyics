cmake_minimum_required(VERSION 3.25.1)
project("ics-sim")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#### Debug and Release Versions ####

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

#### Build IOT-SIM Project ####

add_subdirectory(external/ns-3)

file(GLOB SourceFiles ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(${PROJECT_NAME} ${SourceFiles})

target_include_directories(${PROJECT_NAME}
    PRIVATE external/ns-3/build/include/
    ${PROJECT_SOURCE_DIR}/src/
)

target_link_directories(${PROJECT_NAME} PRIVATE external/ns-3/build/lib/)

target_link_libraries(${PROJECT_NAME}
    ns3-dev-core
    ns3-dev-csma
    ns3-dev-internet
    ns3-dev-network
)

#### Cleaning up pcap files ####

add_custom_target(clean-pcap COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.pcap)

add_custom_target(clean-all
  make clean
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.pcap
)

