@page documentation API Documentation

[TOC]

The following provides a detailed reference documentation grouped into logical units.

For most users of the library it should be sufficient to either use one of the @ref range_adapting_functions or
an @ref integer_range.

If you are looking for a specific function, class, etc., try using the search bar in the top left corner.

## Range adapting functions

@ref range_adapting_functions take one or more existing ranges and return lazy @ref adapted_ranges that
can be iterated over.
Lazy means that new elements are produced on the fly whenever they are needed instead of being precomputed
when the range is created.

The following range adpating functions are available in **itertools**:

* @ref itertools::enumerate "enumerate"
* @ref itertools::make_product "make_product"
* @ref itertools::product "product"
* @ref itertools::slice "slice"
* @ref itertools::stride "stride"
* @ref itertools::transform "transform"
* @ref itertools::zip "zip"

## Adapted ranges

@ref adapted_ranges are returned by the range adapting functions and can be iterated over using one of the
@ref range_iterators.
In most cases, the user will never have to create or modify an adapted range directly.
Instead, it is recommended to simply use the provided @ref range_adapting_functions.

The following adapted ranges are defined in **itertools**:

* @ref itertools::detail::enumerated "enumerated"
* @ref itertools::detail::multiplied "multiplied"
* @ref itertools::detail::sliced "sliced"
* @ref itertools::detail::strided "strided"
* @ref itertools::detail::transformed "transformed"
* @ref itertools::detail::zipped "zipped"

## Range iterators

@ref range_iterators are internally used by the library to iterate over @ref adapted_ranges.
In general, there should be no need for users to deal with range iterators directly.
Instead, it is recommended to use range-based for loops, e.g.
```cpp
for (auto [idx, val] : itertools::enumerate(some_range)) {
    // do something with the index and the value of the range
}
```
vs. the traditional for loops, e.g.
```cpp
auto enum_range = itertools::enumerate(some_range);
for (auto it = enum_range.begin(); it != enum_range.end(); ++it) {
    // do something with the iterator
}
```

The following range iterators are defined in **itertools**:

* @ref itertools::detail::enum_iter "enum_iter"
* @ref itertools::detail::prod_iter "prod_iter"
* @ref itertools::detail::stride_iter "stride_iter"
* @ref itertools::detail::transform_iter "transform_iter"
* @ref itertools::detail::zip_iter "zip_iter"

## Integer range

An @ref integer_range is similar to a Python `range`.
It is defined by a start value, an end value and a step size such that the i-th value of the range is given by
`start + i * step`.

The following classes and functions related to integer ranges are defined in **itertools**:

* @ref itertools::range "range"
* @ref itertools::foreach "foreach"
* @ref itertools::product_range "product_range"

## Utilities

@ref utilities are mostly internal implementation details and should not concern everyday users.
The only functions the might be intersting to some users are: @ref itertools::chunk_range "chunk_range",
@ref itertools::make_vector_from_range "make_vector_from_range" and @ref itertools::omp_chunk "omp_chunk".

The following utilities are defined in **itertools**:

* @ref itertools::chunk_range "chunk_range"
* @ref itertools::distance "distance"
* @ref itertools::iterator_facade<Iter, Value, std::forward_iterator_tag, Reference, Difference> "iterator_facade"
* @ref itertools::make_sentinel "make_sentinel"
* @ref itertools::make_vector_from_range "make_vector_from_range"
* @ref itertools::omp_chunk "omp_chunk"
* @ref itertools::sentinel_t "sentinel_t"