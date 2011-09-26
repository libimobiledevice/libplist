
INCLUDE_DIRECTORIES( ${LIBXML2_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libcnary/include )
 

SET(libplist_SRC
	plist.c
	hashtable.c
	bytearray.c
	ptrarray.c
	bplist.c
	base64.c
	xplist.c )

SET(libplist++_SRC
	Node.cpp
	Boolean.cpp
	Integer.cpp
	Real.cpp
	String.cpp
	Date.cpp
	Data.cpp
	Structure.cpp
	Array.cpp
	Dictionary.cpp
	)

SET(libcnary_object_files
  ${CMAKE_BINARY_DIR}/libcnary/CMakeFiles/libcnary.dir/iterator.c${CMAKE_C_OUTPUT_EXTENSION}
  ${CMAKE_BINARY_DIR}/libcnary/CMakeFiles/libcnary.dir/list.c${CMAKE_C_OUTPUT_EXTENSION}
  ${CMAKE_BINARY_DIR}/libcnary/CMakeFiles/libcnary.dir/node.c${CMAKE_C_OUTPUT_EXTENSION}
  ${CMAKE_BINARY_DIR}/libcnary/CMakeFiles/libcnary.dir/node_iterator.c${CMAKE_C_OUTPUT_EXTENSION}
  ${CMAKE_BINARY_DIR}/libcnary/CMakeFiles/libcnary.dir/node_list.c${CMAKE_C_OUTPUT_EXTENSION}
)

SET_SOURCE_FILES_PROPERTIES(${libcnary_object_files} PROPERTIES EXTERNAL_OBJECT true GENERATED true)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPLIST_BYTE_ORDER=${PLIST_BYTE_ORDER}")
ADD_LIBRARY( plist SHARED ${libplist_SRC} ${libcnary_object_files} )
TARGET_LINK_LIBRARIES( plist ${LIBXML2_LIBRARIES} )
SET_TARGET_PROPERTIES( plist PROPERTIES VERSION ${LIBPLIST_LIBVERSION} )
SET_TARGET_PROPERTIES( plist PROPERTIES SOVERSION ${LIBPLIST_SOVERSION} )
if(APPLE)
  SET_TARGET_PROPERTIES( plist PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

ADD_LIBRARY( plist++ SHARED ${libplist++_SRC} )
TARGET_LINK_LIBRARIES( plist++ plist )
SET_TARGET_PROPERTIES( plist++ PROPERTIES VERSION ${LIBPLIST_LIBVERSION} )
SET_TARGET_PROPERTIES( plist++ PROPERTIES SOVERSION ${LIBPLIST_SOVERSION} )
if(APPLE)
  SET_TARGET_PROPERTIES( plist++ PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

INSTALL(TARGETS plist plist++
 RUNTIME DESTINATION bin COMPONENT lib
 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev
 ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)
