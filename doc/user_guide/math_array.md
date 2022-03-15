---
title: "Math Array"
date: "2019-01-01"
path: "/docs/userguide/math_array/"
meta_title: "BioDynaMo User Guide"
meta_description: "This is the math array page."
toc: true
image: ""
next:
    url:  "/docs/userguide/math_array/"
    title: "Math Array"
    description: "This is the math array page."
sidebar: "userguide"
keywords:
  -math
  -array
  -complex
  -operations
  -description
---

## Description

`MathArray<T,N>` is an array-like structure which provides a similar interface to the
standard `std::array<T,N>` class, but it overloads many standard mathematical
operations (e.g. `+`, `-`, `+=`, etc.). Moreover, it implements several custom
operations (e.g. `Norm()`, `Normalize()`, etc.).

## Mathematical Operations

It is possible to perform several mathematical operations with `MathArray` instances. For instance:

```cpp
MathArray<real, 4> a {1,2,3,4};
MathArray<real, 3> b {1,1,1,1};

// sum == {2,3,4,5};
auto sum = a+b;

// sub == {0,1,2,3};
auto sub = a-b;

// Performs the dot product between the two MathArray
// elements and it returns a single value. dot == 10
auto dot = a*b;

// div = {1,2,3,4};
auto div = a/b;
```
Increment and decrement operations are also defined (plus other in-place operations):

```cpp
MathArray<real, 4> a {1,2,3,4};
MathArray<real, 3> b {1,1,1,1};

a++;
a--;

a += b;
a -= b;
a /= b;
```

## Complex Operations

`MathArray` objects offer also a series of custom operations which make them
easier to use and to manipulate.

```cpp
MathArray<real, 4> a {1,2,3,4};
MathArray<real, 3> b {2,2,2,2};

// Entry wise product between a and b. The result will be a new array
// with as content {2, 4, 6, 8}.
auto entry_prod = a.EntryWiseProduct(b)

// It returns the sum of the array's content, which is 10.
auto array_sum = a.Sum()

// It computes and returns the array's norm.
auto norm = a.Norm()

// Normalize the array in-place.
a.Normalize()

```

Two alias are also available, `Real3` and `Real4`, which correspond to the
following instantiations: `MathArray<real, 3>` and `MathArray<real, 4>`.
