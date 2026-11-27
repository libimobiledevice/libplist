# Test suite.
#
# The test programs and .test scripts match test/Makefile.am. The scripts
# are run with top_srcdir/top_builddir in the environment and expect
# plistutil and the test programs at the same locations as the autotools
# build (see libplist_set_output_dirs).

# ---------------------------------------------------------------------------
# Test programs
# ---------------------------------------------------------------------------
function(libplist_add_test_program name source)
  add_executable(${name} ${source})
  target_include_directories(${name} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/libcnary/include"
    "${CMAKE_BINARY_DIR}"
  )
  target_link_libraries(${name} PRIVATE plist)
  libplist_set_output_dirs(${name} "${CMAKE_BINARY_DIR}/test")
  if(TARGET plistutil)
    add_dependencies(${name} plistutil)
  endif()
endfunction()

libplist_add_test_program(plist_cmp          plist_cmp.c)
# plist_cmp compares nodes directly and needs the libcnary node_* helpers.
target_link_libraries(plist_cmp PRIVATE libcnary)
libplist_add_test_program(plist_test         plist_test.c)
libplist_add_test_program(integer_set_test   integer_set.c)
libplist_add_test_program(date_val_test      date_val.c)
libplist_add_test_program(plist_btest        plist_btest.c)
libplist_add_test_program(plist_jtest        plist_jtest.c)
libplist_add_test_program(plist_otest        plist_otest.c)
libplist_add_test_program(xml_behavior_test  xml_behavior_test.c)
libplist_add_test_program(bplist_behavior_test bplist_behavior_test.c)

# C++ test program.
add_executable(plist_test++ plist_test++.cpp)
target_include_directories(plist_test++ PRIVATE
  "${CMAKE_SOURCE_DIR}/include"
  "${CMAKE_BINARY_DIR}"
)
target_link_libraries(plist_test++ PRIVATE plistpp)
libplist_set_output_dirs(plist_test++ "${CMAKE_BINARY_DIR}/test")
if(TARGET plistutil)
  add_dependencies(plist_test++ plistutil)
endif()

# ---------------------------------------------------------------------------
# CTest registration
#
# Each .test script is run as
#   top_srcdir=<src> top_builddir=<build> sh <test>.test
# and requires plistutil and a POSIX shell.
# ---------------------------------------------------------------------------
set(LIBPLIST_TEST_SHELL "" CACHE FILEPATH
  "Shell used to run the .test scripts. Empty selects sh/bash automatically.")

# find_program does not search again for a cached (empty) variable, so
# detect into a helper variable first.
if(NOT LIBPLIST_TEST_SHELL)
  find_program(_libplist_detected_shell NAMES sh bash dash)
  if(_libplist_detected_shell)
    set(LIBPLIST_TEST_SHELL "${_libplist_detected_shell}")
    message(STATUS "Using shell for tests: ${LIBPLIST_TEST_SHELL}")
  endif()
endif()

if(NOT TARGET plistutil)
  message(STATUS
    "plistutil is not built: the .test scripts are not registered with CTest.")
  return()
endif()

if(NOT LIBPLIST_TEST_SHELL)
  message(STATUS
    "No sh/bash found: the .test scripts are not registered with CTest. "
    "Pass -DLIBPLIST_TEST_SHELL=<path> to enable them.")
  return()
endif()

get_filename_component(_libplist_shell_dir "${LIBPLIST_TEST_SHELL}" DIRECTORY)

libplist_to_posix_path(_libplist_top_srcdir "${CMAKE_SOURCE_DIR}")
libplist_to_posix_path(_libplist_top_builddir "${CMAKE_BINARY_DIR}")
libplist_to_posix_path(_libplist_shell "${LIBPLIST_TEST_SHELL}")
libplist_to_posix_path(_libplist_shell_dir "${_libplist_shell_dir}")

# On Windows the DLLs and plistutil.exe are not in the same directory as the
# test programs, so extend PATH. The current PATH is kept because the
# MSYS2/mingw runtime DLLs may live in the compiler's bin directory.
if(WIN32)
  set(_libplist_test_env_path "${CMAKE_BINARY_DIR};${_libplist_shell_dir};$ENV{PATH}")
  file(TO_CMAKE_PATH "${_libplist_test_env_path}" _libplist_test_env_path)
  # add_test() arguments are a CMake list, so escape the semicolons.
  string(REPLACE ";" "\\;" _libplist_test_path_arg "PATH=${_libplist_test_env_path}")
else()
  set(_libplist_test_path_arg "")
endif()

set(LIBPLIST_TESTS
  date_val.test
  empty.test
  small.test
  medium.test
  large.test
  huge.test
  bigarray.test
  empty++.test
  small++.test
  medium++.test
  large++.test
  huge++.test
  bigarray++.test
  dates.test
  timezone1.test
  timezone2.test
  signedunsigned1.test
  signedunsigned2.test
  signedunsigned3.test
  hex.test
  order.test
  recursion.test
  entities.test
  empty_keys.test
  amp.test
  invalid_tag.test
  cdata.test
  offsetsize.test
  refsize.test
  malformed_dict.test
  uid.test
  integer_set.test
  json1.test
  json2.test
  json3.test
  json-invalid-types.test
  json-int64-min-max.test
  ostep1.test
  ostep2.test
  ostep-strings.test
  ostep-comments.test
  ostep-invalid-types.test
  xml_behavior.test
  bplist_behavior.test
  plistutil-streams.test
)

foreach(_test IN LISTS LIBPLIST_TESTS)
  string(REPLACE ".test" "" _test_name "${_test}")

  add_test(NAME "${_test_name}"
    COMMAND ${CMAKE_COMMAND} -E env
      "top_srcdir=${_libplist_top_srcdir}"
      "top_builddir=${_libplist_top_builddir}"
      ${_libplist_test_path_arg}
      "${_libplist_shell}" "${CMAKE_CURRENT_SOURCE_DIR}/${_test}"
  )

  set_tests_properties("${_test_name}" PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    TIMEOUT 120
  )
endforeach()
