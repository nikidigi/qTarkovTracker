cmake_minimum_required(VERSION 3.19)
project(qTarkovTracker VERSION 0.1.0 LANGUAGES CXX)


find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools SvgWidgets Network)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

qt_add_executable(qTarkovTracker
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    qTarkovTracker.qrc

    .gitignore
    README.md

    appsettings.h appsettings.cpp
    mapconfig.cpp
    mapconfig.h
    mapconfigloader.cpp
    mapconfigloader.h
    screenshotparser.h
    screenshotwatcher.h screenshotwatcher.cpp
    tarkovapi/baseresponse.h tarkovapi/baseresponse.cpp
    tarkovapi/cachemanager.h tarkovapi/cachemanager.cpp
    tarkovapi/client.h tarkovapi/client.cpp
    tarkovapi/itemsresponse.h tarkovapi/itemsresponse.cpp
    tarkovapi/mapitem.h tarkovapi/mapitem.cpp
    tarkovapi/mapsresponse.h tarkovapi/mapsresponse.cpp
    tarkovapi/tasksresponse.h tarkovapi/tasksresponse.cpp
    widgets/arrowmapmarker.h widgets/arrowmapmarker.cpp
    widgets/checklistpopup.h widgets/checklistpopup.cpp
    widgets/headerwidget.h widgets/headerwidget.cpp
    widgets/iconmapmarker.h widgets/iconmapmarker.cpp
    widgets/mapmarker.h widgets/mapmarker.cpp
    widgets/maptooltip.h widgets/maptooltip.cpp
    widgets/mapview.h widgets/mapview.cpp
)

target_compile_definitions(qTarkovTracker PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
)

target_include_directories(qTarkovTracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

qt_add_translations(
    TARGETS qTarkovTracker
    TS_FILES qTarkovTracker_en_US.ts
)

target_link_libraries(qTarkovTracker PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::SvgWidgets
    Qt6::Network
)

set(RESOURCE_DIRS
    maps
)

foreach(dir IN LISTS RESOURCE_DIRS)
    add_custom_command(TARGET qTarkovTracker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${dir}"
            "$<TARGET_FILE_DIR:qTarkovTracker>/${dir}"
        VERBATIM
    )
endforeach()

install(TARGETS qTarkovTracker
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET qTarkovTracker
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
