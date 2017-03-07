CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
ADD_DEFINITIONS("-std=c++11")

PROJECT(libnodewidgets)

SET(QT_MIN_VERSION  "5.2.0")

IF(POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

INCLUDE_DIRECTORIES(SYSTEM ${Qt5Core_INCLUDES})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    ItemModels
    CoreAddons
    WidgetsAddons
)

if (QWT_FOUND)
    find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
        Svg
        OpenGL
        Concurrent
    )

    if (BUILD_MOBILE)
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/lib /libs/armeabi-v7a/)
        set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /usr/local/qwt-6.1.3/include/)
        include_directories(/usr/local/qwt-6.1.3/include/) #FIXME
    endif()
endif()
set(CMAKE_AUTOMOC ON)

# find_package(KChart "2.6.0" REQUIRED) #TODO make optional

include(../cmake/extrawarnings.cmake)

SET(libnodewidgets_SRCS
    statusbar.cpp
    current.cpp
    acquisition.cpp
    devicelist.cpp
#     charttype.cpp
    categorizedtree.cpp
    meter.cpp
    column.cpp
    table.cpp
    memento.cpp
    rowsubset.cpp
    range.cpp
    lcdmeter.cpp
    controls.cpp
    curvechart.cpp
    sequence.cpp
    monitor.cpp
    plot.cpp
)

if (NOT BUILD_MOBILE)
    SET(libnodewidgets_SRCS ${libnodewidgets_SRCS}
        chrono.cpp
        timer.cpp
        deduplicate.cpp
    )
endif()

QT5_WRAP_UI(libnodewidgets_SRCS
#     charttype.ui
    column.ui
    meter.ui
    colorizer.ui
    devicelist.ui
    table.ui
    memento.ui
    statusbar.ui
    rowsubset.ui
    timer.ui
    rangeselection.ui
    range.ui
    current.ui
    acquisition.ui
    lcdmeter.ui
    controls.ui
    deduplicate.ui
    sequence.ui
    chrono.ui
    monitor.ui
)

# QT5_ADD_RESOURCES(libnodewidgets_SRCS
#     assets/assets.qrc
# )

ADD_LIBRARY( nodewidgets STATIC ${libnodewidgets_SRCS} )

TARGET_LINK_LIBRARIES( nodewidgets
    delegates
    ColorWidgets-qt5
    common
    extraproxies
    Qt5::Widgets
    Qt5::Core
    KF5::ItemModels
#     KChart
)

if (QWT_FOUND)
    TARGET_LINK_LIBRARIES(nodewidgets
        ${QWT_LIBRARY}
        Qt5::Svg
        Qt5::OpenGL
        Qt5::Concurrent
    )
endif()

