CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
ADD_DEFINITIONS("-std=c++11")

PROJECT(libsigrokd)

SET(QT_MIN_VERSION  "5.2.0")

find_package(PkgConfig REQUIRED)

IF(POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

add_definitions(-DQT_NO_KEYWORDS)

include(../cmake/extrawarnings.cmake)

PKG_CHECK_MODULES(SIGROK REQUIRED libsigrok)
PKG_CHECK_MODULES(SIGROKCXX REQUIRED libsigrokcxx)
include_directories(${SIGROK_INCLUDE_DIRS})
link_directories(${SIGROK_LIBRARY_DIRS})
add_definitions(${SIGROK_CFLAGS})
add_definitions(-fexceptions)
include_directories(${SIGROKCXX_INCLUDE_DIRS})
link_directories(${SIGROKCXX_LIBRARY_DIRS})
add_definitions(${SIGROKCXX_CFLAGS})

INCLUDE_DIRECTORIES(SYSTEM ${Qt5Core_INCLUDES})

SET(libsigrokd_SRCS
    devicemodel.cpp
    aquisitionmodel.cpp
    sigrokdevice.cpp
)

ADD_LIBRARY( sigrokd STATIC ${libsigrokd_SRCS} )

TARGET_LINK_LIBRARIES( sigrokd
    common
    Qt5::Core
    ${SIGROK_LIBRARIES}
    ${SIGROKCXX_LIBRARIES}
)
