project (tutorial4)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

IF(POLICY CMP0063)
   CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

set(CMAKE_BUILD_TYPE DEBUG)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(PkgConfig REQUIRED)

# Disable foreach, slots, signals to avoid collisions
add_definitions(-DQT_NO_KEYWORDS)

PKG_CHECK_MODULES(SIGROK REQUIRED libsigrok)
PKG_CHECK_MODULES(SIGROKCXX REQUIRED libsigrokcxx)


include_directories(${SIGROK_INCLUDE_DIRS})
link_directories(${SIGROK_LIBRARY_DIRS})
add_definitions(${SIGROK_CFLAGS})

add_definitions(-fexceptions)

include_directories(${SIGROKCXX_INCLUDE_DIRS})
link_directories(${SIGROKCXX_LIBRARY_DIRS})
add_definitions(${SIGROKCXX_CFLAGS})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

add_subdirectory(qt5-node-editor)
add_subdirectory(delegates)
add_subdirectory(common)
add_subdirectory(proxies)
add_subdirectory(nodes)
add_subdirectory(widgets)
add_subdirectory(sigrokd)

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
    configdialog.cpp

    # Model
    models/remotewidgets.cpp

    # Business delegates
    ratesink.cpp
    nodeadapter.cpp

    # Session
    proxynodefactory.cpp
)

ki18n_wrap_ui(tutorial4_SRCS
    mainwindow.ui
    generalconfig.ui
)

kconfig_add_kcfg_files(tutorial4_SRCS data/tutorial4Settings.kcfgc)

add_executable(tutorial4 ${tutorial4_SRCS})

target_link_libraries(tutorial4
    KF5::XmlGui
    qnodeeditor
    extraproxies
    nodes
    nodewidgets
    sigrokd
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
