# Install the public headers into ${includedir}/plist.

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  FILES_MATCHING PATTERN "*.h"
)
