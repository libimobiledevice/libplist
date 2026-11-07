# Fuzzers.
#
# Uses clang's -fsanitize=fuzzer runtime instead of building libFuzzer.a.
# --with-fuzzers implies --with-sanitizers (see LibplistOptions.cmake).
# The fuzzers are skipped when the target does not provide the fuzzer
# runtime (e.g. MinGW) or when a non-clang compiler is used.

include(CheckCXXCompilerFlag)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(WARNING "Building the fuzzers requires clang/clang++.")
endif()

check_cxx_compiler_flag("-fsanitize=fuzzer" LIBPLIST_HAS_FUZZER_RUNTIME)
if(NOT LIBPLIST_HAS_FUZZER_RUNTIME)
  message(WARNING
    "-fsanitize=fuzzer is not supported for this target: "
    "the fuzzers are not built.")
  return()
endif()

set(LIBPLIST_FUZZERS
  xplist_fuzzer
  bplist_fuzzer
  jplist_fuzzer
  oplist_fuzzer
)

foreach(_fuzzer IN LISTS LIBPLIST_FUZZERS)
  add_executable(${_fuzzer} "${CMAKE_CURRENT_SOURCE_DIR}/${_fuzzer}.cc")

  target_include_directories(${_fuzzer} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}"
  )

  target_link_libraries(${_fuzzer} PRIVATE plist)

  target_compile_options(${_fuzzer} PRIVATE -fsanitize=fuzzer-no-link)
  target_link_options(${_fuzzer} PRIVATE -fsanitize=fuzzer)

  libplist_set_output_dirs(${_fuzzer} "${CMAKE_BINARY_DIR}/fuzz")
endforeach()
