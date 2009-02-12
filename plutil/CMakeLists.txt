
SET(plutil_SRC
	plutil.c)

ADD_EXECUTABLE(plutil ${plutil_SRC})
TARGET_LINK_LIBRARIES(plutil plist)

INSTALL( TARGETS plutil RUNTIME DESTINATION bin COMPONENT plutil )