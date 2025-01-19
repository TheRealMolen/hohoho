//********************************************************************************
//  **  p i c o - h o - h o  **
//
// christmas led goodness
//

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "../pico_dma_ws2812/pico_dma_ws2812.hpp"

#include "colour.h"
#include "noise.h"

#define V_BOARD  2
#define XMASx
#define WINTER

constexpr bool board2 = (V_BOARD == 2);
constexpr bool board3 = (V_BOARD == 3);


using std::begin, std::end;

constexpr int LedPin = 25;
constexpr int NeopixelAPin = board3 ? 15 : 28;
constexpr int NumLeds = board2 ? 150 : 50;


bool gLedState = false;

auto gNeoPixelsA = WS2812(NumLeds, pio0, 0, 0, NeopixelAPin);


#if V_BOARD == 1
constexpr int NeopixelBPin = 2;
auto gNeoPixelsB = WS2812(NumLeds, pio1, 1, 1, NeopixelBPin);
#endif


inline constexpr float easeInOut3(float x)
{
    if (x < 0.5f)
        return 4.f * x * x * x;
    
    x = (2.f * x) - 2.f;
	return (0.5f * x * x * x) + 1.f;
}

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

#elif (V_BOARD == 1)

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

#elif defined(WINTER)


std::default_random_engine g_randGenerator;
std::uniform_real_distribution<float> g_randDistribution(0.f, 1.f);

inline float rand01()
{
    return g_randDistribution(g_randGenerator);
}
inline float randRange(float lo, float hi)
{
    return lo + (hi-lo) * rand01();
}
inline const auto& pick(const auto& container)
{
    std::uniform_int_distribution<int> dist(0, int(container.size() - 1));
    int ix = dist(g_randGenerator);
    return container[ix];
}


#define HSL_INT(h,s,l) FHsv{ float(h) / 360.f, float(s) / 100.f, float(l) / 100.f }

const std::array<FHsv, 5> kBgPalette =
{
    HSL_INT(96, 2, 5),
    HSL_INT(339, 40, 56),
    HSL_INT(61, 40, 54),
    HSL_INT(172, 30, 59),
    HSL_INT(0, 10, 90),
};
FHsv palette_lookup(float t, const auto& palette)
{
    t -= floorf(t);
    if (t < 0.f)
        t += 1.f;

    const int paletteLen = palette.size();
    const float scaledT = t * float(paletteLen);
    const int ix0 = int(scaledT);
    const float frac = scaledT - float(ix0);
    const int ix1 = (ix0 == (paletteLen - 1)) ? 0 : ix0 + 1;

    const FHsv hsv0 = palette[ix0];
    const FHsv hsv1 = palette[ix1];

    return lerp(hsv0, hsv1, frac);
}

FHsv g_backBuffer[NumLeds];

enum class EParticlePhase : uint8_t { SLEEPING, WAKING, COOLING, };
struct FParticle
{
    float Pos;
    float Vel;
    float MaxHeat;
    float CurrHeat;
    FHsv Col;
    EParticlePhase Phase;
};
const std::array<FHsv, 3> kParticlePalette =
{
    HSL_INT(339, 80, 56),
    HSL_INT(61, 80, 54),
    HSL_INT(172, 80, 59),
};

void init(FParticle& p)
{
    p.Pos = -1.f;
    p.Vel = 0.f;
    p.MaxHeat = -1.f;
    p.CurrHeat = -1.f;
    p.Col = { 0.1f, 0.1f, 0.5f };
    p.Phase = EParticlePhase::SLEEPING;
}

constexpr int MaxParticles = 4;
FParticle g_particles[MaxParticles];

float g_timeToNextParticle = 1.f;

void init_particles()
{
    // we have hardware random bits, let's use some!
    g_randGenerator.seed(get_rand_32());

    for (FParticle& p : g_particles)
        init(p);

    g_timeToNextParticle = randRange(0.5f, 2.f);
}

FParticle& get_particle_to_recycle()
{
    for (FParticle& p : g_particles)
    {
        if (p.Phase == EParticlePhase::SLEEPING)
            return p;
    }

    // find the oldest particle to reuse
    FParticle* oldest = g_particles;
    for (int i=1; i<MaxParticles; ++i)
    {
        if (g_particles[i].CurrHeat < oldest->CurrHeat)
            oldest = &g_particles[i];
    }

    return *oldest;
}
void spawn_particle()
{
    FParticle& p = get_particle_to_recycle();

    p.Phase = EParticlePhase::WAKING;

    p.Pos = randRange(float(NumLeds) * 0.2f, float(NumLeds) * 0.8f);
    p.Vel = randRange(2.f, 5.f);
    if (rand01() < 0.5f)
        p.Vel = -p.Vel;

    p.MaxHeat = randRange(0.5f, 3.f);
    p.CurrHeat = 0.05f;

    p.Col = pick(kParticlePalette);
}

