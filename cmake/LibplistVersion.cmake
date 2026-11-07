# Version detection.
#
# The version is derived from, in order:
#   1. git describe --tags --dirty
#   2. NEWS version with "git-<commit>[-dirty]" appended
#   3. .tarball-version (release tarballs)
#   4. NEWS version (source snapshots without git metadata)
#
# The result is stored in LIBPLIST_VERSION. Pass -DLIBPLIST_VERSION=...
# to override it.

find_package(Git QUIET)

set(LIBPLIST_VERSION "")

# Extract the version from the first "Version ..." line in NEWS. This is
# also used as a fallback when no git metadata is available.
set(_libplist_news_version "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/NEWS")
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/NEWS" _libplist_news_lines
       LIMIT_COUNT 5)
  foreach(_line IN LISTS _libplist_news_lines)
    if(_line MATCHES "^Version[ \t]+([^ \t]+)")
      set(_libplist_news_version "${CMAKE_MATCH_1}")
      break()
    endif()
  endforeach()
endif()

# Use git metadata only when the source tree itself is a git checkout;
# git-version-gen does the same (`test -r "${SRCDIR}/.git"`).
if(NOT LIBPLIST_VERSION AND GIT_FOUND
   AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}"
            describe --tags --dirty
    OUTPUT_VARIABLE _libplist_describe
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _libplist_describe_result
    ERROR_QUIET)

  if(_libplist_describe_result EQUAL 0 AND _libplist_describe)
    set(LIBPLIST_VERSION "${_libplist_describe}")
  elseif(_libplist_news_version)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}"
              rev-parse --short HEAD
      OUTPUT_VARIABLE _libplist_commit
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE _libplist_commit_result
      ERROR_QUIET)

    if(_libplist_commit_result EQUAL 0 AND _libplist_commit)
      # Append "-dirty" when the working tree has uncommitted changes.
      execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}"
                diff --quiet HEAD
        RESULT_VARIABLE _libplist_diff_result
        ERROR_QUIET)
      set(_libplist_dirty "")
      if(NOT _libplist_diff_result EQUAL 0)
        set(_libplist_dirty "-dirty")
      endif()
      set(LIBPLIST_VERSION
          "${_libplist_news_version}-git-${_libplist_commit}${_libplist_dirty}")
    endif()
  endif()
endif()

# Release tarballs carry the version in .tarball-version.
if(NOT LIBPLIST_VERSION AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.tarball-version")
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.tarball-version" _libplist_tarball_version)
  string(STRIP "${_libplist_tarball_version}" _libplist_tarball_version)
  if(_libplist_tarball_version)
    set(LIBPLIST_VERSION "${_libplist_tarball_version}")
  endif()
endif()

# Fallback for source snapshots without git metadata.
if(NOT LIBPLIST_VERSION AND _libplist_news_version)
  set(LIBPLIST_VERSION "${_libplist_news_version}")
endif()

if(NOT LIBPLIST_VERSION)
  message(FATAL_ERROR
    "PACKAGE_VERSION is not defined. Make sure to configure a source tree "
    "checked out from git or that .tarball-version is present.")
endif()

message(STATUS "libplist version: ${LIBPLIST_VERSION}")
