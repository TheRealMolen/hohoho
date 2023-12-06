//********************************************************************************
//  **  p i c o - h o - h o  **
//
// christmas led goodness
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>
#include "pico/stdlib.h"
#include "../pico_dma_ws2812/pico_dma_ws2812.hpp"


struct Rgb { uint8_t r, g, b; };

namespace col
{
    const Rgb CalmWhite(120, 70, 20);
    const Rgb NiceRed(140, 0, 0);
    const Rgb NiceGreen(5, 100, 0);
    const Rgb NiceBlue(5, 0, 40);
    const Rgb NiceAmber(130, 35, 0);
    const Rgb NicePink(140, 20, 40);
};


using std::begin, std::end;

constexpr int LedPin = 25;
constexpr int NeopixelPin = 28;
constexpr int NumLeds = 50;

bool gLedState = false;

auto gNeoPixels = WS2812(NumLeds, pio0, 0 /* sm? */, NeopixelPin);

void loop()
{
    uint64_t usSinceBoot = to_us_since_boot(get_absolute_time());
    // choose to wrap rather than have precision degrade
    usSinceBoot &= 0xfffffffffull;
    float now = (float(usSinceBoot) * (1.f / (1000.f * 1000.f)));

    int currLed = int(NumLeds * (0.5f + 0.5f * cosf(now * 2.f)));

    for (int i=0; i<NumLeds; ++i)
    {
        Rgb col = col::NiceRed;
        if ((i & 3) == 2)
            col = col::NiceGreen;
        if (i & 1)
        {
            float t = cosf(float(i) * 0.3f + now * 4.f);
            col = (t > 0.4f) ? col::NiceBlue : col::NiceAmber;
        }
        gNeoPixels.set_rgb(i, col.r, col.g, col.b);
    }

    gNeoPixels.update(true);
}

int main() {
    gpio_init(LedPin);
    gpio_set_dir(LedPin, GPIO_OUT);
    gpio_put(LedPin, 1);

    stdio_usb_init();

    for(;;)
    {
        loop();
    }

    return 0;
}
