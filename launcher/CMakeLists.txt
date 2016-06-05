add_definitions(-DTRANSLATION_DOMAIN=\"kdevembedded-launcher\")

########### next target ###############
set(kdevembedded-launcher_PART_UIS
    launcher/embeddedlauncher.ui
)

set(kdevembedded-launcher_PART_SRCS
    launcher/projecttargetscombobox.cpp
    launcher/executeplugin.cpp
    launcher/embeddedlauncher.cpp
    launcher/launcherjob.cpp
)

ki18n_wrap_ui( kdevembedded-launcher_PART_SRCS ${kdevembedded-launcher_PART_UIS} )

kdevplatform_add_plugin(kdevembedded-launcher JSON launcher/kdevembedded-launcher.json SOURCES ${kdevembedded-launcher_PART_SRCS})

target_link_libraries(kdevembedded-launcher
    KF5::Solid
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView
    KDev::Shell
)

########### install files ###############

install(FILES launcher/iexecuteplugin.h DESTINATION ${DATA_INSTALL_DIR}/kdevembedded/launcher COMPONENT Devel)
