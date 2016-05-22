#add_definitions(-DTRANSLATION_DOMAIN=\"kdevembedded\")
project(embedded)

set(ARDUINO_SDK_VERSION "169")
set(ARDUINO_SDK_VERSION_NAME "1.6.9")
set(ARDUINO_SDK_MIN_VERSION_NAME "1.6.8")

set(CMAKE_CXX_STANDARD 11)

cmake_minimum_required(VERSION 3.5)
########### install target ###############

find_package(ECM REQUIRED)

set(CMAKE_MODULE_PATH ${KDevPlatform_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

set(KDEVEMBEDDED_PART_SRCS
    embedded.cpp
    arduinowindow.cpp
    toolkit.cpp
    board.cpp
    firsttimewizard.cpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(KDEVEMBEDDED_PART_UI
    arduinowindow.ui
    firsttimewizard.ui
)

find_package(KDevPlatform REQUIRED)
find_package(KDevelop-PG-Qt REQUIRED)
include_directories(${KDEVPGQT_INCLUDE_DIR})

find_package(KF5 REQUIRED Archive I18n NewStuff ItemModels TextEditor ThreadWeaver)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

ki18n_wrap_ui(KDEVEMBEDDED_PART_SRCS ${KDEVEMBEDDED_PART_UI})

qt5_add_resources(KDEVEMBEDDED_PART_SRCS kdevembedded.qrc)
kdevplatform_add_plugin(kdevembedded JSON kdevembedded.json SOURCES ${KDEVEMBEDDED_PART_SRCS})

target_link_libraries(kdevembedded
    KF5::Archive
    KF5::Solid
    KF5::Parts
    KF5::TextEditor
    KDev::Interfaces
    KDev::Util
    KDev::Language
)

message(${DATA_INSTALL_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/firsttimewizard.h.in ${CMAKE_CURRENT_SOURCE_DIR}/firsttimewizard.h)
install(DIRECTORY boardsimg DESTINATION ${DATA_INSTALL_DIR}/kdevembedded)

add_subdirectory(doc_templates)

############################################
set(TEMPLATE_DIRS embeddedproject)
kdevplatform_add_app_templates(${TEMPLATE_DIRS})
