add_executable(sensors
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/sensors.c
)

target_link_libraries(sensors PRIVATE
    pico_stdlib

    hardware_gpio
    hardware_watchdog
    hardware_uart

    nonna_proto
    nanopb_encode
    proto_framing
    bootsel_button
)

pico_enable_stdio_usb(sensors 1)
pico_enable_stdio_uart(sensors 0)
pico_add_extra_outputs(sensors)
