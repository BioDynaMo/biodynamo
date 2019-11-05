# BioDynaMo FAQ

A collection of answers for frequently asked questions regarding BioDynaMo.

## Unexpected fraction output.

When using some mathematics within the C++ language such as fractions one may get different behaviour than they expect if your data types are not declared correctly , the most likely cause of this is due to how C++ handles doubles and integers. 

This is most commonly caused by the use of fractions, as follows:

```

double A = pow(25.0, 1/2);

```

If  our data types where correctly declared A should return as 5, however in this case we will return a 0. This error is due to the fraction we are using, C++ sees the power as an integer rather than a double and then handles it in a way you may not be expecting, causing A to output 0 if called.
This can be simply fixed by making sure our fraction is in the form of double, as follows:

```

double A = pow(25.0, 1.0/2.0);

```
in the case of a fractional power we could also simply use:

```

double A = pow(25.0 , 0.5);

```

## General C ++ tutorial.

If you are unfamiliar with the basics of the C++ language in general or simply need a refresher there are a plethora of useful guides online. For example cplusplus provides and excellent guide http://www.cplusplus.com/doc/tutorial/ , from entry level to more adept concepts for C++ including compilation, data types and much more.
