# libplist and libplist++ libraries.
#
# Target names: plist -> libplist-2.0, plistpp -> libplist++-2.0.
# libtool's -version-info 12:1:0 maps to VERSION 12.0.1 / SOVERSION 12.

set(LIBPLIST_SO_VERSION_CURRENT 12)
set(LIBPLIST_SO_VERSION_REVISION 1)
set(LIBPLIST_SO_VERSION_AGE 0)

set(LIBPLIST_C_SOURCES
  base64.c
  bytearray.c
  hashtable.c
  ptrarray.c
  time64.c
  common.c
  xplist.c
  bplist.c
  jsmn.c
  jplist.c
  oplist.c
  out-default.c
  out-plutil.c
  out-limd.c
  plist.c
)

set(LIBPLIST_CXX_SOURCES
  Node.cpp
  Structure.cpp
  Array.cpp
  Boolean.cpp
  Data.cpp
  Date.cpp
  Dictionary.cpp
  Integer.cpp
  Key.cpp
  Real.cpp
  String.cpp
  Uid.cpp
)

# ---------------------------------------------------------------------------
# libplist-2.0
# ---------------------------------------------------------------------------
add_library(plist
  ${LIBPLIST_C_SOURCES}
)

target_include_directories(plist
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libcnary/include
    ${CMAKE_BINARY_DIR}
)

# libcnary provides the node_* functions.
target_link_libraries(plist PRIVATE libcnary)

if(LIBPLIST_MATH_LIBRARY)
  target_link_libraries(plist PRIVATE ${LIBPLIST_MATH_LIBRARY})
endif()

set_target_properties(plist PROPERTIES
  OUTPUT_NAME "plist-2.0"
)

if(BUILD_SHARED_LIBS)
  set_target_properties(plist PROPERTIES
    VERSION "${LIBPLIST_SO_VERSION_CURRENT}.${LIBPLIST_SO_VERSION_AGE}.${LIBPLIST_SO_VERSION_REVISION}"
    SOVERSION "${LIBPLIST_SO_VERSION_CURRENT}"
  )
endif()

libplist_set_output_dirs(plist "${CMAKE_BINARY_DIR}")

install(TARGETS plist
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# libplist++-2.0
# ---------------------------------------------------------------------------
add_library(plistpp
  ${LIBPLIST_CXX_SOURCES}
)

target_include_directories(plistpp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libcnary/include
    ${CMAKE_BINARY_DIR}
)

target_link_libraries(plistpp PUBLIC plist)

set_target_properties(plistpp PROPERTIES
  OUTPUT_NAME "plist++-2.0"
)

if(BUILD_SHARED_LIBS)
  set_target_properties(plistpp PROPERTIES
    VERSION "${LIBPLIST_SO_VERSION_CURRENT}.${LIBPLIST_SO_VERSION_AGE}.${LIBPLIST_SO_VERSION_REVISION}"
    SOVERSION "${LIBPLIST_SO_VERSION_CURRENT}"
  )
endif()

# The C++ headers have no export macro; export all symbols for MSVC DLL builds.
if(MSVC AND BUILD_SHARED_LIBS)
  set_target_properties(plistpp PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# The autotools build passes -static-libgcc for the MinGW DLLs.
if(MINGW AND BUILD_SHARED_LIBS)
  target_link_options(plist PRIVATE -static-libgcc)
  target_link_options(plistpp PRIVATE -static-libgcc)
endif()

libplist_set_output_dirs(plistpp "${CMAKE_BINARY_DIR}")

install(TARGETS plistpp
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ---------------------------------------------------------------------------
# pkg-config files
# ---------------------------------------------------------------------------
set(PACKAGE_NAME "libplist")
set(PACKAGE_VERSION "${LIBPLIST_VERSION}")

set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")

configure_file(
  "${CMAKE_SOURCE_DIR}/src/libplist-2.0.pc.in"
  "${CMAKE_BINARY_DIR}/libplist-2.0.pc"
  @ONLY)

configure_file(
  "${CMAKE_SOURCE_DIR}/src/libplist++-2.0.pc.in"
  "${CMAKE_BINARY_DIR}/libplist++-2.0.pc"
  @ONLY)

install(FILES
  "${CMAKE_BINARY_DIR}/libplist-2.0.pc"
  "${CMAKE_BINARY_DIR}/libplist++-2.0.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
