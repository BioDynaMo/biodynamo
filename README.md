# BioDynaMo
Biological Dynamic Modeller - Based on Cortex 3D (Cx3D)

[![Build Status](https://travis-ci.org/BioDynaMo/biodynamo.svg?branch=master)](https://travis-ci.org/BioDynaMo/biodynamo)
[![Join the chat at https://cernopenlab.slack.com/messages/biodynamo/](https://img.shields.io/badge/chat-on_slack-ff69b4.svg?style=flat)](https://cernopenlab.slack.com/messages/biodynamo/)

## Introduction

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

## Getting Started

Please have a look at our [user guide](https://biodynamo.github.io/biodynamo/)

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

### Build Targets
| Target          | Description  |
| --------------- | ------------ |
| `test`  | executes all tests |
| `check` | executes all tests and shows test output on failure |
| `clean` | will clean all targets, also the external projects |
| `bdmclean` | will only clean the `biodynamo` and `runBiodynamoTests*` targets |
| `testbdmclean` | will only clean the `runBiodynamoTests*` target |
| `doc` | will generate the Doxygen documentation in directory `build/doc`. It contains a html and latex version. You can view the html version by opening `build/doc/html/index.html` in your browser. |
| `coverage` | will execute the test target and generate a coverage report in `build/coverage`. Make sure that `gcov` and `lcov` are installed and configure cmake with `cmake -Dcoverage=on ..` |
| `coverage-build` | same as `make coverage`, but builds it in a separate directory (`build/coverage`). Since building the coverage report requires different compiler flags, building it in a separate directory keeps the current build directory in good order. |

#### Build Targets Related to C++ Code Style-guide
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

### Contributing Code

The following process describes steps to contribute code changes to the `master` branch.
It follows best practices from industry to ensure a maintainable,
high quality code base.

There are two slightly different versions: (a) for members of our github organization or (b) for externals. Steps are marked accordingly if they differ from each other.

The shown commands assume that `biodynamo/build` is the current directory.

If you follow these steps it will make life of the code reviewer a lot easier!
Consequently, it will ensure that your code is accepted sooner :)

**1. Get familiar with our coding convention**

Carefully read the [BioDynamo Developer Guide](https://github.com/BioDynaMo/biodynamo/wiki/BioDynaMo-Developers-Guide)
and [C++ style guide](https://google.github.io/styleguide/cppguide.html)

**2. (b) Fork the repository**

https://help.github.com/articles/fork-a-repo/

**3. Checkout the `master` branch**

```
git checkout master
```

**4. Get latest version of `master`**

```
# (a)
git pull --rebase origin master
# (b)
git pull --rebase upstream master
```

**5. Create the feature branch**

```
git checkout -b my-feature-branch
```

**6. Make your changes and write tests**

You can make intermediate commits without performing all subsequent steps.
However, for your final submission these steps are essential.

Also for intermediate commit messages: have a look at
[how to write good commit messages](https://github.com/BioDynaMo/biodynamo/wiki/BioDynaMo-Developers-Guide)!

**7. Compile and run tests**

```
make && make check
```
Please make sure that there are no compiler warnings

**8. Code coverage**

Check if code is sufficiently covered by tests.
```
make coverage-build
# open it in browser - e.g.
chromium-browser coverage/coverage/index.html
```

**9. Performance**

Check if code changes affected performance

**10. Documentation**

Write documentation and check result in browser
```
make doc
chromium-browser doc/html/index.html
```
Check if
*  API documentation has been generated correctly
*  it is consistent with code (copy-paste errors)
*  it sufficiently describes the functionality

Please pay attention to warnings from doxygen generation. Here an example of an inconsistent documentation:
```
# make doc ouput:
...
kd_tree_node.h:132: warning: argument 'axis' of command @param is not found in the argument list of bdm::spatial_organization::KdTreeNode< T >::GetSAHSplitPoint()
kd_tree_node.h:132: warning: argument 'num' of command @param is not found in the argument list of bdm::spatial_organization::KdTreeNode< T >::GetSAHSplitPoint()
```

The corresponding code snippet shows a mismatch between code and documentation
which needs to be fixed.
```
/// Gets point, which we use for surface area heuristics
/// @param axis - on what axis are we separating: x=0,y=1,z=2
/// @param num - what parttion are we on (1;N)
/// @return sah rating
Point GetSAHSplitPoint();
```

**11. Perform final checks on your machine**

```
make check-submission
```
This command will execute all tests, check code formatting, styleguide rules, build the documentation and coverage report.

False positives from `clang-tidy` can be silenced by adding `// NOLINT` at the end of the line.
Disabling `clang-format` for a certain part can be done by encapsulating it with the following comments:
```
// clang-format off
code here is not changed by clang-format
// clang-format on
```

If there are no false positives and you are fine with the changes suggested by `clang-format` and `clang-tidy` run: `make fix-submission`. However, failing build, tests, compiler warnings, issues from cpplint and warnings from doxygen must be fixed manually. Also some `clang-tidy` issues cannot be resolved automatically. After running `make fix-submission` please execute `make check-submission` to see if all issues have been resolved.

Please verify that:
- [ ] code compiles without warnings
- [ ] all tests pass
- [ ] all valgrind tests pass
- [ ] code complies with our coding styleguide -- no errors from `clang-format`, `clang-tidy` or `cpplint`
- [ ] documentation is in good order -- see point 10
- [ ] code is sufficiently covered by test cases
- [ ] performance did not degrade due to the code changes

**12. Commit**

Once `make check-submission` does not report any issues, the final commit can be done.
Have a look at [how to write good commit messages](https://github.com/BioDynaMo/biodynamo/wiki/BioDynaMo-Developers-Guide)!
```
git add -i
git commit
```

**13. Create pull request**

Please create a [pull request](https://help.github.com/articles/creating-a-pull-request/)

**14. Verify if Travis-CI builds are OK**

Open the Travis-CI build for Linux and OSX and go through the checklist from point 11 for each of them.
Unlike compilation and test suite execution, problems caused by formatting, code style and documentation will not fail the build. However, they need to be fixed!

**15. If everything is OK contact one of the code reviewers on Slack**

**16. Discuss suggested changes with the code reviewer**

If code changes are necessary, go back to step 6

**17. Congratulations, your code has been merged into the `master` branch**

Many thanks for your contribution, rigor and patience!
