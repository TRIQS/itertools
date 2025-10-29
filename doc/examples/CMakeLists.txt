# We need GNU and c++23 for some std::ranges
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(ex1 ex1.cpp)
    target_link_libraries(ex1 itertools::itertools_c)
    target_compile_features(ex1 PRIVATE cxx_std_23)
endif()