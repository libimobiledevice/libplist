# --with-sanitizers support (ASAN/UBSAN).
#
# Every supported sanitizer and coverage flag is enabled. The build uses
# -O1 -fno-omit-frame-pointer and line table debug info, as in the
# autotools build.

include(CheckCCompilerFlag)

set(_libplist_sanitizer_flags "")

check_c_compiler_flag("-fsanitize=address" LIBPLIST_HAS_ASAN)
if(LIBPLIST_HAS_ASAN)
  list(APPEND _libplist_sanitizer_flags "-fsanitize=address")

  check_c_compiler_flag("-fsanitize=address -fsanitize-address-use-after-scope"
                        LIBPLIST_HAS_ASAN_USE_AFTER_SCOPE)
  if(LIBPLIST_HAS_ASAN_USE_AFTER_SCOPE)
    list(APPEND _libplist_sanitizer_flags "-fsanitize-address-use-after-scope")
  endif()
endif()

check_c_compiler_flag("-fsanitize=undefined" LIBPLIST_HAS_UBSAN)
if(LIBPLIST_HAS_UBSAN)
  list(APPEND _libplist_sanitizer_flags "-fsanitize=undefined")
endif()

if(NOT _libplist_sanitizer_flags)
  message(FATAL_ERROR
    "The compiler supports neither -fsanitize=address nor -fsanitize=undefined.")
endif()

# Coverage instrumentation.
set(_libplist_coverage_checks "")
foreach(_check IN ITEMS trace-pc-guard trace-cmp edge)
  check_c_compiler_flag("-fsanitize-coverage=${_check}"
                        LIBPLIST_HAS_COVERAGE_${_check})
  if(LIBPLIST_HAS_COVERAGE_${_check})
    list(APPEND _libplist_coverage_checks "${_check}")
  endif()
endforeach()

if(_libplist_coverage_checks)
  string(JOIN "," _libplist_coverage_flags ${_libplist_coverage_checks})
  list(APPEND _libplist_sanitizer_flags
       "-fsanitize-coverage=${_libplist_coverage_flags}")
else()
  message(WARNING "No sanitizer coverage supported by compiler")
endif()

add_compile_options(-O1 -fno-omit-frame-pointer)

check_c_compiler_flag("-gline-tables-only" LIBPLIST_HAS_GLINE_TABLES_ONLY)
if(LIBPLIST_HAS_GLINE_TABLES_ONLY)
  add_compile_options(-gline-tables-only)
else()
  add_compile_options(-g)
endif()

add_compile_options(${_libplist_sanitizer_flags})
add_link_options(${_libplist_sanitizer_flags})

if(NOT MSVC)
  set(CMAKE_CXX_STANDARD 11)
endif()

# On Windows the sanitizer runtimes cannot be linked into multiple DLLs
# (duplicate libclang_rt symbols); a static build is required.
if(WIN32 AND BUILD_SHARED_LIBS)
  message(WARNING
    "On Windows the sanitizer runtimes (ASAN/UBSAN) cannot be linked into "
    "multiple DLLs (duplicate libclang_rt symbols). Configure with "
    "-DBUILD_SHARED_LIBS=OFF, or build sanitizer builds on Linux/macOS.")
endif()

message(STATUS "Sanitizer flags: ${_libplist_sanitizer_flags}")
