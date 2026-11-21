# CMake build for libplist.
#
# This is an alternative to the autotools build. The cache variables mirror
# the autotools configure switches, and their defaults match the autotools
# defaults: tools, test suite and Python bindings are enabled; debug code,
# sanitizers and fuzzers are disabled; shared libraries are built.
#
# To change a default, pass for example:
#   -DLIBPLIST_WITH_TOOLS=OFF       --without-tools
#   -DLIBPLIST_WITH_TESTS=OFF       --without-tests
#   -DLIBPLIST_DEBUG=ON             --enable-debug
#   -DLIBPLIST_WITH_CYTHON=OFF      --without-cython
#   -DLIBPLIST_WITH_SANITIZERS=ON   --with-sanitizers
#   -DLIBPLIST_WITH_FUZZERS=ON      --with-fuzzers
#   -DBUILD_SHARED_LIBS=OFF         --enable-static --disable-shared
#
# Out-of-tree build:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build -j
#   ctest --test-dir build --output-on-failure
#
# Requires CMake >= 3.20.

cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(libplist LANGUAGES C CXX)

include(LibplistOptions)
include(LibplistVersion)

include(GNUInstallDirs)
include(LibplistHelpers)

# Let installed executables find the shared libraries in the same prefix.
if(NOT CMAKE_INSTALL_RPATH)
  if(APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  elseif(NOT WIN32)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
  endif()
endif()

include(LibplistCompilerFlags)

if(LIBPLIST_WITH_SANITIZERS)
  include(LibplistSanitizers)
endif()

# The autotools build adds this define to CFLAGS for fuzzer builds.
if(LIBPLIST_WITH_FUZZERS)
  add_compile_definitions(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
endif()

include(LibplistChecks)

# Configuration summary.
message(STATUS "")
message(STATUS "Configuration for libplist ${LIBPLIST_VERSION}:")
message(STATUS "-------------------------------------------")
message(STATUS "  Install prefix ..........: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build type ..............: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libraries ........: ${BUILD_SHARED_LIBS}")
message(STATUS "  Debug code ..............: ${LIBPLIST_DEBUG}")
message(STATUS "  Python bindings .........: ${LIBPLIST_WITH_CYTHON}")
message(STATUS "  Build tools .............: ${LIBPLIST_WITH_TOOLS}")
message(STATUS "  Build test suite ........: ${LIBPLIST_WITH_TESTS}")
if(LIBPLIST_WITH_SANITIZERS)
  message(STATUS "  Sanitizers ..............: enabled")
endif()
if(LIBPLIST_WITH_FUZZERS)
  message(STATUS "  Build fuzzers ...........: yes")
endif()
message(STATUS "")

enable_testing()

add_subdirectory(libcnary)
add_subdirectory(src)
add_subdirectory(include)

# plistutil requires <getopt.h>, which MSVC does not provide.
if(LIBPLIST_WITH_TOOLS AND HAVE_GETOPT_H)
  add_subdirectory(tools)
elseif(LIBPLIST_WITH_TOOLS)
  message(WARNING
    "<getopt.h> not found: skipping plistutil. Use MinGW/MSYS2 or a POSIX "
    "platform to build the tools; the plistutil-driven CTest scripts will "
    "not be registered.")
endif()

if(LIBPLIST_WITH_TESTS)
  add_subdirectory(test)
endif()

if(LIBPLIST_WITH_CYTHON)
  add_subdirectory(cython)
endif()

if(LIBPLIST_WITH_FUZZERS)
  add_subdirectory(fuzz)
endif()

add_subdirectory(docs)
