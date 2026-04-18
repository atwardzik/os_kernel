add_library(drivers OBJECT
        atomic.S
        resets.S
        time.S
        gpio_functions.S
        pio.S
        uart.S

        divider.S

        ps2_keyboard.S
        vga.c
        vga.S
        spi.S
        ethernet.c
        sd_card.c
        ps2.c
        ps2_keyboard.c
)

target_include_directories(drivers PUBLIC ${CMAKE_SOURCE_DIR}/include/drivers)
