Every test source file in this directory is compiled to a separate binary.
BioDynaMo sometimes uses forward declared default parameters. C++ one definition
rule forbids changing them once they are defined. Although it compiles and links
if they are in different translation units, it is undefined behavior.
For more information please see:
https://stackoverflow.com/questions/9364720/c-different-classes-with-the-same-name-in-different-translation-units?rq=1

Therefore, to test code with different compile time parameters it is necessary
to compile them into a separate binary. CMake automatically adds them to the
the targets `test, check, coverage, testbdmclean`.
