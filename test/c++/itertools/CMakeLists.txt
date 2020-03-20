add_executable(itertools itertools.cpp)
target_link_libraries(itertools PRIVATE gtest itertools_c)
add_test(itertools itertools)
