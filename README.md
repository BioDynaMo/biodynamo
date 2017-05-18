# BioDynaMo

[![Join the chat at https://gitter.im/BioDynaMo/biodynamo](https://badges.gitter.im/BioDynaMo/biodynamo.svg)](https://gitter.im/BioDynaMo/biodynamo?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
Biological Dynamic Modeller - Based on Cortex 3D (Cx3D)

[![Build Status](https://travis-ci.org/BioDynaMo/biodynamo.svg?branch=master)](https://travis-ci.org/BioDynaMo/biodynamo)
[![Join the chat at https://cernopenlab.slack.com/messages/biodynamo/](https://img.shields.io/badge/chat-on_slack-ff69b4.svg?style=flat)](https://cernopenlab.slack.com/messages/biodynamo/)

##Introduction:

The brain is an extremely complex system, consisting of approximately 100 billion neurons
that are connected to one another. The way these neurons are structured allows for very
efficient and robust function. For example, human face recognition outperforms any currently
available machine algorithm. One way to better understand this complex structure is to
elucidate how it arises during development. The improvements in computing technology in
the last few years have made it possible to use large-scale computer simulations to
investigate such developmental processes. However, the appropriate software that can fully
exploit the potentials of the state-of-the-art hardware remains to be implemented.

A currently available software solution to simulate neural development is Cx3D,
<https://www.ini.uzh.ch/~amw/seco/cx3d/>. However, this software is Java-based, and not
ideal for high-performance computing (HPC). In order to adapt Cx3D to support HPC, a
software that has similar functionalities as Cx3D but is coded in C++ is needed.

## Prerequesites
Obligatory:
 * Compiler (g++ >= 4.8.4 or clang >= 3.9.0)
 * CMake >= 3.2.0
 * git
 * valgrind

Optional:
 * doxygen (to build documentation)
 * gcov and lcov (to generate test coverage report)
 * clang-format (to format code according to our style guide)
 * python (to run `cpplint` which checks coding conventions)

## Build Instructions

Check out code from this repository:
```
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
```

Create build directory
```
mkdir build && cd build
```

Configure and build all targets
```
cmake ..
make
```

This commands build a shared library for BioDynaMo together with a test executable. Furthermore, dependencies
(GMP, gtest, ...) are automatically downloaded and built as well. They are defined in CMake as ExternalProjects

Run Tests
```
make check
```

In contrast to `make test`, the target `check` will show the test output on failure

### CMake Options
Our CMake build script uses a few options to influence the build process. They can be set as follows:
```
cmake -Doption=value ..
```
The value for binary options is `on` or `off`.
If you change the value of these switches, you might have to delete `CMakeCache.txt` beforehand.

| Option          | Default Value | Description  |
| --------------- | ------------- | ------------ |
| `test`       | `on` | build the test executable; precondition for e.g. `valgrind` and `coverage` |
| `valgrind`      | `on` | enable memory leak checks |
| `coverage`      | `off` | creates a make target to generate a html report indicating which parts of the code are tested by automatic tests |

#### Further CMake command line parameters:

| Option          | Description  |
| --------------- | ------------ |
| `CMAKE_CXX_FLAGS`  | specify compiler optimization flags - e.g. `"-O3 -mavx"` |
| `CMAKE_C_COMPILER, CMAKE_CXX_COMPILER` | change default compiler |
| `CMAKE_BUILD_TYPE`  | specify the build type. Possible values are `Debug, Release, RelWithDebInfo, MinSizeRel` |

### Make Targets
`make test` executes all tests

`make check` executes all tests and shows test output on failure

`make clean` will clean all targets, also the external projects

`make bdmclean` will only clean the `biodynamo` and `runBiodynamoTests` targets

`make testbdmclean` will only clean the `runBiodynamoTests` target

`make doc` will generate the Doxygen documentation in directory `build/doc`. It contains a html and latex version.
You can view the html version by opening `build/doc/html/index.html` in your browser.

`make coverage` will execute the test target and generate a coverage report in `build/coverage`. Make sure that `gcov`
 and `lcov` are installed and configure cmake with `cmake -Dcoverage=on ..`


### Contributing Code

The following process describes steps to contribute code changes to the `master` branch.
It follows best practices from industry to ensure a maintainable,
high quality code base.

There are two slightly different versions: (a) for members of our github organization or (b) for externals. Steps are marked accordingly if they differ from each other.

The shown commands assume that `biodynamo/build` is the current directory.

If you follow these steps it will make life of the code reviewer a lot easier!
Consequently, it will ensure that your code is accepted sooner :)

1. Get familiar with our coding convention

  Carefully read the [BioDynamo Developer Guide](https://github.com/BioDynaMo/biodynamo/wiki/BioDynaMo-Developers-Guide)
 and [C++ style guide](https://google.github.io/styleguide/cppguide.html)

2. (b) Fork the repository

   https://help.github.com/articles/fork-a-repo/

3. Checkout the `master` branch

    ```
    git checkout master
    ```
4. Get latest version of `master`

    ```
    # (a)
    git pull --rebase origin master
    # (b)
    git pull --rebase upstream master
    ```
5. Create the feature branch
    ```
    git checkout -b my-feature-branch
    ```
6. Make your changes and write tests

   You can make intermediate commits without performing all subsequent steps.
   However, for your final submission these steps are essential.

   Also for intermediate commit messages: have a look at
   [how to write good commit messages](https://github.com/BioDynaMo/biodynamo/wiki/BioDynaMo-Developers-Guide)!

7. Run the tests

    ```
    make check
    ```
8. Code coverage

   Check if code is sufficiently covered by tests.
   ```
   # e.g.
   mkdir ../build_coverage && cd ../build_coverage
   cmake -Dcoverage=on ..
   make coverage
   # open it in browser - e.g.
   chromium-browser ./coverage/index.html
   ```

9. Performance

    Check if code changes affected performance

10. Documentation

   Build documentation and open it in a browser.
   ```
   make doc
   chromium-browser doc/html/index.html
   ```
   Check if
   *  API documentation has been generated correctly
   *  it is consistent with code (copy-paste errors)
   *  it sufficiently describes the functionality

11. Apply `clang-format` to all changed source files
   ```
   # e.g.
   clang-format -i ../src/backend.h
   ```
12. check code style
   ```
   ../housekeeping/cpplint/runCppLint.sh ../src/backend.h
   # for more infos see
   ../housekeeping/cpplint/runCppLint.sh help
   ```

   sample output:
   ```
   ../src/backend.h:1:  #ifndef header guard has wrong style, please use: BACKEND_H_  [build/header_guard] [5]
../src/backend.h:166:  #endif line should be "#endif  // BACKEND_H_"  [build/header_guard] [5]
../src/backend.h:9:  Found C system header after C++ system header. Should be: backend.h, c system, c++ system, other.  [build/include_order] [4]
../src/backend.h:16:  Single-parameter constructors should be marked explicit.  [runtime/explicit] [5]
../src/backend.h:18:  Missing username in TODO; it should look like "// TODO(my_username): Stuff."  [readability/todo] [2]
../src/backend.h:80:  Single-parameter constructors should be marked explicit.  [runtime/explicit] [5]
../src/backend.h:81:  Single-parameter constructors should be marked explicit.  [runtime/explicit] [5]
../src/backend.h:136:  Missing username in TODO; it should look like "// TODO(my_username): Stuff."  [readability/todo] [2]
Done processing ../src/backend.h
Total errors found: 8
   ```

   _*Caution*_:
   `runCppLint.sh` is not able to verify all coding rules. For example, names of classes,
   methods, members, ... are not checked. Therefore it is important that you
   have read the coding guidelines and manually check if there are errors.

13. Commit

   Have a look at [how to write good commit messages](https://github.com/BioDynaMo/biodynamo/wiki/BioDynaMo-Developers-Guide)!
    ```
    git add -i
    git commit
    ```

14. Create pull request

    Once everything has been done it is ready for code review. Therefore, please
    create a [pull request](https://help.github.com/articles/creating-a-pull-request/)

15. Check if Travis-CI reports a passing build

    If not, please go back to step 6 and fix the issue

16. Wait for feedback from code review

17. Discuss suggested changes with the code reviewer

    If code changes are necessary, go back to step 6

18. Congratulations, your code has been merged into the `master` branch

    Many thanks for your contribution, rigor and patience!
