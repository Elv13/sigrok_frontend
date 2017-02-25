CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
ADD_DEFINITIONS("-std=c++11")

PROJECT(libextraproxies)

SET(QT_MIN_VERSION  "5.2.0")

IF(POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

INCLUDE_DIRECTORIES(SYSTEM ${Qt5Core_INCLUDES})

include(../cmake/extrawarnings.cmake)

#WARNING: If this is enabled, the resulting binary is GPL
# find_package(KChart "2.6.0")

SET(libextraproxies_SRCS
    columnproxy.cpp
    meterproxy.cpp
    rangeproxy.cpp
    coloredrangeproxy.cpp
    mementoproxy.cpp
    tailproxy.cpp
    headproxy.cpp
    lastrowtolistproxy.cpp
    chronometerproxy.cpp
    filtertoplevelproxy.cpp
    rowsdeletionproxy.cpp
    deduplicateproxy.cpp
    vectorizingproxy.cpp
)

ADD_LIBRARY( extraproxies STATIC ${libextraproxies_SRCS} )

TARGET_LINK_LIBRARIES( extraproxies
    ${Qt5Core_LIBRARIES}
#     KChart
)
