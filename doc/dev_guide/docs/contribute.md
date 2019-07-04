---
title: "Contribute"
meta_title: "contribute"
keywords:
  -contribute
  -give back
sidebar: "devguide"
---

## From Cloning BioDynaMo to Your First Contribution

The following process describes steps to contribute code changes to the `master` branch.
It follows best practices from industry to ensure a maintainable, high quality code base.

The shown commands assume that `biodynamo/build` is the current directory.

If you follow these steps it will make life of the code reviewer a lot easier!
Consequently, it will ensure that your code is accepted sooner :)

### 1. Get familiar with our coding convention

Carefully read the [C++ style guide](https://google.github.io/styleguide/cppguide.html)
and our page about [Code Quality](code_quality)

### 2. Clone the repository

``` bash
git clone https://github.com/BioDynaMo/biodynamo.git
```

### 3. Checkout the `master` branch

``` bash
git checkout master
```

### 4. Get latest version of `master`

``` bash
git pull origin master
```

### 5. Create the feature branch

``` bash
git checkout -b my-feature-branch
```

### 6. Make your changes and write tests

You can make intermediate commits without performing all subsequent steps.
However, for your final submission these steps are essential.

Also for intermediate commit messages: have a look at
[how to write good commit messages](code_quality/#git-commit-message)!

### 7. Compile and run tests

``` bash
make && make check
```
Please make sure that there are no compiler warnings

### 8. Code coverage

Check if code is sufficiently covered by tests.
``` bash
make coverage-build
# open it in browser - e.g.
chromium-browser coverage/coverage/index.html
```

### 9. Performance

Check if code changes affected performance

### 10. Documentation

Write documentation and check result in browser
``` bash
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

### 11. Perform final checks on your machine

``` bash
make check-submission
```
This command will execute all tests, check code formatting, styleguide rules, build the documentation and coverage report ([more info](contribute/#make-check-submission-explained)).

False positives from `clang-tidy` can be silenced by adding `// NOLINT` at the end of the line.
Disabling `clang-format` for a certain part can be done by encapsulating it with the following comments:
```
// clang-format off
code here is not changed by clang-format
// clang-format on
```

If there are no false positives and you are fine with the changes suggested by `clang-format` and `clang-tidy` run: `make fix-submission`. However, failing build, tests, compiler warnings, issues from cpplint and warnings from doxygen must be fixed manually. Also some `clang-tidy` issues cannot be resolved automatically. After running `make fix-submission` please execute `make check-submission` to see if all issues have been resolved.

Please verify that:

* [ ] code compiles without warnings
* [ ] all tests pass
* [ ] all valgrind tests pass
* [ ] code complies with our coding styleguide -- no errors from `clang-format`, `clang-tidy` or `cpplint`
* [ ] documentation is in good order -- see point 10
* [ ] code is sufficiently covered by test cases
* [ ] performance did not degrade due to the code changes

### 12. Commit

Once `make check-submission` does not report any issues, the final commit can be done.
Have a look at [how to write good commit messages](https://github.com/BioDynaMo/biodynamo/wiki/BioDynaMo-Developers-Guide)!
``` bash
git add -i
git commit
```

### 13. Create pull request

Please create a [pull request](https://help.github.com/articles/creating-a-pull-request/)

### 14. Verify if Travis-CI builds are OK

Open the Travis-CI build for Linux and OSX and go through the checklist from point 11 for each of them.
Unlike compilation and test suite execution, problems caused by formatting, code style and documentation will not fail the build. However, they need to be fixed!

### 15. If everything is OK contact one of the code reviewers on Slack

### 16. Discuss suggested changes with the code reviewer

If code changes are necessary, go back to step 6

### 17. Congratulations, your code has been merged into the `master` branch

Many thanks for your contribution, rigor and patience!

## 98% Finished Projects
In the open source world sometimes it happens that people work on a feature for weeks or month and leave after it has been finished for 98%. Although this 2% don't look like a big issue, usually that means that all your work doesn't make it into the production code. Normally, other developers are busy and don't have the time to dive into your work and find the pieces that are missing or not working yet. This situation would be a waste of your precious time. We bet that it is way more satisfying if your contribution makes it into production and will be used by scientists around the world.

For a contribution to be considered 100% complete, it must (be)
* comply to our coding guidelines,
* unit tested,
* well documented
* include a demo / screencast in certain cases.

Therefore, we want to encourage you to reserve enough time in the end where you don't code. We do our best to support you!


## `make check-submission` explained

The command `make check-submission` is our central automatic tool to validate if code changes are ready to be merged into master. It performs a series of checks and reports errors or warnings.

Therefore, it makes the code review process easier. Since developers can execute it on their local machine, the feedback loop is much tighter, resulting in a faster submission process.
Although, many issues are caught, it has its limitations. Thus, it cannot fully replace manual code reviews.

Since it possibly outputs a lot of information, this page explains how to interpret the results.

[Here](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233559945) an example how the output should look like if all checks are OK

  * [Successful build without compiler warnings](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233559945#L930-L1015)
  * [All tests pass](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233559945#L1017-L1026)
  * [`clang-format` does not report issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233559945#L1063)
  * [`clang-tidy` does not report issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233559945#L1092)
  * [`cpplint` does not report issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233559945#L1145)
  * [doxygen does not report issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233559945#L1150)

[Here](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836) an example of a **passing build**, but with issues in many categories -- these issues must be fixed as well:

  * [Compiler warning](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836#L936-L940)
  * [`clang-format` reports issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836#L1047-L1051)
  * [Since there were issues, they are displayed](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836#L1059-L1079)
  * [`clang-tidy` reports issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836#L1086-L1093)
  * [Since there were issues, they are displayed](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836#L1100-L1114)
  * [`cpplint` reports issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836#L1119-L1129)
  * [doxygen reports issues](https://travis-ci.org/BioDynaMo/biodynamo/jobs/233636836#L1133-L1142)
