add_executable(hohoho
        hohoho.cc
        noise.cc
        )
        
target_compile_options(hohoho PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>)

target_link_libraries(hohoho pico_stdlib pico_dma_ws2812)

# enable usb output, disable uart output
pico_enable_stdio_usb(hohoho 1)
pico_enable_stdio_uart(hohoho 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(hohoho)

