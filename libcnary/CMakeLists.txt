cmake_minimum_required(VERSION 2.6)

SET(libcnary_SRC
	iterator.c
	list.c
	node.c
	node_iterator.c
	node_list.c )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ADD_LIBRARY(libcnary STATIC ${libcnary_SRC})

SET_TARGET_PROPERTIES(libcnary PROPERTIES OUTPUT_NAME cnary)

