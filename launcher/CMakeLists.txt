add_definitions(-DTRANSLATION_DOMAIN=\"kdevexecute2\")

########### next target ###############
set(kdevexecute_PART_UIS
    launcher/nativeappconfig.ui
)

set(kdevexecute_PART_SRCS
    launcher/projecttargetscombobox.cpp
    launcher/executeplugin.cpp
    launcher/nativeappconfig.cpp
    launcher/nativeappjob.cpp
)

ki18n_wrap_ui( kdevexecute_PART_SRCS ${kdevexecute_PART_UIS} )

kdevplatform_add_plugin(kdevexecute2 JSON launcher/kdevexecute2.json SOURCES ${kdevexecute_PART_SRCS})

target_link_libraries(kdevexecute2
    #KF5::KCMUtils
    KDev::Interfaces
    KDev::Util
    KDev::Project
    KDev::OutputView
    KDev::Shell
)

########### install files ###############

install(FILES launcher/iexecuteplugin2.h DESTINATION ${DATA_INSTALL_DIR}/kdevembedded/launcher COMPONENT Devel)
