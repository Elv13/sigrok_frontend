CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
ADD_DEFINITIONS("-std=c++11")

PROJECT(desktop)

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
    OpenGL
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    XmlGui
    ConfigWidgets
    WidgetsAddons
    ItemModels
    KIO
)

SET(desktop_SRCS
    mainwindow.cpp
    configdialog.cpp
    docktitle.cpp
    toolbox.cpp
)

QT5_ADD_RESOURCES(desktop_SRCS
    desktop.qrc
)

QT5_WRAP_UI(desktop_SRCS
    mainwindow.ui
    generalconfig.ui
    docktitle.ui
    toolbox.ui
)

kconfig_add_kcfg_files(desktop_SRCS ../data/tutorial4Settings.kcfgc)

ADD_LIBRARY( desktop STATIC ${desktop_SRCS} )

TARGET_LINK_LIBRARIES( desktop
#     sigrokd
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Widgets
    Qt5::RemoteObjects
    Qt5::OpenGL
    qnodeeditor
    extraproxies
    nodes
    nodewidgets
    KF5::XmlGui
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::ConfigWidgets
    KF5::KIOCore
)
