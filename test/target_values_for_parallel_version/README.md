Cortex3D did not have any unit tests. The testing strategy used to
verify correctness of the ported version (bdm-serial) was black-box-testing the
whole simulator.

However, in the parallel version of BioDynaMo we need unit tests and therefore
target values. Hence, we take bdm-serial as ground truth and extract target
values for the parallel version.

Workflow:
1. Select input parameters for the parallel version
2. Replicate the object initialization for bdm-serial (here)
3. Execute it and measure the result in bdm-serial (=target value)
4. Take this measurements and insert it in the test assertions in the parallel
   version
5. To document how the target value was obtained, also create a test-case in
   bdm-serial

Run target value tests:
`./runBiodynamoTests --gtest_filter=TargetValue*`
