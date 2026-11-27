# Python bindings (Cython).
#
# Builds plist.pyx into the plist extension module and installs plist.pxd
# so that other packages can cimport it. Automatically disabled when Cython
# or the Python development files are missing.

find_package(Python3 3.0 COMPONENTS Interpreter Development.Module QUIET)

if(NOT Python3_FOUND)
  message(WARNING
    "Python 3 (with development headers) not found: the Cython bindings "
    "are disabled. Use -DLIBPLIST_WITH_CYTHON=OFF to silence this message.")
  return()
endif()

find_program(CYTHON_EXECUTABLE NAMES cython cython3)

if(NOT CYTHON_EXECUTABLE)
  message(WARNING
    "Cython not found: the Cython bindings are disabled. "
    "Use -DLIBPLIST_WITH_CYTHON=OFF to silence this message.")
  return()
endif()

execute_process(
  COMMAND "${CYTHON_EXECUTABLE}" --version
  OUTPUT_VARIABLE _libplist_cython_version
  ERROR_VARIABLE _libplist_cython_version
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE)

string(REGEX MATCH "[0-9]+\\.[0-9]+(\\.[0-9]+)?"
       _libplist_cython_version_number "${_libplist_cython_version}")

if(_libplist_cython_version_number VERSION_LESS 3.0)
  message(WARNING
    "Cython ${_libplist_cython_version_number} is too old (3.0.0 or newer "
    "is required): the Cython bindings are disabled.")
  return()
endif()

# Generate plist.c.
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/plist.c"
  COMMAND "${CYTHON_EXECUTABLE}"
          -I "${CMAKE_CURRENT_SOURCE_DIR}"
          -I "${CMAKE_SOURCE_DIR}/src"
          -o "${CMAKE_CURRENT_BINARY_DIR}/plist.c"
          "${CMAKE_CURRENT_SOURCE_DIR}/plist.pyx"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/plist.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/plist.pxd"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "Generating plist.c from plist.pyx using Cython"
)

# The extension module.
add_library(plist_cython MODULE
  "${CMAKE_CURRENT_BINARY_DIR}/plist.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/plist_util.c"
)

target_include_directories(plist_cython PRIVATE
  "${CMAKE_SOURCE_DIR}/include"
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_BINARY_DIR}"
  ${Python3_INCLUDE_DIRS}
)

target_link_libraries(plist_cython PRIVATE plist Python3::Module)

# The generated code exports the Python module symbols; override the global
# -fvisibility=hidden.
if(NOT MSVC)
  target_compile_options(plist_cython PRIVATE -fvisibility=default)
endif()

set_target_properties(plist_cython PROPERTIES
  OUTPUT_NAME "plist"
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

if(WIN32)
  set_target_properties(plist_cython PROPERTIES SUFFIX ".pyd")
endif()

# Installation: the extension module goes into the Python site-packages
# directory, plist.pxd into ${includedir}/plist/cython.
set(_libplist_python_version "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
set(LIBPLIST_PYTHON_INSTALL_DIR
  "${CMAKE_INSTALL_LIBDIR}/python${_libplist_python_version}/site-packages"
  CACHE PATH "Install directory for the libplist Python module")

install(TARGETS plist_cython
  LIBRARY DESTINATION "${LIBPLIST_PYTHON_INSTALL_DIR}"
  RUNTIME DESTINATION "${LIBPLIST_PYTHON_INSTALL_DIR}"
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/plist.pxd"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/plist/cython"
)
