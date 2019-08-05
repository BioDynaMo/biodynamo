# Building BioDynaMo

To build BioDynaMo from source execute the following commands:

!!! info
    If you are a user please follow the installation instructions in our [user guide](https://biodynamo.github.io/user/)

## Ubuntu 16.04, 18.04

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Install the prerequisites for the project
./prerequisites.sh all

# Create the build directory
mkdir build
cd build

# Build the project
cmake ../
make

# (Optional) Installs the library
make install
```

The script `prerequisites.sh` is used to install all the dependencies needed by BioDynaMo. You will need
to run it before actually calling `cmake` and `make`. It will also choose the specific dependencies given the operating systems.
Run `./prerequisites.sh --help` to see how to use it.

!!! attention

    When trying to install the prerequisites on MacOS the script will user `brew` as a default install method.
    If you do not have `brew` on your system, or you are using a different package manager, you will need to
    manually install all the required packages. Please have a look to the [Prerequisites](user/prerequisites) page.

## CentOS 7.6.1810

In case of CentOS, you will need to run some additional commands before actually calling `cmake` and `make`. This is because
CentOS do not provide by default the correct C++ compilers and the correct python interpreter. Moerover, we will need to
load the OpenMPI module. You will need to run these instructions only before building BioDynaMo. You will not need them in
order to run the library.

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
./prerequisites.sh all centos-7.6.1810

export MESA_GL_VERSION_OVERRIDE=3.3
. scl_source enable rh-python36
. scl_source enable devtoolset-7

. /etc/profile.d/modules.sh
module load mpi

mkdir build && cd build && cmake ../ && make
make install
```

## MacOS

Before building BioDynaMo on MacOS you will need to provide to `cmake` a C++14 and OpenMP compatible compiler. This can
be done by setting the environmental variables `CXX` and `C` for the C++ and C compilers.
Here as example we show the procedure using `clang` compiler installed using `brew`.

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
./prerequisites.sh all osx

export LLVMDIR="/usr/local/opt/llvm"
export CC=$LLVMDIR/bin/clang
export CXX=$LLVMDIR/bin/clang++
export CXXFLAGS=-I$LLVMDIR/include
export LDFLAGS=-L$LLVMDIR/lib
export PATH=$LLVMDIR/bin:$PATH

mkdir build && cd build && cmake ../ && make
make install
```



## CMake Build Options
Our CMake build script uses a few options to influence the build process. They can be set as follows:
``` bash
cmake -Doption=value ..
```
The value for binary options is `on` or `off`.
If you change the value of these switches, you might have to delete `CMakeCache.txt` beforehand.

| Option          | Default Value | Description  |
| --------------- | ------------- | ------------ |
| `test`       | `on` | build the test executables; precondition for e.g. `valgrind` and `coverage` |
| `dict`       | `on` | build ROOT dictionaries. These are compulsory to use backups. Turning them off reduces compilation time. |
| `paraview`       | `on` | Enable visualization using ParaView. Visualization cannot be used if this switch is turned off. |
| `cuda`       | `off` | enable CUDA code generation for GPU acceleration |
| `opencl`        | `off` | enable OpenCL code generation for GPU acceleration |
| `valgrind`      | `on` | enable memory leak checks |
| `coverage`      | `off` | creates a make target to generate a html report indicating which parts of the code are tested by automatic tests |

### Further CMake command line parameters

| Option          | Description  |
| --------------- | ------------ |
| `CMAKE_CXX_FLAGS`  | specify additional compiler flags - e.g. `"-mavx"` |
| `CMAKE_BUILD_TYPE`  | specify the build type. Possible values are `Debug, Release, RelWithDebInfo, MinSizeRel` |

## Build Targets
| Target          | Description  |
| --------------- | ------------ |
| `test`  | executes all tests |
| `check` | executes all tests and shows test output on failure |
| `clean` | will clean all targets, also the external projects |
| `bdmclean` | will only clean the `biodynamo` and `runBiodynamoTests*` targets |
| `testbdmclean` | will only clean the `runBiodynamoTests*` target |
| `doc` | will generate the API, user and developer documentation in directory `build/doc`. |
| `coverage` | will execute the test target and generate a coverage report in `build/coverage`. Make sure that `gcov` and `lcov` are installed and configure cmake with `cmake -Dcoverage=on ..` |
| `coverage-build` | same as `make coverage`, but builds it in a separate directory (`build/coverage`). Since building the coverage report requires different compiler flags, building it in a separate directory keeps the current build directory in good order. |

### C++ Code Style Related Build Targets
The following targets are only available if `clang-format`, `clang-tidy` and `git` are installed.

Build targets indicated with `*` always come in three different flavors.

  * `no-suffix`: executes the target on source files that changed compared to origin/master -- e.g. `make check-format`
  * `-staged`: executes the target on source files that have been staged -- e.g. `make check-format-staged`
  * `-all`: executes the target on all source files in the project -- e.g. `make check-format-all`

| Target          | Description  |
| --------------- | ------------ |
| `check-format*` | run clang-format on selected files. Fails if any file needs to be reformatted |
| `show-format*` | run clang-format on selected files and display differences |
| `format*` | run clang-format on selected files and update them in-place |
| `check-tidy*` | run clang-tidy on selected files. Fails if errors are found |
| `show-tidy*` | run clang-tidy on selected files and display errors. |
| `tidy*` | run clang-tidy on selected files and attempt to fix any warning automatically |
| `check-cpplint*` | run cpplint on selected files. Fails if errors are found and displays them. |
| `check-submission` | will build, run all tests, check formatting, code style, and generate documentation and coverage report |
| `fix-submission` | will attempt to fix the reported issues using `clang-format` and `clang-tidy`. Failing build, tests, compiler warnings, issues from cpplint and warnings from doxygen must be fixed manually. Also some `clang-tidy` issues cannot be resolved automatically |

### Documentation Related Build Targets

| Target          | Description  |
| --------------- | ------------ |
| `doc` | will generate the API, user and developer documentation in directory `build/doc` |
| `live-dev-guide` and `live-user-guide` | starts a local web server so you can immediately view the documentation in the browser. The website is automatically reloaded if you change a source file.  |

## Advanced Build Options

#### Use a Custom Compiler
If you need to user a custom compilers (instead of the one automatically detected by BioDynaMo) you will need
to set first two variables: `CXX` for the C++ compiler and `CC` for the C compiler. Please not that your custom compiler
must support C++14 standard and also it must be compatible with OpenMP. The complete procedure will become:
```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Let's say I want to use a custom version of clang
export CXX=/opt/local/bin/clang++-mp-8.0
export C=/opt/local/bin/clang++-mp-8.0

./install.sh
```

#### Use a Custom ParaView/ROOT installation

BioDynaMo will download automatically the required ParaView, ROOT and Qt5 libraries to build the project. However,
it is also possible to specify custom version of them. You will need to set some environmental variables such to
enable this behaviour. Check out the example below.

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

export ROOT_DIR=/opt/local/root
export ParaView_DIR=/opt/local/paraview
export Qt5_DIR=/usr/local/qt

./install.sh
```

!!! attention

    If you specify ParaView_DIR, then you will need to provide also the Qt5_DIR variable.
    This happens because ParaView needs to explicitly know where Qt is located.

#### Speed Up Installation Tests with a Local BioDynaMo-LFS Copy

The installation scripts fetch large precompiled dependencies like paraview
or root from biodynamo's large file storage (LFS). To enable faster builds you can download the whole
LFS and tell BioDynaMo to access the local version instead. This is done with the
environmental flag `BDM_LOCAL_LFS`. Use an absolute path to the directory
that contains the local copy.

``` bash
export BDM_LOCAL_LFS=/path/to/local/lfs
```

If you want to download the files from remote LFS again execute:

``` bash
unset BDM_LOCAL_LFS
```

!!! warning
    At the moment there is no check if the local copy is in synch with
    remote. You have to ensure that yourself!
    
