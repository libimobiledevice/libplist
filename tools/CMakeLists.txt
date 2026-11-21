# plistutil command line tool.

add_executable(plistutil plistutil.c)

target_include_directories(plistutil PRIVATE
  "${CMAKE_SOURCE_DIR}/include"
  "${CMAKE_BINARY_DIR}"
)

target_link_libraries(plistutil PRIVATE plist)

libplist_set_output_dirs(plistutil "${CMAKE_BINARY_DIR}/tools")

install(TARGETS plistutil
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# Install plist2json as an alias of plistutil. The existing link (from a
# previous install) is removed first so that repeating "make install" works.
if(WIN32)
  install(CODE "
    file(REMOVE
      \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/plist2json.exe\")
    file(CREATE_LINK
      \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/plistutil.exe\"
      \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/plist2json.exe\"
      SYMBOLIC COPY_ON_ERROR)
  ")
else()
  install(CODE "
    file(REMOVE
      \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/plist2json\")
    file(CREATE_LINK plistutil
      \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/plist2json\"
      SYMBOLIC COPY_ON_ERROR)
  ")
endif()
