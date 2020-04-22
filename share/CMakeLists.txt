add_subdirectory(cmake)

configure_file(itertools.modulefile.in itertools.modulefile @ONLY)
configure_file(itertoolsvars.sh.in itertoolsvars.sh @ONLY)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/itertools.modulefile
    ${CMAKE_CURRENT_BINARY_DIR}/itertoolsvars.sh
  DESTINATION share
)

message(STATUS "***************************************************************")
message(STATUS "* Use:                                                         ")
message(STATUS "*                                                              ")
message(STATUS "*   source ${CMAKE_INSTALL_PREFIX}/share/itertoolsvars.sh            ")
message(STATUS "*                                                              ")
message(STATUS "* to set up the environment variables                          ")
message(STATUS "***************************************************************")
