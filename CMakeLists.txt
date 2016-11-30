project (tutorial4)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

IF(POLICY CMP0063)
   CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

set(CMAKE_BUILD_TYPE DEBUG)

#add_subdirectory(kf5) #FIXME check for the frameworks or build them

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

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
    WidgetsAddons
    ItemModels
)

add_subdirectory(common)

if (BUILD_MOBILE)
    set(DISABLE_SIGROK 1)
endif()

# SIGROK is GPL and cannot be shipped in the mobile binary
if(NOT DISABLE_SIGROK)
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

    add_subdirectory(sigrokd)
endif()

add_subdirectory(qt5-node-editor)
add_subdirectory(delegates)
add_subdirectory(proxies)
add_subdirectory(nodes)
add_subdirectory(widgets)

# Build the desktop centric (KDE) GUI
if (NOT BUILD_MOBILE)
    add_subdirectory(desktop)
endif()

# Build the Android / iOS companion app
if (BUILD_MOBILE)
    add_subdirectory(mobile)
endif()

# find_package(KChart "2.6.0" REQUIRED)

include(cmake/extrawarnings.cmake)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(tutorial4_SRCS
    # Application
    main.cpp

    # Model
    models/remotewidgets.cpp

    # Session
    proxynodefactory.cpp
    columnserializationadapter.cpp

    # Business delegates
#     ratesink.cpp
)

add_executable(tutorial4 ${tutorial4_SRCS})

target_link_libraries(tutorial4
    qnodeeditor
    extraproxies
    nodes
    nodewidgets
    Qt5::RemoteObjects
    KF5::CoreAddons
)

if (NOT BUILD_MOBILE)
    target_link_libraries(tutorial4
        desktop
        sigrokd
    )
endif()

if (BUILD_MOBILE)
    target_link_libraries(tutorial4
        mobile
    )
endif()


install(TARGETS tutorial4  ${INSTALL_TARGETS_DEFAULT_ARGS})
INSTALL( FILES data/tutorial4ui.rc DESTINATION  ${KXMLGUI_INSTALL_DIR}/tutorial4 )
