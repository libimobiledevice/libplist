cmake_minimum_required(VERSION 3.8)


set(PROJECT_VERSION_MAJOR	2)
set(PROJECT_VERSION_MINOR	2)
set(PROJECT_VERSION_PATCH	1)
set(PROJECT_VERSION_SUFFIX	2.0)
set(PROJECT_NAME_CC		"plist")
set(PROJECT_NAME_CXX		"plist++")
project(plist VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})


message(STATUS "Operation System : ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_INSTALL_PREFIX : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_C_COMPILER : ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER : ${CMAKE_CXX_COMPILER}")


set(PLIST_URL_PREFIX		"https://github.com/libimobiledevice/libplist")
set(PACKAGE_NAME		"${PROJECT_NAME}")
set(PACKAGE_VERSION		"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PACKAGE_URL			"${PLIST_URL_PREFIX}")
set(PACKAGE_BUGREPORT		"${PLIST_URL_PREFIX}/issues")

add_definitions(-DPACKAGE_NAME="${PROJECT_NAME}")
add_definitions(-DPACKAGE_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
add_definitions(-DPACKAGE_URL="${PLIST_URL_PREFIX}")
add_definitions(-DPACKAGE_BUGREPORT="${PLIST_URL_PREFIX}/issues")

message(STATUS "PACKAGE_NAME : ${PACKAGE_NAME}")
message(STATUS "PACKAGE_VERSION : ${PACKAGE_VERSION}")
message(STATUS "PACKAGE_URL : ${PACKAGE_URL}")
message(STATUS "PACKAGE_BUGREPORT : ${PACKAGE_BUGREPORT}")


if (BUILD_SHARED_LIBS STREQUAL "OFF" OR BUILD_STATIC_LIBS STREQUAL "ON")
	set(IDM_PLIST_LIBRARY_TYPE "STATIC")
else()
	set(IDM_PLIST_LIBRARY_TYPE "SHARED")
endif ()
configure_file("${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME_CC}-config.cmake.in" "${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CC}-${PROJECT_VERSION_SUFFIX}-config.cmake")
configure_file("${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME_CC}.pc.in" "${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CC}-${PROJECT_VERSION_SUFFIX}.pc")
configure_file("${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME_CXX}-config.cmake.in" "${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CXX}-${PROJECT_VERSION_SUFFIX}-config.cmake")
configure_file("${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME_CXX}.pc.in" "${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CXX}-${PROJECT_VERSION_SUFFIX}.pc")
configure_file("${CMAKE_SOURCE_DIR}/config/VersionInfo.rc.in" "${CMAKE_BINARY_DIR}/VersionInfo.rc")


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_INSTALL_PREFIX}/lib/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_MODULE_PATH})
message(STATUS "CMAKE_MODULE_PATH : ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")


if (MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra -Wredundant-decls -Wshadow -Wpointer-arith  -Wwrite-strings -Wswitch-default -Wno-unused-parameter -Wno-strict-aliasing -fvisibility=hidden)
endif()
if (IDM_PLIST_LIBRARY_TYPE STREQUAL "STATIC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -static")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -static")
endif()
message(STATUS "CMAKE_CXX_FLAGS_DEBUG : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE : ${CMAKE_CXX_FLAGS_RELEASE}")


include(TestBigEndian)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckVariableExists)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckCSourceCompiles)
include(CheckPrototypeDefinition)

set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} time.h)
set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE=1)
CHECK_TYPE_SIZE("((struct tm*)0)->tm_gmtoff"		HAVE_TM_TM_GMTOFF)
CHECK_TYPE_SIZE("((struct tm*)0)->tm_zone"		HAVE_TM_TM_ZONE)

