# Compiler flags and feature defines.

include(CheckCCompilerFlag)

# Additional warnings. MSVC uses a different option syntax, so it is skipped.
if(NOT MSVC)
  set(_libplist_warning_flags
    -Wall
    -Wextra
    -Wredundant-decls
    -Wshadow
    -Wpointer-arith
    -Wwrite-strings
    -Wswitch-default
    -Wno-unused-parameter
    -Wno-strict-aliasing)

  foreach(_flag IN LISTS _libplist_warning_flags)
    string(MAKE_C_IDENTIFIER "LIBPLIST_HAS_${_flag}" _var)
    check_c_compiler_flag("${_flag}" ${_var})
    if(${_var})
      add_compile_options("${_flag}")
    endif()
  endforeach()
endif()

# -fvisibility=hidden is only passed to the C compiler. In the autotools
# build it is appended to GLOBAL_CFLAGS only; the C++ binding must keep
# default visibility so that its classes are exported.
check_c_compiler_flag("-fvisibility=hidden" LIBPLIST_HAS_VISIBILITY_HIDDEN)
if(LIBPLIST_HAS_VISIBILITY_HIDDEN)
  set(HAVE_FVISIBILITY 1)
  add_compile_options("$<$<COMPILE_LANGUAGE:C>:-fvisibility=hidden>")
endif()

# --enable-debug
if(LIBPLIST_DEBUG)
  set(DEBUG 1)
  if(NOT MSVC)
    add_compile_options(-g)
  endif()
endif()

# Static builds define LIBPLIST_STATIC; plist.h then uses no
# dllexport/dllimport.
if(NOT BUILD_SHARED_LIBS)
  set(LIBPLIST_STATIC 1)
  add_compile_definitions(LIBPLIST_STATIC)
endif()

# All sources are compiled with -DHAVE_CONFIG_H, as automake does.
add_compile_definitions(HAVE_CONFIG_H)
