# BioDynaMo FAQ

A collection of answers for frequently asked questions regarding BioDynaMo.

## Inncorect handling of data type.

When using some mathematics within the C++ language we need to make sure that our type is declared correctlly, as due to how the C++ language hadngles doubles and intergers e.t.c in some mathematics we may get an unexpected result or error.

This is most commonly caused by the use of fractions, as follows:

```

double A = pow(25.0, 1/2);

```

We know that if handled correctlly A should return as 5, however it will return as 0. This error is due to the fraction we are using, C++ see's the power as an interger rather than a double and then handles it inccorectly causing A to output 0 if called.
This can be simply fixed by making sure our fraction is in the form of double, as follows:

```

double A = pow(25.0, 1.0/2.0);

```
in the case of a fractional power we could also simply use:

```

double A = pow(25.0 , 0.5);

```
in the case of other mathematical fractions we could simply use:

```

double A = 1.0 *(x/y);

```

Where x and y could be any previously defined variable, here the 1.0 is ensuring that we get a double value for A to prevent unexpected results.
