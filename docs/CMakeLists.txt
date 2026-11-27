# Documentation: install the plistutil man page and, when Doxygen is
# available, generate the API documentation.

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/plistutil.1"
  DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
)

find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
  set(_libplist_docs_dir "${CMAKE_BINARY_DIR}/docs")

  # docs/README.doxygen.md (equivalent of the awk rule in Makefile.am).
  add_custom_command(
    OUTPUT "${_libplist_docs_dir}/README.doxygen.md"
    COMMAND ${CMAKE_COMMAND}
            -DINPUT=${CMAKE_SOURCE_DIR}/README.md
            -DOUTPUT=${_libplist_docs_dir}/README.doxygen.md
            -P ${CMAKE_SOURCE_DIR}/cmake/GenerateReadmeDoxygen.cmake
    DEPENDS
      "${CMAKE_SOURCE_DIR}/README.md"
      "${CMAKE_SOURCE_DIR}/cmake/GenerateReadmeDoxygen.cmake"
    COMMENT "Generating docs/README.doxygen.md"
  )

  # doxygen.cfg: substitute @PACKAGE@/@VERSION@ (provided by AC_INIT in the
  # autotools build) and rewrite the input and output paths so the
  # documentation can be generated out of tree.
  set(PACKAGE "libplist")
  set(VERSION "${LIBPLIST_VERSION}")
  configure_file(
    "${CMAKE_SOURCE_DIR}/doxygen.cfg.in"
    "${CMAKE_BINARY_DIR}/doxygen.cfg"
    @ONLY)

  file(READ "${CMAKE_BINARY_DIR}/doxygen.cfg" _libplist_doxygen_cfg)
  string(REGEX REPLACE
    "OUTPUT_DIRECTORY([ \t]*)=[ \t]*docs"
    "OUTPUT_DIRECTORY\\1= ${_libplist_docs_dir}"
    _libplist_doxygen_cfg "${_libplist_doxygen_cfg}")
  string(REPLACE
    "= include/plist/plist.h"
    "= ${CMAKE_SOURCE_DIR}/include/plist/plist.h"
    _libplist_doxygen_cfg "${_libplist_doxygen_cfg}")
  string(REPLACE
    "docs/README.doxygen.md"
    "${_libplist_docs_dir}/README.doxygen.md"
    _libplist_doxygen_cfg "${_libplist_doxygen_cfg}")
  file(WRITE "${CMAKE_BINARY_DIR}/doxygen.cfg" "${_libplist_doxygen_cfg}")

  add_custom_target(docs
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/doxygen.cfg"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DEPENDS "${_libplist_docs_dir}/README.doxygen.md"
    COMMENT "Generating API documentation with Doxygen"
  )
else()
  message(STATUS "Doxygen not found: the 'docs' target is disabled")
endif()
