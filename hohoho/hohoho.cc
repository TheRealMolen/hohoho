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


struct Rgb { uint8_t r, g, b; };
namespace col
{
    const Rgb CalmWhite(130, 70, 20);
    const Rgb NiceRed(180, 0, 0);
    const Rgb NiceGreen(5, 85, 0);
    const Rgb NiceBlue(5, 0, 40);
    const Rgb NiceAmber(130, 35, 0);
    const Rgb NicePink(140, 20, 40);

    const Rgb Dark(0, 0, 2);
};


using std::begin, std::end;

constexpr int LedPin = 25;
constexpr int NeopixelAPin = 28;
constexpr int NeopixelBPin = 2;
constexpr int NumLeds = 50;



#define kPerlinOctaves      3
static const float kaPerlinOctaveAmplitude[kPerlinOctaves] =
{
  0.75f, 0.4f, 0.1f,
};

float perlinNoise1( long x )
{
  x = ( x<<13 ) ^ x;
  return ( 1.0f - ( (x * (x * x * 15731 + 789221L) + 1376312589L) & 0x7fffffff) * (1.0f / 1073741824.0f) );
}

float perlinSmoothedNoise1( long x )
{
  return perlinNoise1(x)*0.5f
       + perlinNoise1(x-1)*0.25f
       + perlinNoise1(x+1)*0.25f;
}

float perlinLerpedNoise1( float x )
{
  long  xInteger  = long(x);
  float xFraction = x - xInteger;

  float v1 = perlinSmoothedNoise1( xInteger );
  float v2 = perlinSmoothedNoise1( xInteger + 1 );

  return std::lerp( v1, v2, xFraction );
}

float perlinNoise1D( float x )
{
  float total = 0.0f;

  for( int octave = 0; octave < kPerlinOctaves; octave++ )
  {
    int   frequency = 1 << octave;
    float amplitude = kaPerlinOctaveAmplitude[octave];

    total += perlinLerpedNoise1( x * frequency ) * amplitude;
  }

  // map from [-1,1] to [0,1] and constrain
  return std::clamp( (total + 1.0f) * 0.5f, 0.0f, 1.0f );
}


struct FRgb
{
    float r, g, b;

    FRgb(float r, float g, float b) : r(r), g(g), b(b) { /**/ }
    FRgb(const Rgb& rgb)
    {
        r = float(rgb.r) * (1.f / 255.f);
        g = float(rgb.g) * (1.f / 255.f);
        b = float(rgb.b) * (1.f / 255.f);
    }

    FRgb operator*(float s)
    {
        s = std::clamp(s, 0.f, 1.f);
        return {r * s, g * s, b * s};
    }
};


bool gLedState = false;

auto gNeoPixelsA = WS2812(NumLeds, pio0, 0, 0, NeopixelAPin);
auto gNeoPixelsB = WS2812(NumLeds, pio1, 1, 1, NeopixelBPin);


inline void set_pixel(WS2812& leds, int pixelIx, const FRgb& rgb)
{
    leds.set_rgb(pixelIx, uint8_t(rgb.r * 255.f), uint8_t(rgb.g * 255.f), uint8_t(rgb.b * 255.f));
}


void updateStringA(float now)
{
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


void loop()
{
    uint64_t usSinceBoot = to_us_since_boot(get_absolute_time());
    // choose to wrap rather than have precision degrade
    usSinceBoot &= 0xfffffffffull;
    float now = (float(usSinceBoot) * (1.f / (1000.f * 1000.f)));

    updateStringA(now);
    updateStringB(now);

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
