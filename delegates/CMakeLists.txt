CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
ADD_DEFINITIONS("-std=c++11")

PROJECT(libdelegates)

SET(QT_MIN_VERSION  "5.2.0")

IF(POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

include(../cmake/extrawarnings.cmake)

INCLUDE_DIRECTORIES(SYSTEM ${Qt5Core_INCLUDES})

SET(libdelegates_SRCS
    categorizeddelegate.cpp
    autocompletiondelegate.cpp
    colordelegate.cpp
)

ADD_LIBRARY( delegates STATIC ${libdelegates_SRCS} )

TARGET_LINK_LIBRARIES( delegates
    ${Qt5Core_LIBRARIES}
)
