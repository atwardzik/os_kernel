add_library(drivers STATIC
        atomic.S
        resets.S
        time.S
        gpio_functions.S
        pio.S
        uart.S

        divider.S

        keyboard.S
        vga.S
)

target_include_directories(drivers PUBLIC ${CMAKE_SOURCE_DIR}/include/drivers)
