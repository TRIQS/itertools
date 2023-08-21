(changelog)=

# Changelog

## Version 1.2.0

This is Version 1.2.0 of Itertools, a single-header library for adapting C++ ranges.

We thank all contributors: Thomas Hahn, Alexander Hampel, Olivier Parcollet, Dylan Simon, Nils Wentzell

Find below an itemized list of changes in this release.

### General
* Add default constructors to various iterator types
* Add default move assign/constructor for transformed_iter
* Add size function for itertools::sliced
* Use itertools distance in omp_chunk implementation
* Change loop order in product such that rightmost range is the fastest
* deprecate range default constructor
* clang-format all source files
* Fix compiler warnings

### cmake
* Add compiler warnings for IntelLLVM (#6)
* Synchronize deps/CMakeLists.txt with nda
* Remove redundant PythonSupport check
* Update Findsanitizer.cmake to include TSAN and MSAN
* Do not build documentation as subproject

### fixes
* Fix in transform_iter::operator=
* Fix copy assignment for transform_iter
* Fix bug in range(...).size() for negative steps + test
* Fix bug in itertools::range::size function


## Version 1.1.0

This is Version 1.1.0 of Itertools, a single-header library for adapting C++ ranges.

We thank all contributors: Daniel Bauernfeind, Alexander Hampel, Dylan Simon, Nils Wentzell

Find below an itemized list of changes in this release.

### General
* Bugfix: Use sentinel_t for those range adaptors using std::end/cend on the underlying range
* Add defaulted operator== to the various range adaptors
* Fix make_vector_from_range to work for ranges with mismatching begin and end type
* Add benchmarks for range and product_range
* Important FIX in range implementation for nontrivial steps + test
* Be sure to add initializer for all member of range type
* Keep alias range::index_t for backward compatibility
* Add range::all_t and associated static member range::all
* Merge range implementation of nda, improve range() documentability
* Make sure to generate empty ranges in accordance with python ranges

### doc
* Add link to reference doc to README.md
* Minor doc cleanups for doxygen generation, add Doxyfile and update .gitignore

### c++20
* Resplace std::result_of by std::invoke_result

### cmake
* Bump Version number to 1.1.0
* Set CXX standard using target_compile_features
* Remove unused FindPython.cmake file


## Version 1.0.0

Itertools Version 1.0.0 is a single-header
C++ library that allows, with a simple interface,
for the writing of various types of range-based for loops.

This is the initial release for this project.
