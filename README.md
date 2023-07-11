## Building

To set up and compile the project, follow these steps:

1. **Clone the repository:** use --recursive to include the dependencies.
```sh
git clone --recursive https://github.com/ale-candia/icsim.git
```
Or if you want to use it as a git submodule:
```sh
git submodule add https://github.com/ale-candia/icsim.git
```

2. **Run the configuration script:** Navigate to the `icsim` directory and execute the configuration script using the following command:
```sh
cd icsim
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
├── icsim/
└── your_file.py    # imports icsim
```

