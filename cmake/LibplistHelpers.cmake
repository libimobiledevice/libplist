# Helper functions.

# Give targets a fixed output directory. The test scripts expect the tools
# and test programs at $top_builddir/tools and $top_builddir/test, so this
# also covers multi-config generators (Visual Studio, Xcode).
function(libplist_set_output_dirs target dir)
  set_target_properties(${target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${dir}"
    LIBRARY_OUTPUT_DIRECTORY "${dir}"
    ARCHIVE_OUTPUT_DIRECTORY "${dir}")
  foreach(_cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${_cfg}" _CFG)
    set_target_properties(${target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY_${_CFG} "${dir}"
      LIBRARY_OUTPUT_DIRECTORY_${_CFG} "${dir}"
      ARCHIVE_OUTPUT_DIRECTORY_${_CFG} "${dir}")
  endforeach()
endfunction()

# Convert a path to CMake style (forward slashes), for use in shell scripts.
function(libplist_to_posix_path out_var path)
  file(TO_CMAKE_PATH "${path}" _libplist_posix)
  set(${out_var} "${_libplist_posix}" PARENT_SCOPE)
endfunction()
