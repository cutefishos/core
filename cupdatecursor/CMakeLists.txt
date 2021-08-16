find_package(Qt5 COMPONENTS Core Gui X11Extras REQUIRED)
find_package(X11)

add_executable(cupdatecursor
  main.cpp
)
target_link_libraries(cupdatecursor
    Qt5::Core
    Qt5::Gui
    Qt5::X11Extras
    ${X11_LIBRARIES}
    X11::X11
    X11::Xi
    X11::Xcursor
)

install(TARGETS cupdatecursor DESTINATION ${CMAKE_INSTALL_BINDIR})
