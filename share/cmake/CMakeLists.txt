configure_file(itertools-config.cmake.in itertools-config.cmake @ONLY)
configure_file(itertools-config-version.cmake.in itertools-config-version.cmake @ONLY)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/itertools-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/itertools-config-version.cmake
  DESTINATION lib/cmake/itertools
)

install(EXPORT itertools-targets NAMESPACE itertools:: DESTINATION lib/cmake/itertools)
