add_executable(sensors
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/sensors.c
    ${CMAKE_CURRENT_LIST_DIR}/control_loop.c
    ${CMAKE_CURRENT_LIST_DIR}/turn_pid.c
    ${CMAKE_CURRENT_LIST_DIR}/brain.c
    ${CMAKE_CURRENT_LIST_DIR}/neural_network.c
)

target_link_libraries(sensors PRIVATE
    pico_stdlib
    pico_multicore
    pico_rand

    hardware_gpio
    hardware_watchdog
    hardware_uart
    hardware_sync

    nonna_proto
    nanopb_encode
    nanopb_decode
    proto_framing
)

pico_enable_stdio_usb(sensors 1)
pico_enable_stdio_uart(sensors 0)
pico_add_extra_outputs(sensors)
