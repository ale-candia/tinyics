#!/bin/bash

# This file is used to configure the build with Compile Commands

if [ -z $1 ]; then
    echo "No build type specified, building in Release by default"
    cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
elif [ "$1" = "Release" ] || [ "$1" = "Debug" ]; then
    cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$1
else
    echo "Unrecognized bulid type '$1', build should be 'Release' or 'Debug'"
    echo "Configuration not written"
fi
