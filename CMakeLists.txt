cmake_minimum_required(VERSION 3.5)
project(core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(QT Core Gui Widgets Quick QuickControls2 DBus Xml LinguistTools)
find_package(Qt6 REQUIRED ${QT})
find_package(KF6Screen REQUIRED)
# find_package(FishUI REQUIRED)
find_package(PkgConfig REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

include(GNUInstallDirs)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(polkit-agent)
add_subdirectory(screen-brightness)
add_subdirectory(session)
add_subdirectory(settings-daemon)
add_subdirectory(shutdown-ui)
add_subdirectory(powerman)
add_subdirectory(cpufreq)
add_subdirectory(sddm-helper)
add_subdirectory(clipboard)

install(FILES cutefish DESTINATION /etc/ COMPONENT Runtime)
