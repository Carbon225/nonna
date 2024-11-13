add_executable(motors
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/motors.c
)

target_link_libraries(motors PRIVATE
    pico_stdlib
    hardware_watchdog
    hardware_pwm
    hardware_gpio
    hardware_uart
)

pico_enable_stdio_usb(motors 1)
pico_enable_stdio_uart(motors 0)
pico_add_extra_outputs(motors)
