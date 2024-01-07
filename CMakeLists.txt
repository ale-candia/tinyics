cmake_minimum_required(VERSION 3.25)
project("tinyics")

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

#### Options ####

option(BUILD_TESTS "Bulid tests for TinyICS" OFF)

#### Sub directories ####

set(NS3_ENABLED_MODULES "csma;internet") # ns-3 modules to build

add_subdirectory(external/ns-3)
add_subdirectory(external/pybind11)
add_subdirectory(src/bindings)
add_subdirectory(src)
add_subdirectory(sandbox)   # To use library with cpp

if (BUILD_TESTS)
    add_subdirectory(external/googletest)
    add_subdirectory(test)
endif()
