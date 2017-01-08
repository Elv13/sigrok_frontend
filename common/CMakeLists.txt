CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
ADD_DEFINITIONS("-std=c++11")

PROJECT(libcommon)

SET(QT_MIN_VERSION  "5.2.0")

IF(POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

INCLUDE_DIRECTORIES(SYSTEM ${Qt5Core_INCLUDES})

include(../cmake/extrawarnings.cmake)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    RemoteObjects
)

SET(libcommon_SRCS
    pagemanager.cpp
    abstractnode.cpp
    remotemanager.cpp
)

qt5_generate_repc(libcommon_SRCS
    repc/meterdata.rep
    SOURCE
)
qt5_generate_repc(libcommon_SRCS
    repc/device.rep
    SOURCE
)

qt5_generate_repc(libcommon_SRCS
    repc/meterdata.rep
    REPLICA
)
qt5_generate_repc(libcommon_SRCS
    repc/device.rep
    REPLICA
)

ADD_LIBRARY( common STATIC ${libcommon_SRCS} )

TARGET_LINK_LIBRARIES( common
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::RemoteObjects
)