CHECK_SYMBOL_EXISTS(asprintf		"stdio.h"	HAVE_ASPRINTF)
CHECK_SYMBOL_EXISTS(vasprintf		"stdio.h"	HAVE_VASPRINTF)
CHECK_SYMBOL_EXISTS(strerror		"stdio.h"	HAVE_STRERROR)
CHECK_SYMBOL_EXISTS(strcasecmp		"strings.h"	HAVE_STRCASECMP)
CHECK_SYMBOL_EXISTS(strdup		"string.h"	HAVE_STRDUP)
CHECK_SYMBOL_EXISTS(strndup		"string.h"	HAVE_STRNDUP)
CHECK_SYMBOL_EXISTS(stpcpy		"string.h"	HAVE_STPCPY)
CHECK_SYMBOL_EXISTS(memmem		"string.h"	HAVE_MEMMEM)
CHECK_SYMBOL_EXISTS(gmtime_r		"time.h"	HAVE_GMTIME_R)
CHECK_SYMBOL_EXISTS(localtime_r		"time.h"	HAVE_LOCALTIME_R)
CHECK_SYMBOL_EXISTS(timegm		"time.h"	HAVE_TIMEGM)
CHECK_SYMBOL_EXISTS(strptime		"time.h"	HAVE_STRPTIME)

if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
	message(STATUS "CMAKE_HOST_SYSTEM_NAME MATCHES Windows")
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Mingw")
	message(STATUS "CMAKE_HOST_SYSTEM_NAME MATCHES Mingw")
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Cygwin")
	message(STATUS "CMAKE_HOST_SYSTEM_NAME MATCHES Cygwin")
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Android")
	message(STATUS "CMAKE_HOST_SYSTEM_NAME MATCHES Android")
elseif(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "CMAKE_HOST_SYSTEM_NAME MATCHES Darwin")
else()
	message(STATUS "CMAKE_HOST_SYSTEM_NAME MATCHES Other")
	add_definitions(-DHAVE_PTHREAD_ONCE=1)
	set (HAVE_PTHREAD_ONCE 1)
endif()


if(HAVE_TM_TM_GMTOFF)
	add_definitions(-DHAVE_TM_TM_GMTOFF=1)
	set (HAVE_TM_TM_GMTOFF 1)
endif()
if(HAVE_TM_TM_ZONE)
	add_definitions(-DHAVE_TM_TM_ZONE=1)
	set (HAVE_TM_TM_ZONE 1)
endif()
if(HAVE_STRNDUP)
	add_definitions(-DHAVE_STRNDUP=1)
	set (HAVE_STRNDUP 1)
endif()

test_big_endian(isBigEndian)
if (${isBigEndian})
	add_definitions(-D__BIG_ENDIAN__=1)
	set (__BIG_ENDIAN__ 1)
else()
	add_definitions(-D__LITTLE_ENDIAN__=1)
	set (__LITTLE_ENDIAN__ 1)
endif()


set(CMAKE_IGNORE_PATH ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
message(STATUS "CMAKE_IGNORE_PATH : ${CMAKE_IGNORE_PATH}")
message(STATUS "CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")


include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/libcnary/include)


link_directories(${CMAKE_BINARY_DIR}/src)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
endif ()
if (${CMAKE_C_COMPILER_ID} MATCHES "GNU")
	add_definitions(-DHAVE_FVISIBILITY=1)
elseif (${CMAKE_C_COMPILER_ID} MATCHES "Clang")
	add_definitions(-DHAVE_FVISIBILITY=1)
endif ()
message(STATUS "CMAKE_C_COMPILER_ID : ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE_HOST_SYSTEM_NAME : ${CMAKE_HOST_SYSTEM_NAME}")


set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
add_subdirectory(${PROJECT_SOURCE_DIR}/libcnary)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/tools)


install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CC}-${PROJECT_VERSION_SUFFIX}-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake)
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CC}-${PROJECT_VERSION_SUFFIX}.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CXX}-${PROJECT_VERSION_SUFFIX}-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake)
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME_CXX}-${PROJECT_VERSION_SUFFIX}.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
