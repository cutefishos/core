find_package(Qt5 COMPONENTS Core DBus REQUIRED)
find_package(KF5GlobalAccel REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    application.cpp
)

add_executable(chotkeys
    ${PROJECT_SOURCES}
)

target_link_libraries(chotkeys
    PRIVATE
    Qt5::Core
    Qt5::DBus
    KF5::GlobalAccel
)

install(TARGETS chotkeys RUNTIME DESTINATION /usr/bin)
