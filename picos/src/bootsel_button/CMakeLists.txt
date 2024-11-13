add_library(bootsel_button STATIC
    ${CMAKE_CURRENT_LIST_DIR}/bootsel_button.c
)

target_include_directories(bootsel_button PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(bootsel_button PRIVATE
    pico_stdlib
    hardware_sync
    hardware_gpio
)
