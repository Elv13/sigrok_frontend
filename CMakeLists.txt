project (tutorial4)

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

ADD_DEFINITIONS("-std=c++14")
ADD_DEFINITIONS("-DCLAZY_CHECKS='level2,no-qenums'")

IF(POLICY CMP0063)
   CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

# For the mobile build
if (BUILD_MOBILE)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/lib /libs/armeabi-v7a/)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /usr/local/qwt-6.1.3/include/)
    include_directories(/usr/local/qwt-6.1.3/include/) #FIXME
endif()

set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

set(CMAKE_BUILD_TYPE DEBUG)

#add_subdirectory(kf5) #FIXME check for the frameworks or build them

find_package(ECM 1.0.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR}
    ${ECM_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

include(KDEInstallDirs)
include(KDECMakeSettings)
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

find_package(Qwt OPTIONAL_COMPONENTS)

add_subdirectory(common)

if (BUILD_MOBILE)
    set(DISABLE_SIGROK 1)
endif()

if (QWT_FOUND)
    add_definitions(-DWITH_QWT)
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
add_subdirectory(Qt-Color-Widgets)
add_subdirectory(delegates)
add_subdirectory(proxies)

add_subdirectory(widgets)

# Build the desktop centric (KDE) GUI
if (NOT BUILD_MOBILE)
    include(KDECompilerSettings)

    add_subdirectory(nodes)
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

    # Business delegates
#     ratesink.cpp
)


if (NOT BUILD_MOBILE) #FIXME
    set(tutorial4_SRCS ${tutorial4_SRCS}
        # Session
        session.cpp
        columnserializationadapter.cpp
    )
endif()

add_executable(tutorial4 ${tutorial4_SRCS})

target_link_libraries(tutorial4
    qnodeeditor
    extraproxies
    Qt5::RemoteObjects
    KF5::CoreAddons
)

if (NOT BUILD_MOBILE)
    target_link_libraries(tutorial4
        desktop
        sigrokd
        nodes
        nodewidgets
    )
endif()

if (BUILD_MOBILE)
    target_link_libraries(tutorial4
        mobile
        common
    )
endif()


if (BUILD_MOBILE)
    install(TARGETS tutorial4 RUNTIME DESTINATION bin)
else()
    install(TARGETS tutorial4  ${INSTALL_TARGETS_DEFAULT_ARGS})
    install( FILES data/tutorial4ui.rc DESTINATION  ${KXMLGUI_INSTALL_DIR}/tutorial4 )
endif()
