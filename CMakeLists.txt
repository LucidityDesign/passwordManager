cmake_minimum_required(VERSION 3.19)
project(passwordmanager LANGUAGES CXX)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where Qt is (adjust path if needed)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

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





    src/ui/stackedwidget.h src/ui/stackedwidget.cpp src/ui/stackedwidget.ui
    src/ui/newlogindialog.h src/ui/newlogindialog.cpp src/ui/newlogindialog.ui
    src/vault/vaultmanager.h src/vault/vaultmanager.cpp
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
target_link_libraries(passwordmanager PRIVATE Qt6::Widgets)

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
