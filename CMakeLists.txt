cmake_minimum_required(VERSION 3.19)
project(passwordmanager LANGUAGES CXX)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(passwordmanager
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    src/crypto/cryptoutils.cpp
    src/crypto/cryptoutils.h
    src/utils/fileutils.cpp
    src/utils/fileutils.h
)

qt_add_translations(
    TARGETS passwordmanager
    TS_FILES passwordmanager_en_US.ts
)

target_link_libraries(passwordmanager
    PRIVATE
        Qt::Core
        Qt::Widgets
        sodium
)

include(GNUInstallDirs)

install(TARGETS passwordmanager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET passwordmanager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
