# Building BioDynaMo

To build BioDynaMo from source execute the following commands:

!!! info
    If you are a user please follow the installation instructions in our [user guide](https://biodynamo.github.io/user/)


``` bash
git clone https://github.com/BioDynaMo/biodynamo
cd biodynamo

./prerequesites.sh
# follow the instructions of the script:
source <path-to-bdm-installation>/biodynamo-env.sh

mkdir build && cd build
cmake .. && make -j4
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
| `cuda`       | `off` | enable CUDA code generation for GPU acceleration |
| `opencl`        | `off` | enable OpenCL code generation for GPU acceleration |
| `valgrind`      | `on` | enable memory leak checks |
| `coverage`      | `off` | creates a make target to generate a html report indicating which parts of the code are tested by automatic tests |

### Further CMake command line parameters:

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

## Speed Up Installation Tests with a Local BioDynaMo-LFS Copy

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
