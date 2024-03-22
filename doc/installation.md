@page installation Installation

[TOC]

**itertools** is a header only library and installation is not necessary.
However, it still supports the usual installation procedure using CMake.

If you want to skip the installation step, you can go directly to @ref integration to see how you can integrate
**itertools** into your own C++ project.

> **Note:** To guarantee reproducibility in scientific calculations, we strongly recommend the use of a stable
> [release version](https://github.com/TRIQS/itertools/releases).

@section dependencies Dependencies

The dependencies of **itertools** are as follows:

* gcc version 12 or later OR clang version 15 or later
* CMake version 3.20 or later (for installation or integration into an existing project via CMake)

@section install_steps Installation steps

1. Download the source code of the latest stable version by cloning the [TRIQS/itertools](https://github.com/triqs/itertools)
repository from GitHub:

    ```console
    $ git clone https://github.com/TRIQS/itertools itertools.src
    ```

2. Create and move to a new directory where you will compile the code:

    ```console
    $ mkdir itertools.build && cd itertools.build
    ```

3. In the build directory call cmake, including any additional custom CMake options (see below):

    ```console
    $ cmake -DCMAKE_INSTALL_PREFIX=path_to_install_dir ../itertools.src
    ```

    Note that it is required to specify ``CMAKE_INSTALL_PREFIX``, otherwise CMake will stop with an error.

4. Compile the code, run the tests and install the application:

    ```console
    $ make -j N
    $ make test
    $ make install
    ```

    Replace `N` with the number of cores you want to use to build the library.

@section versions Versions

To use a particular version, go into the directory with the sources, and look at all available versions:

```console
$ cd itertools.src && git tag
```

Checkout the version of the code that you want:

```console
$ git checkout 1.2.0
```

and follow steps 2 to 4 to compile the code.

@section cmake_options Custom CMake options

The compilation of **itertools** can be configured using CMake options

```console
$ cmake ../itertools.src -DOPTION1=value1 -DOPTION2=value2 ...
```


| Options                                 | Syntax                                            |
|-----------------------------------------|---------------------------------------------------|
| Specify an installation path            | ``-DCMAKE_INSTALL_PREFIX=path_to_install_dir``    |
| Build in Debugging Mode                 | ``-DCMAKE_BUILD_TYPE=Debug``                      |
| Disable testing (not recommended)       | ``-DBuild_Tests=OFF``                             |
| Build the documentation                 | ``-DBuild_Documentation=ON``                      |
| Build benchmarks                        | ``-DBuild_Benchs=ON``                             |
