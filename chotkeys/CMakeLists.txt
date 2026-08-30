find_package(Qt6 COMPONENTS Core Widgets DBus Gui REQUIRED)
find_package(XCB MODULE REQUIRED COMPONENTS XCB KEYSYMS)
find_package(X11)

set(PROJECT_SOURCES
    main.cpp
    application.cpp
    hotkeys.cpp
)

add_executable(chotkeys
    ${PROJECT_SOURCES}
)

target_link_libraries(chotkeys
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    Qt6::Gui
    ${XCB_LIBS}
    ${X11_LIBRARIES}
    XCB::KEYSYMS
)

install(TARGETS chotkeys RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
