#!/usr/bin/env python3

import sys
import os
import subprocess

def run_build():
    num_cores = os.cpu_count()
    build_cmd = ['make', '-j', f'{num_cores}']
    subprocess.run(build_cmd)

def generate_build_files(build_type):
    cmake_cmd = ['cmake', '-S', '.', '-B', 'build/', f'-DCMAKE_BUILD_TYPE={build_type}']
    subprocess.run(cmake_cmd)

if __name__ == '__main__':
    # Get build type
    build_type = 'release' if len(sys.argv) == 1 else sys.argv[1].lower()
    if build_type != 'release' and  build_type != 'debug':
        print(f'Unrecognize build type: {build_type}')
        sys.exit(1)
    print(f'Build type: {build_type}')

    # Create build directory if it doesn't exist
    if not os.path.exists('build'):
        os.mkdir('build')

    generate_build_files(build_type)

    # Go into the build directory
    os.chdir('build')

    # Run the build
    run_build()

    # Go back to the root directory
    os.chdir('..')
