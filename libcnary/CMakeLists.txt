# libcnary: internal node list helpers, linked into libplist.
#
# This is a static library so that the test programs (e.g. plist_cmp) can
# also link the node_* functions directly.

add_library(libcnary STATIC
  node.c
  node_list.c
)

target_include_directories(libcnary PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(libcnary PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)
