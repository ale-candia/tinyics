cmake_minimum_required(VERSION 3.25.1)
project("icsim")

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

#### Sub directories ####

add_subdirectory(external/ns-3)
add_subdirectory(external/pybind11)
add_subdirectory(bindings)
add_subdirectory(sandbox)   # To use library with cpp

#### Build ICS-SIM library ####

set(source_files
    src/industrial-network-builder.cc
    src/industrial-process.cc
    src/modbus.cc
    src/plc-application.cc
    src/scada-application.cc
    src/utils.cc
    src/modbus-command.cc
    src/modbus-request.cc
    src/modbus-response.cc
)

set(lib_name "icssim")

add_library(${lib_name} SHARED ${source_files})

add_dependencies(${lib_name} libcore libcsma libinternet libnetwork)

target_include_directories(${lib_name} PRIVATE
    external/ns-3/build/include/
    src/
)

target_link_directories(${lib_name} PRIVATE external/ns-3/build/lib/)

target_link_libraries(${lib_name}
    libcore
    libcsma
    libinternet
    libnetwork
)

