# Differences between Math.exp (Java) and std::exp (C++)

tested with:
g++ (Ubuntu 5.2.1-22ubuntu2) 5.2.1 20151010
openjdk version "1.8.0_66-internal"


## compile and execute
```bash
javac MathExp.java && java MathExp
g++ -std=c++11 std_exp.cpp -o std_exp && ./std_exp
```

## Output
Java:
```bash
0.9971316701217852
3fefe880ad1f5360
```

C++
```bash
0.99713167012178527
3fefe880ad1f5361
```
