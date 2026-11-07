# Platform checks and config.h generation.

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(CheckLibraryExists)

# Header files.
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)

# plistutil requires getopt_long(); MSVC does not provide <getopt.h>.
check_include_file(getopt.h HAVE_GETOPT_H)

# BUILD_TOOLS is defined only when the tools are actually built.
if(LIBPLIST_WITH_TOOLS AND HAVE_GETOPT_H)
  set(BUILD_TOOLS 1)
endif()

# Functions (same linking checks as autoconf's AC_CHECK_FUNCS).
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(strndup HAVE_STRNDUP)
check_function_exists(strerror HAVE_STRERROR)
check_function_exists(gmtime_r HAVE_GMTIME_R)
check_function_exists(localtime_r HAVE_LOCALTIME_R)
check_function_exists(timegm HAVE_TIMEGM)
check_function_exists(strptime HAVE_STRPTIME)
check_function_exists(memmem HAVE_MEMMEM)

# Types (autoconf: AC_C_CONST, AC_TYPE_SIZE_T, AC_TYPE_SSIZE_T,
# AC_TYPE_UINT8_T, AC_TYPE_UINT16_T, AC_TYPE_UINT32_T).
check_c_source_compiles("
int main(void)
{
    const int c = 1;
    return c;
}" HAVE_CONST)

check_c_source_compiles("
#include <stddef.h>
int main(void)
{
    size_t s = 0;
    return (int)s;
}" HAVE_SIZE_T)

check_c_source_compiles("
#include <sys/types.h>
int main(void)
{
    ssize_t s = 0;
    return (int)s;
}" HAVE_SSIZE_T)

check_c_source_compiles("
#include <stdint.h>
int main(void)
{
    uint8_t a = 0;
    uint16_t b = 0;
    uint32_t c = 0;
    return a + b + c;
}" HAVE_UINT_TYPES)

# struct tm extension members used by time64.c.
check_c_source_compiles("
#include <time.h>
int main(void)
{
    struct tm tm;
    tm.tm_gmtoff = 1;
    return 0;
}" HAVE_TM_TM_GMTOFF)

check_c_source_compiles("
#include <time.h>
int main(void)
{
    struct tm tm;
    tm.tm_zone = (char *)\"UTC\";
    return 0;
}" HAVE_TM_TM_ZONE)

# Endianness: the sources use __BIG_ENDIAN__ / __LITTLE_ENDIAN__.
if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
  set(__BIG_ENDIAN__ 1)
elseif(CMAKE_C_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
  set(__LITTLE_ENDIAN__ 1)
else()
  include(TestBigEndian)
  test_big_endian(_libplist_big_endian)
  if(_libplist_big_endian)
    set(__BIG_ENDIAN__ 1)
  else()
    set(__LITTLE_ENDIAN__ 1)
  endif()
endif()

# libm (fmin).
set(LIBPLIST_MATH_LIBRARY "")
if(NOT WIN32)
  check_library_exists(m fmin "" LIBPLIST_HAVE_LIBM)
  if(LIBPLIST_HAVE_LIBM)
    set(LIBPLIST_MATH_LIBRARY m)
  endif()
endif()

# Generate config.h.
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
  "${CMAKE_BINARY_DIR}/config.h"
  @ONLY)

# Detection summary.
message(STATUS "Character set: ${CMAKE_C_BYTE_ORDER}")
message(STATUS "HAVE_STRNDUP: ${HAVE_STRNDUP}")
message(STATUS "HAVE_STRPTIME: ${HAVE_STRPTIME}")
message(STATUS "HAVE_TIMEGM: ${HAVE_TIMEGM}")
message(STATUS "HAVE_MEMMEM: ${HAVE_MEMMEM}")
