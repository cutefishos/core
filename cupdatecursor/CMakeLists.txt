find_package(Qt6 COMPONENTS Core Gui REQUIRED)
find_package(X11)

add_executable(cupdatecursor
  main.cpp
)
target_link_libraries(cupdatecursor
    Qt6::Core
    Qt6::Gui
    ${X11_LIBRARIES}
    X11::X11
    X11::Xi
    X11::Xcursor
)

install(TARGETS cupdatecursor DESTINATION ${CMAKE_INSTALL_BINDIR})
