# MPI C++ Interface is header-only
# Create an Interface Target
add_library(itertools_c INTERFACE)
target_include_directories(itertools_c
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/c++>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)
target_compile_options(itertools_c INTERFACE -std=c++17 $<$<CXX_COMPILER_ID:GNU>:-fconcepts>)

# Install library and headers
install(TARGETS itertools_c EXPORT nda-targets)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
