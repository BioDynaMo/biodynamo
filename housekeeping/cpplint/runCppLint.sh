#!/bin/bash

#runs cpplint for all source files in dir src/main/cpp
find ./src/main/cpp -name '*.h' -or -name '*.hpp' -or -name '*.cpp' -or -name '*.cxx' -name '*.cc' | xargs housekeeping/cpplint/cpplint.py --root=src/main/cpp/include --linelength=120 --filter=-build/c++11

#runs cpplint for all source files in dir src/test/cpp
find ./src/test/cpp -name '*.h' -or -name '*.hpp' -or -name '*.cpp' -or -name '*.cxx' -name '*.cc' | xargs housekeeping/cpplint/cpplint.py --root=src/test/cpp/include --linelength=120 --filter=-build/c++11