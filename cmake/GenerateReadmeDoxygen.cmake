# Generate docs/README.doxygen.md from README.md.
#
# This replaces the awk rule in the top-level Makefile.am: the directory
# list under "## Table of Contents" is replaced by a "[TOC]" placeholder.
#
# Usage: cmake -DINPUT=<README.md> -DOUTPUT=<README.doxygen.md> -P <this file>

if(NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
  message(FATAL_ERROR "GenerateReadmeDoxygen.cmake requires -DINPUT=<file> and -DOUTPUT=<file>")
endif()

file(READ "${INPUT}" _content)

set(_marker "## Table of Contents")
string(FIND "${_content}" "${_marker}" _pos)

if(_pos EQUAL -1)
  file(WRITE "${OUTPUT}" "${_content}")
else()
  string(LENGTH "${_marker}" _marker_len)
  math(EXPR _title_end "${_pos} + ${_marker_len}")

  # Find the first blank line after the heading; the list ends there.
  # (Avoid the start argument of string(FIND) for broader CMake compatibility.)
  string(SUBSTRING "${_content}" "${_title_end}" -1 _rest)
  string(FIND "${_rest}" "\n\n" _blank_rel)

  if(_blank_rel EQUAL -1)
    string(SUBSTRING "${_content}" 0 "${_title_end}" _head)
    file(WRITE "${OUTPUT}" "${_head}\n[TOC]\n")
  else()
    string(SUBSTRING "${_content}" 0 "${_title_end}" _head)
    string(APPEND _head "\n")
    math(EXPR _after_blank "${_blank_rel} + 1")
    string(SUBSTRING "${_rest}" "${_after_blank}" -1 _tail)
    file(WRITE "${OUTPUT}" "${_head}[TOC]\n${_tail}")
  endif()
endif()
