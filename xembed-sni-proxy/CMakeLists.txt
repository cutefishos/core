project(cutefish-xembedsniproxy)

add_definitions(-DQT_NO_CAST_TO_ASCII
-DQT_NO_CAST_FROM_ASCII
-DQT_NO_URL_CAST_FROM_STRING
-DQT_NO_CAST_FROM_BYTEARRAY)

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE OPTIONAL
                        PURPOSE "Required for building the X11 based workspace")

if(X11_FOUND)
  find_package(XCB MODULE REQUIRED COMPONENTS XCB RANDR)
  set_package_properties(XCB PROPERTIES TYPE REQUIRED)
  if(NOT X11_SM_FOUND)
    message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
  endif(NOT X11_SM_FOUND)

  find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS X11Extras)
endif()

if(X11_FOUND AND XCB_XCB_FOUND)
  set(HAVE_X11 1)
endif()

find_package(XCB
    REQUIRED COMPONENTS
        XCB
        XFIXES
        DAMAGE
        COMPOSITE
        RANDR
        SHM
        UTIL
        IMAGE
)

find_package(KF5WindowSystem)

set(XCB_LIBS
    XCB::XCB
    XCB::XFIXES
    XCB::DAMAGE
    XCB::COMPOSITE
    XCB::RANDR
    XCB::SHM
    XCB::UTIL
    XCB::IMAGE
)

set(XEMBED_SNI_PROXY_SOURCES
    main.cpp
    fdoselectionmanager.cpp
    snidbus.cpp
    sniproxy.cpp
    debug.cpp
    xtestsender.cpp
)

qt5_add_dbus_adaptor(DBUS_SOURCES org.kde.StatusNotifierItem.xml
                     sniproxy.h SNIProxy)
set(statusnotifierwatcher_xml org.kde.StatusNotifierWatcher.xml)
qt5_add_dbus_interface(DBUS_SOURCES ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)

set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(cutefish-xembedsniproxy ${XEMBED_SNI_PROXY_SOURCES} ${DBUS_SOURCES})
set_package_properties(XCB PROPERTIES TYPE REQUIRED)
target_link_libraries(cutefish-xembedsniproxy
    Qt5::Core
    Qt5::X11Extras
    Qt5::DBus
    KF5::WindowSystem
    ${XCB_LIBS}
    ${X11_XTest_LIB}
)

install(TARGETS cutefish-xembedsniproxy DESTINATION ${CMAKE_INSTALL_BINDIR})
