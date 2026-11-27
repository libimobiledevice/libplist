# Build options. These mirror the autotools configure switches.

option(LIBPLIST_WITH_TOOLS
  "Build the libplist tools (plistutil)."
  ON)

# autoconf: build_tests defaults to ${build_tools}, i.e. the test suite is
# only enabled when plistutil is built.
option(LIBPLIST_WITH_TESTS
  "Build the libplist test suite (defaults to LIBPLIST_WITH_TOOLS)."
  ${LIBPLIST_WITH_TOOLS})

option(LIBPLIST_DEBUG
  "Build debug message output code."
  OFF)

option(LIBPLIST_WITH_CYTHON
  "Build the Python bindings using Cython, if available."
  ON)

option(LIBPLIST_WITH_SANITIZERS
  "Build with AddressSanitizer/UndefinedBehaviorSanitizer."
  OFF)

option(LIBPLIST_WITH_FUZZERS
  "Build the fuzzers (implies sanitizers)."
  OFF)

# Default to shared libraries, matching the default libtool behavior.
# (option() keeps a value passed with -DBUILD_SHARED_LIBS=... on the
# command line.)
option(BUILD_SHARED_LIBS "Build shared libraries (OFF builds static libraries)." ON)

# --with-fuzzers implies --with-sanitizers.
if(LIBPLIST_WITH_FUZZERS AND NOT LIBPLIST_WITH_SANITIZERS)
  set(LIBPLIST_WITH_SANITIZERS ON CACHE BOOL
    "Build with AddressSanitizer/UndefinedBehaviorSanitizer." FORCE)
endif()

# The test suite requires plistutil.
if(LIBPLIST_WITH_TESTS AND NOT LIBPLIST_WITH_TOOLS)
  message(FATAL_ERROR
    "Building the test suite requires plistutil. "
    "Reconfigure with -DLIBPLIST_WITH_TOOLS=ON, "
    "or pass -DLIBPLIST_WITH_TESTS=OFF.")
endif()
