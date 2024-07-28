## Building

---
**NOTE**

Currently, the supported platforms are MacOS and Linux.

---

To set up and compile the project, follow these steps:

1. **Create a new directory:** Create a directory for your project and `cd` into the project directory.

```sh
mkdir my_project && cd my_project
```

2. **Clone the repository:** use `--recursive` to include the dependencies.
```sh
git clone --recursive https://github.com/ale-candia/tinyics.git
```

3. **Run the build script:** Navigate to the `tinyics` directory and execute the bulid script:
```sh
cd tinyics
python3 bulid.py
```

Once the project has been successfully built, simulation scripts can be built using the following project structure:

```
my_project/
├─ tinyics/
└─ my_simulation_scipt.py --> uses the tinyics library
```

## Getting started

To get started, check out the [tutorial](docs/tutorial.md) for an example of how to use the simulator.
