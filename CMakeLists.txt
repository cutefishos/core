cmake_minimum_required(VERSION 3.5)
project(core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(QT Core Gui Widgets Quick QuickControls2 DBus Xml X11Extras LinguistTools)
find_package(Qt5 REQUIRED ${QT})
# find_package(FishUI REQUIRED)
find_package(PkgConfig REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

include(GNUInstallDirs)

add_subdirectory(polkit-agent)
add_subdirectory(screen-brightness)
add_subdirectory(session)
add_subdirectory(settings-daemon)
add_subdirectory(shutdown-ui)
add_subdirectory(xembed-sni-proxy)
add_subdirectory(powerman)
add_subdirectory(cpufreq)
add_subdirectory(chotkeys)
add_subdirectory(cupdatecursor)
add_subdirectory(gmenuproxy)
add_subdirectory(notificationd)
add_subdirectory(sddm-helper)
add_subdirectory(clipboard)

install(FILES cutefish DESTINATION /etc/ COMPONENT Runtime)
