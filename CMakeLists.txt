#add_definitions(-DTRANSLATION_DOMAIN=\"kdevpatrick\")
project(PATRICK C CXX)

########### install target ###############

set(kdevpatrick_PART_SRCS
    patrickplugin.cpp
    arduinowindow.cpp
)

include_directories(CMAKE_CURRENT_BINARY_DIR)

set(kdevpatrick_PART_UI
    arduinowindow.ui
)

ki18n_wrap_ui(kdevpatrick_PART_SRCS ${kdevpatrick_PART_UI})

qt5_add_resources(kdevpatrick_PART_SRCS kdevpatrick.qrc)
kdevplatform_add_plugin(kdevpatrick JSON kdevpatrick.json SOURCES ${kdevpatrick_PART_SRCS})

target_link_libraries(kdevpatrick
    KF5::Parts
    KF5::TextEditor
    KDev::Interfaces
    KDev::Util
    KDev::Language
)

add_subdirectory(doc_templates)
