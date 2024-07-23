//********************************************************************************
//  **  p i c o - h o - h o  **
//
// christmas led goodness
//

#include <cmath>
#include <cstdint>
#include <vector>
#include "pico/stdlib.h"
#include "../pico_dma_ws2812/pico_dma_ws2812.hpp"

#include "colour.h"
#include "noise.h"

#define XMASx


using std::begin, std::end;

constexpr int LedPin = 25;
constexpr int NeopixelAPin = 28;
constexpr int NeopixelBPin = 2;
constexpr int NumLeds = 50;


bool gLedState = false;

auto gNeoPixelsA = WS2812(NumLeds, pio0, 0, 0, NeopixelAPin);
auto gNeoPixelsB = WS2812(NumLeds, pio1, 1, 1, NeopixelBPin);


inline void set_pixel(WS2812& leds, int pixelIx, const FRgb& rgb)
{
    leds.set_rgb(pixelIx, uint8_t(rgb.r * 255.f), uint8_t(rgb.g * 255.f), uint8_t(rgb.b * 255.f));
}


#ifdef XMAS

void updateStringA(float now)
{
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
        gNeoPixelsA.set_rgb(i, col.r, col.g, col.b);
    }
}

void updateStringB(float now)
{
    FRgb baseCol(col::CalmWhite);
    for (int i=0; i<NumLeds; ++i)
    {
        float t = cosf(float(i) * (6.28f / NumLeds) + now * 0.8f);
        t = 0.4f * std::max(0.f, t);
        t += 0.6f * perlinNoise1D(float(i) * 3.f + now * 10.f);

        FRgb col = baseCol * t;
        set_pixel(gNeoPixelsB, i, col);
    }
}

void updateLights(float now)
{
    updateStringA(now);
    updateStringB(now);
}

#else

inline constexpr float easeInOut3(float x)
{
    if (x < 0.5f)
        return 4.f * x * x * x;
    
    x = (2.f * x) - 2.f;
	return (0.5f * x * x * x) + 1.f;
}

inline FHsv calc_light_col_hsv(int i, float now)
{
    // return { 0.5f * (1.f + cosf(float(i) * 0.3f + now * 1.5f)),
    //          0.3f + 0.4f * perlinNoise1D(float(i) * 3.f + now * 10.f),
    //          0.33f };

    if (i & 2)
    {
        return { 0.055f,
                 1.f,
                 0.1f + 0.3f * perlinNoise1D(float(i) * 3.f + now * 7.f) };
    }
    return { 0.3f,
             1.f,
             0.05f + 0.3f * perlinNoise1D(float(i) * 3.f + now * 6.f) };
}

void updateLights(float now)
{
    sleep_us(100);
    for (int i=0; i<NumLeds; ++i)
    {
        FHsv col = calc_light_col_hsv(i + 0, now);
        gNeoPixelsA.set_hsv_scaled(i, col.h, col.s, col.v, 1.f, 0.6f, 0.2f);
    }
    sleep_us(100);
    for (int i=0; i<NumLeds; ++i)
    {
        FHsv col = calc_light_col_hsv(99 - i, now);
        gNeoPixelsB.set_hsv_scaled(i, col.h, col.s, col.v, 1.f, 0.6f, 0.2f);
    }
}

#endif


void loop()
{
    uint64_t usSinceBoot = to_us_since_boot(get_absolute_time());
    // choose to wrap rather than have precision degrade
    usSinceBoot &= 0xfffffffffull;
    float now = (float(usSinceBoot) * (1.f / (1000.f * 1000.f)));

    updateLights(now);

    gNeoPixelsA.update();
    gNeoPixelsB.update();
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
