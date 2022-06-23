cmake_minimum_required(VERSION 3.8)


project(cnary VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})


if (MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall)
endif()
if (IDM_PLIST_LIBRARY_TYPE STREQUAL "STATIC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -static")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -static")
endif()


set(CMAKE_IGNORE_PATH ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
message(STATUS "CMAKE_IGNORE_PATH : ${CMAKE_IGNORE_PATH}")


include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})


set(CMAKE_C_STANDARD 11)


add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)


aux_source_directory(${PROJECT_SOURCE_DIR} SRC)


add_library(${PROJECT_NAME}-${PROJECT_VERSION_SUFFIX} STATIC ${SRC})


set_target_properties(${PROJECT_NAME}-${PROJECT_VERSION_SUFFIX} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-${PROJECT_VERSION_SUFFIX} PROPERTIES DEBUG_POSTFIX -d)
