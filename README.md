# TinyICS

Library for simulating ICS Networks built on top of NS-3.

## Building

To set up and compile the project, follow these steps:

1. **Clone the repository:** use --recursive to include the dependencies.
```sh
git clone --recursive https://github.com/ale-candia/tinyics.git
```

2. **Run the configuration script:** Navigate to the `tinyics` directory and execute the configuration script using the following command:
```sh
cd tinyics
bash config.sh
```

3. **Compile the target:** After running the configuration script, navigate to the `build/` directory and compile the project using the make command:
```sh
cd build/
make
```
This command will compile the project. If you have multiple cores available, you can speed up the compilation process by specifying the number of cores to use. For example, you can use `make -j 4` want to use 4 cores.

Once the project has been successfully built, you should be able to use it with the following project structure:
```sh
you_project/
├── tinyics/
└── your_file.py    # imports tinyics
```

## Getting started

Check out the [tutorial](docs/tutorial.md) for an example.

## Unsuported features

- PLC to PLC communication
