SET( libplist_HDR
	${CMAKE_CURRENT_SOURCE_DIR}/plist/plist.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/plist++.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Array.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Boolean.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Data.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Date.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Dictionary.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Integer.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Node.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Real.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/String.h
	${CMAKE_CURRENT_SOURCE_DIR}/plist/Structure.h
  )

INSTALL( FILES ${libplist_HDR}
   DESTINATION include/plist COMPONENT dev)
