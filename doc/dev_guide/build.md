---
title: "Building Biodynamo"
date: "2019-01-01"
path: "/docs/devguide/build/"
meta_title: "BioDynaMo Dev Guide"
meta_description: "This is the how to build page."
toc: true
image: ""
next:
    url:  "/docs/devguide/build/"
    title: "Building Biodynamo"
    description: "This is the how to build page."
sidebar: "devguide"
keywords:
  -build
  -start
  -create
---

To build BioDynaMo from source execute the following commands:

<a class="sbox" href= "/docs/userguide/installation/" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Note</b></h4>
        <p>If you are a user please follow the installation instructions in our <font color="blue"><u>User Guide</u></font>.
        </p>
    </div>
</a>
<br>

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Install the prerequisites
./prerequisites.sh all

# Create the build directory
mkdir build
cd build

# Build BioDynaMo
cmake ..
make

# (Optional) Installs the libraries
make install
```

The script `prerequisites.sh` is used to install all the dependencies needed by BioDynaMo. You will need
to run it once before actually calling `cmake`. It will choose the specific dependencies depending on the used operating system.
Run `./prerequisites.sh --help` to see how to use it.

<a class="sbox" href= "/docs/userguide/prerequisites/" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Note</b></h4>
    	<p>On macOS the prerequisites script will use `brew` to install the needed packages.
    If you do not have `brew` on your system, do yourself a favour and install it.
    Please have a look to the <font color="blue"><u>prerequisites</u></font> page for more details.
        </p>
    </div>
</a>

## Rebuilding BioDynaMo

If you make developments in the BioDynaMo code you will typically create a new branch and recompile after making your code changes:

```bash
cd biodynamo
git pull
git checkout -b <new-branch>

[edit the files]

# clean the previous build but keep the third party libraries, typically ROOT and ParaView
cd build
ninja cleanbuild

# Build BioDynaMo
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
ninja
```

When developing `ninja` is preferred as it is slightly faster than `make`.

Also we advice you to put the following lines in your `.bashrc` or `.zshrc` file on Linux platforms where we use `pyenv` to manage the different python versions:

```bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"
``` 

Once finished, we hope that you want to contribute your code changes back to the BioDynaMo project. For more on how to contribute see the page on [Contributing your code](/docs/devguide/contribute).

## CentOS 7

In case of CentOS 7, you will need to run the following commands before actually calling `cmake`. This is because CentOS do not provide by default the correct C++ compilers and the correct python interpreter. Moerover, we will need to load the OpenMPI module. You will need to run these instructions only before building BioDynaMo. You will not need them in order to run the program.

```bash
export MESA_GL_VERSION_OVERRIDE=3.3
. scl_source enable devtoolset-8

. /etc/profile.d/modules.sh
module load mpi
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
| `jemalloc`      | `off` | use `jemalloc` for memory allocations |
| `tcmalloc`      | `off` | use `tcmalloc` for memory allocations |
| `website`       | `off` | enable website generation (`make website<-live>` target (see below for more information)) |

### Further CMake command line parameters

| Option          | Description  |
| --------------- | ------------ |
| `CMAKE_CXX_FLAGS`  | specify additional compiler flags - e.g. `"-mavx"` |
| `CMAKE_BUILD_TYPE`  | specify the build type. Possible values are `Debug, Release, RelWithDebInfo, MinSizeRel` |

## Build Targets

| Target          | Description  |
| --------------- | ------------ |
| `run-unit-tests` | executes all BioDynaMo unit tests |
| `run-valgrind` | executes BioDynaMo valgrind tests |
| `run-check` | executes both unit and valgrind tests |
| `run-demos` | executes all demos and integration tests |
| `clean` | will clean all targets, also the external projects |
| `cleanbuild` | will clean everything in the build directory, except for third_party (useful for avoiding downloading third party software) |
| `bdmclean` | will only clean the `biodynamo` and `runBiodynamoTests*` targets |
| `testbdmclean` | will only clean the `runBiodynamoTests*` target |
| `doc` | will generate the API, user and developer documentation in directory `build/doc`. |
| `coverage` | will execute the test target and generate a coverage report in `build/coverage`. Make sure that `kcov` are installed and configure cmake with `cmake -Dcoverage=on ..` |
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
| `check-yapf*` | run YAPF on selected files (.py). Fails if errors are found |
| `show-yapf*` | run YAPF on selected files (.py) and display suggested changes. |
| `yapf*` | run YAPF on selected files (.py) and update them inplace as suggested by `show-yapf*`. |
| `check-cpplint*` | run cpplint on selected files. Fails if errors are found and displays them. |
| `check-submission` | will build, run all tests, check formatting, code style, and generate documentation and coverage report |
| `fix-submission` | will attempt to fix the reported issues using `clang-format`, `clang-tidy`, and `yapf`. Failing build, tests, compiler warnings, issues from cpplint and warnings from doxygen must be fixed manually. Also some `clang-tidy` issues cannot be resolved automatically |

### Website Related Build Targets

| Target          | Description  |
| --------------- | ------------ |
| `website` | will generate the static files used for biodynamo.org |
| `website-live` | starts a local web server so you can immediately view the website in the browser. The website is automatically reloaded if you change a source file. |

## Advanced Build Options

#### Use a Custom Compiler
If you need to user a custom compilers (instead of the one automatically detected by BioDynaMo) you will need
to set the variables: `CXX` for the C++ compiler and `CC` for the C compiler. Please not that your custom compiler
must support the C++14 standard and must be compatible with OpenMP. The complete procedure will become:
```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Let's say I want to use a custom version of clang
export CXX=/opt/local/bin/clang++-mp-8.0
export C=/opt/local/bin/clang++-mp-8.0

./install.sh
```

#### Use a Specific ROOT / ParaView Installation

When you want to inform BioDynaMo of a specific installation of ROOT and /or ParaView on your system,
you will need to perform the following instructions prior to installation.

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# For ROOT
source <root_installation_dir>/bin/thisroot.sh

# For ParaView
export ParaView_DIR=<paraview_installation_dir>/lib/cmake/paraview-5.8
export Qt5_DIR=<qt5_installation_dir>/lib/cmake/Qt5

./install.sh
```

<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Note</b></h4>
    	<p>If you specify ParaView_DIR, then you will need to provide also the Qt5_DIR variable.
    This is because ParaView implicitly relies on the Qt5 installation.
        </p>
    </div>
</a>
<br>


#### Speed Up Installation Tests with a Local BioDynaMo-LFS Copy

The installation scripts fetch large precompiled dependencies like ROOT or ParaView
from biodynamo's large file storage (LFS). To enable faster builds you can download the whole
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

<br>
<a class="sbox" target="_blank" rel="noopener">
    <div class="sbox-content">
    	<h4><b>Warning</b><h4>
    	<p>At the moment there is no check if the local copy is in synch with
    remote. You have to ensure that yourself!
        </p>
    </div>
</a>
