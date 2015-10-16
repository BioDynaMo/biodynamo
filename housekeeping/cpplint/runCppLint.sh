#!/bin/bash

#runs cpplint for all staged source files in dir src/main/cpp
git diff --name-only --cached | grep "src/main" | grep "\(\.h\)\|\(\.cc\)$" | xargs housekeeping/cpplint/cpplint.py --root=src/main/cpp/include --linelength=120 --filter=-build/c++11

#runs cpplint for all staged source files in dir src/main/cpp
git diff --name-only --cached | grep "src/test" | grep "\(\.h\)\|\(\.cc\)$" | xargs housekeeping/cpplint/cpplint.py --root=src/test/cpp/include --linelength=120 --filter=-build/c++11
