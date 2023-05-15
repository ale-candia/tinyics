#!/bin/bash

if [ -z $1 ]; then
    echo "No build type specified, building in Release by default"
    cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DNS3_ENABLED_MODULES="core;csma;internet;network"
elif [ "$1" = "Release" ] || [ "$1" = "Debug" ]; then
    cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=$1 -DNS3_ENABLED_MODULES="core;csma;internet;network"
else
    echo "Unrecognized bulid type '$1', build should be 'Release' or 'Debug'"
    echo "Configuration not written"
fi

