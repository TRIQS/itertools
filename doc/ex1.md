@page ex1 Example 1: Comparison with std::ranges

[TOC]

In this example, we compare **itertools**'s adapted ranges with their counterparts from `std::ranges`.

@include ex1.cpp

Output:

```
(0, a) (1, b)
(0, a) (1, b)
1 4 9 16 25 36
1 4 9 16 25 36
(1, a) (2, b)
(1, a) (2, b)
(1, a) (1, b) (2, a) (2, b) (3, a) (3, b) (4, a) (4, b) (5, a) (5, b) (6, a) (6, b)
(1, a) (1, b) (2, a) (2, b) (3, a) (3, b) (4, a) (4, b) (5, a) (5, b) (6, a) (6, b)
1 3 5
1 3 5
2 3 4 5
2 3 4 5
10 12 14 16 18
10 12 14 16 18
```