#add_definitions(-DTRANSLATION_DOMAIN=\"kdevembedded\")
project(EMBEDDED C CXX)

set(ARDUINO_SDK_VERSION "168")
set(ARDUINO_SDK_VERSION_NAME "1.6.8")
set(ARDUINO_SDK_MIN_VERSION_NAME "1.6.8")

add_definitions(-DARDUINO_SDK_VERSION="${ARDUINO_SDK_VERSION}")
add_definitions(-DARDUINO_SDK_VERSION_NAME="${ARDUINO_SDK_VERSION_NAME}")
add_definitions(-DARDUINO_SDK_MIN_VERSION_NAME="${ARDUINO_SDK_MIN_VERSION_NAME}")

########### install target ###############

set(KDEVEMBEDDED_PART_SRCS
    embedded.cpp
    arduinowindow.cpp
    toolkit.cpp
    board.cpp
    firsttimewizard.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

set(KDEVEMBEDDED_PART_UI
    arduinowindow.ui
    firsttimewizard.ui
)

ki18n_wrap_ui(KDEVEMBEDDED_PART_SRCS ${KDEVEMBEDDED_PART_UI})

qt5_add_resources(KDEVEMBEDDED_PART_SRCS kdevembedded.qrc)
kdevplatform_add_plugin(kdevembedded JSON kdevembedded.json SOURCES ${KDEVEMBEDDED_PART_SRCS})

target_link_libraries(kdevembedded
    KF5::Parts
    KF5::TextEditor
    KDev::Interfaces
    KDev::Util
    KDev::Language
)

add_subdirectory(doc_templates)

############################################
set(TEMPLATE_DIRS embeddedproject)
kdevplatform_add_app_templates(${TEMPLATE_DIRS}) 
