project (tutorial4)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(PkgConfig)

# Disable foreach, slots, signals to avoid collisions
add_definitions(-DQT_NO_KEYWORDS)

PKG_CHECK_MODULES(SIGROK REQUIRED libsigrok)
PKG_CHECK_MODULES(SIGROKCXX REQUIRED libsigrokcxx)

add_subdirectory(qt5-node-editor)

include_directories(${SIGROK_INCLUDE_DIRS})
link_directories(${SIGROK_LIBRARY_DIRS})
add_definitions(${SIGROK_CFLAGS})

include_directories(${SIGROKCXX_INCLUDE_DIRS})
link_directories(${SIGROKCXX_LIBRARY_DIRS})
add_definitions(${SIGROKCXX_CFLAGS})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    RemoteObjects
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    XmlGui
    TextWidgets
    ConfigWidgets
    WidgetsAddons
    SyntaxHighlighting
    ItemModels
    KIO
)

find_package(KChart "2.6.0" REQUIRED)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(tutorial4_SRCS
    # Application
    main.cpp
    mainwindow.cpp
    remotemanager.cpp

    # Model
    devicemodel.cpp
    aquisitionmodel.cpp
    models/remotewidgets.cpp

    # Proxy
    proxies/columnproxy.cpp
    proxies/meterproxy.cpp
    proxies/rangeproxy.cpp
    proxies/coloredrangeproxy.cpp
    proxies/mementoproxy.cpp
    proxies/tailproxy.cpp
    proxies/headproxy.cpp
    proxies/lastrowtolistproxy.cpp
    proxies/chronometerproxy.cpp
    proxies/filtertoplevelproxy.cpp

    # Business delegates
    ratesink.cpp
    nodeadapter.cpp

    # Widgets
    widgets/statusbar.cpp
    widgets/current.cpp
    widgets/aquisition.cpp
    widgets/devicelist.cpp
    widgets/charttype.cpp
    widgets/categorizedtree.cpp
    widgets/meter.cpp
    widgets/column.cpp
    widgets/table.cpp
    widgets/memento.cpp
    widgets/rowsubset.cpp
    widgets/timer.cpp
    widgets/range.cpp

    # Processing nodes
    nodes/abstractnode.cpp
    nodes/devicenode.cpp
    nodes/proxynode.cpp
    nodes/chartnode.cpp
    nodes/tablenode.cpp
    nodes/meternode.cpp
    nodes/columnnode.cpp
    nodes/colornode.cpp
    nodes/mementonode.cpp
    nodes/multiplexernode.cpp
    nodes/tailnode.cpp
    nodes/headnode.cpp
    nodes/currentvalues.cpp
    nodes/timernode.cpp
    nodes/chrononode.cpp
    nodes/remote/remotetable.cpp
    nodes/remote/remotemeter.cpp

    # Delegates
    delegates/categorizeddelegate.cpp
    delegates/autocompletiondelegate.cpp

    # Session
    proxynodefactory.cpp
)

qt5_generate_repc(tutorial4_SRCS
    repc/meterdata.rep
    SOURCE
)
qt5_generate_repc(tutorial4_SRCS
    repc/remotewidgets.rep
    SOURCE
)

ki18n_wrap_ui(tutorial4_SRCS
    mainwindow.ui
    current.ui
    aquisition.ui
    widgets/charttype.ui
    widgets/column.ui
    widgets/meter.ui
    widgets/colorizer.ui
    widgets/devicelist.ui
    widgets/table.ui
    widgets/memento.ui
    widgets/statusbar.ui
    widgets/rowsubset.ui
    widgets/timer.ui
    widgets/rangeselection.ui
    widgets/range.ui
)

add_executable(tutorial4 ${tutorial4_SRCS})

target_link_libraries(tutorial4
    qnodeeditor
    Qt5::Widgets
    Qt5::RemoteObjects
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::TextWidgets
    KF5::ConfigWidgets
    KF5::WidgetsAddons
    KF5::KIOCore
    KF5::SyntaxHighlighting
    KF5::ItemModels
    ${SIGROK_LIBRARIES}
    ${SIGROKCXX_LIBRARIES}
    KChart
)


install(TARGETS tutorial4  ${INSTALL_TARGETS_DEFAULT_ARGS})
