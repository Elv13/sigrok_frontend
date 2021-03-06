CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
ADD_DEFINITIONS("-std=c++11")

PROJECT(libnodes)

SET(QT_MIN_VERSION  "5.2.0")

IF(POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    ItemModels
    WidgetsAddons
)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    RemoteObjects
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

include(../cmake/extrawarnings.cmake)

# find_package(KChart "2.6.0" REQUIRED) #TODO make optional

SET(libnodes_SRCS
    proxynode.cpp
#     chartnode.cpp
    curvechartnode.cpp
    tablenode.cpp
    meternode.cpp
    columnnode.cpp
    colornode.cpp
    mementonode.cpp
    multiplexernode.cpp
    tailnode.cpp
    headnode.cpp
    currentvalues.cpp
    timernode.cpp
    chrononode.cpp
    lcdmeternode.cpp
    remoteaction.cpp
    deduplicatenode.cpp
    manualacquisitionnode.cpp
    sequencenode.cpp
    monitornode.cpp
    autorangenode.cpp
#     xmlnode.cpp
)

if (QWT_FOUND)
    SET(libnodes_SRCS
        ${libnodes_SRCS}
        scalenode.cpp
            plotnode.cpp
    )
endif()

if(NOT DISABLE_SIGROK)
    SET(libnodes_SRCS
        ${libnodes_SRCS}
        liveacquisitionnode.cpp #FIXME is doesn't have to be there
        devicelistnode.cpp
    )
endif()

ADD_LIBRARY( nodes STATIC ${libnodes_SRCS} )

TARGET_LINK_LIBRARIES( nodes
    common
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Xml
    Qt5::RemoteObjects
    KF5::ItemModels
    KF5::WidgetsAddons
#     KChart
)

if (QWT_FOUND)
    TARGET_LINK_LIBRARIES(nodes
        ${QWT_LIBRARY}
    )
endif()

if(NOT DISABLE_SIGROK)
    TARGET_LINK_LIBRARIES( nodes
        sigrokd
    )
endif()
