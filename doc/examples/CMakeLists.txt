# we need GNU and c++23 for some std::ranges in the example ex1.cpp
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(ex1 ex1.cpp)
    target_link_libraries(ex1 itertools::itertools_c)
    target_compile_features(ex1 PRIVATE cxx_std_23)
endif()

# all other example codes
file(GLOB_RECURSE all_examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
list(FILTER all_examples EXCLUDE REGEX "ex1.cpp")

foreach(ex_src ${all_examples})
    get_filename_component(ex_name ${ex_src} NAME_WE)
    add_executable(${ex_name} ${ex_src})
    target_link_libraries(${ex_name} itertools::itertools_c)
endforeach()