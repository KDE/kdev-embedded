#add_definitions(-DTRANSLATION_DOMAIN=\"kdevembedded\")
project(embedded)

set(ARDUINO_SDK_VERSION "1610")
set(ARDUINO_SDK_VERSION_NAME "1.6.10")
set(ARDUINO_SDK_MIN_VERSION_NAME "1.6.8")

set(CMAKE_CXX_STANDARD 11)

cmake_minimum_required(VERSION 3.5)
include(CMakeFindDependencyMacro)
########### install target ###############

set(CMAKE_AUTOUIC ON)

find_package(ECM REQUIRED)
find_package(KDevPlatform REQUIRED 5.0)

set(CMAKE_MODULE_PATH ${KDevPlatform_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/launcher)

set(KDEVEMBEDDED_PART_SRCS
    embedded.cpp
    toolkit.cpp
    board.cpp
    firsttimewizard.cpp
    arduinowindowmodel.cpp
)

set(KDEVEMBEDDED_LAUNCHER_PART_SRCS
    launcher/projecttargetscombobox.cpp
    launcher/executeplugin.cpp
    launcher/embeddedlauncher.cpp
    launcher/launcherjob.cpp
    arduinowindowmodel.cpp
    firsttimewizard.cpp
    toolkit.cpp
    board.cpp
)

set(KDEVEMBEDDED_LAUNCHER_PART_UIS
    launcher/embeddedlauncher.ui
    firsttimewizard.ui
)

add_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

find_package(KF5 REQUIRED Archive I18n NewStuff ItemModels TextEditor ThreadWeaver)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

qt5_add_resources(KDEVEMBEDDED_PART_SRCS kdevembedded.qrc)

kdevplatform_add_plugin(kdevembedded
                        JSON kdevembedded.json
                        SOURCES ${KDEVEMBEDDED_PART_SRCS}
)
kdevplatform_add_plugin(kdevembedded-launcher
                        JSON launcher/kdevembedded-launcher.json
                        SOURCES ${KDEVEMBEDDED_LAUNCHER_PART_SRCS}
)

target_link_libraries(kdevembedded
    KF5::Archive
    KF5::I18n
    KF5::Solid
    KF5::Parts
    KF5::TextEditor
    KDev::Interfaces
    KDev::Util
    KDev::Language
    KDev::Project
)

target_link_libraries(kdevembedded-launcher
    KF5::Archive
    KF5::I18n
    KF5::Solid
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView
    KDev::Shell
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arduinoversion.h.in
            ${CMAKE_CURRENT_BINARY_DIR}/arduinoversion.h
)

install(DIRECTORY boardsimg DESTINATION ${DATA_INSTALL_DIR}/kdevembedded)
install(FILES kdevembedded.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kdevembedded)

add_subdirectory(doc_templates)

############################################
set(TEMPLATE_DIRS embeddedproject arduinoproject)
kdevplatform_add_app_templates(${TEMPLATE_DIRS})