void update_particles(float deltaTime)
{
    // tick all existing particles
    for (FParticle& p : g_particles)
    {
        switch (p.Phase)
        {
            case EParticlePhase::WAKING:
                p.CurrHeat += deltaTime * 4.f;
                if (p.CurrHeat >= p.MaxHeat)
                    p.Phase = EParticlePhase::COOLING;
                break;

            case EParticlePhase::COOLING:
                p.CurrHeat -= deltaTime;
                if (p.CurrHeat <= 0.f)
                    p.Phase = EParticlePhase::SLEEPING;
                break;
        }
        if (p.Phase == EParticlePhase::SLEEPING)
            continue;

        p.Pos += p.Vel * deltaTime;
    }

    // spawn a new particle if it's time
    g_timeToNextParticle -= deltaTime;
    if (g_timeToNextParticle <= 0.f)
    {
        spawn_particle();
        g_timeToNextParticle = randRange(1.f, 2.f);
    }
}

void render_particles()
{
    for (FParticle& p : g_particles)
    {
        if (p.Phase == EParticlePhase::SLEEPING)
            continue;

        const bool movingUp = p.Vel > 0.f;
        const int dir = movingUp ? 1 : -1; 
        const int headLed = int(p.Pos + dir);

        const int length = int(p.CurrHeat * 10.f);
        const int tailLed = headLed + (dir * length);

        float amount = p.CurrHeat; 
        for (int i = headLed; i != tailLed; i += dir)
        {
            if (i < 0 || i >= NumLeds)
                continue;
            if (amount < 0.1f)
                break;
            
            g_backBuffer[i] = lerp(g_backBuffer[i], p.Col,  amount);
            amount *= 0.9f;
        }
    }
}



inline float ramp(float t)
{
    return fmodf(t, 1.f);
}

float g_lastNow = 0.f;
void updateLights(float now)
{
    float deltaTime = now - g_lastNow;
    g_lastNow = now;

    // base shimmer
    for (int i=0; i<NumLeds; ++i)
    {
        float t = perlinNoise1D(float(i) * 0.4f + now * 0.3f);
        FHsv col = palette_lookup(t, kBgPalette);

        float shimmer = 0.3f * perlinNoise1D(float(i) * 3.f + now * 5.f);
        col.v = shimmer;

        g_backBuffer[i] = col;
    }

    // particles
    update_particles(deltaTime);
    render_particles();

    for (int i=0; i<NumLeds; ++i)
    {
        FHsv col = g_backBuffer[i];
        Rgb rgb = col.toRgbScaled(1.f, 0.6f, 0.3f);
        gNeoPixelsA.set_rgb(i, rgb.r, rgb.g, rgb.b);
    }

    sleep_us(500);
}

#else // board #2


FHsv gentle_shimmer(int i, float now)
{
    return {
        0.1f,
        0.3f,
        0.3f * perlinNoise1D(float(i) * 3.f + now * 5.f) };
}

FHsv classic_xmas(int i, float now)
{
    float v = 0.05f + 0.2f * perlinNoise1D(float(i) * 3.f + now * 4.f);
    v = std::max(0.f, v);
    switch(i & 3)
    {
        case 0: return { 0.f, 0.8f, v };      // red
        case 1: return { 0.6f, 1.f, v };     // blue
        case 2: return { 0.1f, 0.8f, v };    // amber
        default: return { 0.35f, 0.8f, v };   // green
    }
}


void updateLights(float now)
{
    sleep_us(500);
    for (int i=0; i<NumLeds; ++i)
    {
        FHsv scene1 = classic_xmas(i, now);
        FHsv scene2 = gentle_shimmer(i, now);
        float scene = easeInOut3(perlinNoise1D(float(i) * 0.1f + now * 1.f));
        FHsv col = lerp(scene1, scene2, scene);

        gNeoPixelsA.set_hsv_scaled(i, col.h, col.s, col.v, 1.f, 0.6f, 0.2f);
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

#if V_BOARD == 1
    gNeoPixelsB.update();
#endif
}

int main() {
    gpio_init(LedPin);
    gpio_set_dir(LedPin, GPIO_OUT);
    gpio_put(LedPin, 1);

    stdio_usb_init();

    init_particles();

    for(;;)
    {
        loop();
    }

    return 0;
}
