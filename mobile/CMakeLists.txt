CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
ADD_DEFINITIONS("-std=c++11")

PROJECT(mobile)

SET(QT_MIN_VERSION  "5.2.0")

IF(POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

INCLUDE_DIRECTORIES(SYSTEM ${Qt5Core_INCLUDES})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})


include(../cmake/extrawarnings.cmake)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Network
)


if (QWT_FOUND)
    find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
        Svg
        OpenGL
        Concurrent
        Quick
        QuickWidgets
        QuickControls2
    )
endif()

SET(mobile_SRCS
    mainwindow.cpp
)

QT5_WRAP_UI(mobile_SRCS
    mainwindow.ui
    appbar.ui
    titlebar.ui
)

QT5_ADD_RESOURCES(mobile_SRCS
    mobile.qrc
)

QT5_ADD_RESOURCES(mobile_SRCS
    icons/icons.qrc
)

ADD_LIBRARY( mobile STATIC ${mobile_SRCS} )

TARGET_LINK_LIBRARIES( mobile
    Qt5::Widgets
    Qt5::Gui
    Qt5::Quick
    Qt5::QuickWidgets
    Qt5::Network
    Qt5::QuickControls2
    common
    nodewidgets
)

if (QWT_FOUND)
    TARGET_LINK_LIBRARIES(mobile
        ${QWT_LIBRARY}
        Qt5::Svg
        Qt5::OpenGL
        Qt5::Concurrent
    )
endif()
