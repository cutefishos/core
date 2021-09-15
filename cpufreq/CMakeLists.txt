project(cutefish-cpufreq)
set(TARGET cutefish-cpufreq)

set(SOURCES
    main.cpp
)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Quick
        Qt5::DBus
        Qt5::X11Extras
)

configure_file(
    com.cutefish.cpufreq.pkexec.policy.in
    com.cutefish.cpufreq.pkexec.policy
    @ONLY
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/com.cutefish.cpufreq.pkexec.policy DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/polkit-1/actions/)
