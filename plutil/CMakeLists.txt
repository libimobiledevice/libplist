
SET(plutil_SRC
	plutil.c)

ADD_EXECUTABLE(plutil ${plutil_SRC})
TARGET_LINK_LIBRARIES(plutil plist)

INSTALL( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/plutil
   DESTINATION bin )
