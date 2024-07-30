# TinyICS

Library for simulating ICS Networks built on top of NS-3.

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

For some examples on how to use the simulator, check out on of the [examples](examples).
