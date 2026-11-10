//********************************************************************************
//  **  p i c o - h o - h o  **
//
// christmas led goodness
//

// board 1 is og T shaped board with 2 independent 50 led strings coming off it
// board 2 is one string of 150 leds
// board 3 is one string of 50 leds
// board 4 is one 5x8 panel of 40 leds
#define V_BOARD  1
#define XMAS
#define WINTERx

constexpr bool board1 = (V_BOARD == 1);
constexpr bool board2 = (V_BOARD == 2);
constexpr bool board3 = (V_BOARD == 3);
constexpr bool board4 = (V_BOARD == 4);

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

#include "xmas_pal.def.h"



#define HSL_INT(h,s,l) FHsv{ float(h) / 360.f, float(s) / 100.f, float(l) / 100.f }


using std::begin, std::end;

constexpr int LedPin = 25;
constexpr int NeopixelAPin = board3 ? 15 : 28;
constexpr int NumLeds = board2 ? 150 : (board4 ? 40 : 50);
constexpr int TotalLeds = board1 ? (2*NumLeds) : NumLeds;

constexpr float RScale = board4 ? 0.3f : 1.f;
constexpr float GScale = board4 ? 0.3f : 0.6f;
constexpr float BScale = board4 ? 0.3f : 0.2f;

bool gLedState = false;

auto gNeoPixelsA = WS2812(NumLeds, pio0, 0, 0, NeopixelAPin);


#if V_BOARD == 1
constexpr int NeopixelBPin = 2;
auto gNeoPixelsB = WS2812(NumLeds, pio1, 1, 1, NeopixelBPin);
#endif


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

template<typename T>
inline T sign(T x)
{
    return (x >= T(0)) ? T(1) : T(-1);
}

inline float ramp(float t)
{
    return fmodf(t, 1.f);
}


//-------------------------------------------------------------------------------
// X M A S
//
namespace xmas
{

void updateString_og(float now, auto& string)
{
    FHsv blue(col::NiceBlue);
    FHsv amber(col::NiceAmber);

    for (int i=0; i<NumLeds; ++i)
    {
        FRgb col = col::NiceRed;
        if ((i & 3) == 2)
            col = col::NiceGreen;
        if (i & 1)
        {
            float t = cosf(float(i) * 0.3f + now * 1.f);
            t *= 1.3f;                       // scale up
            t = std::clamp(t, -1.f, 1.f);   // clamp to get clipped flat tops
            const float sgn = sign(t);
            t = sgn * powf(t * sgn, 0.1f);  // smooth transitions
            t = 0.5f * (1.f + t);
            t = std::clamp(t, 0.f, 1.f);

            col = lerp(blue, amber, t).toRgb();
        }

        FRgb out(col);

        {
            float t = cosf(float(i) * (6.28f / NumLeds) + now * 0.4f);
            t = 0.6f * std::max(0.f, t);
            t += 0.4f * perlinNoise1D(float(i) * 3.f + now * 4.f);
            out = out * t;
        }

        set_pixel(string, i, out);
    }
}


void updateString(float now, auto& string, int led_offs = 0)
{
    FHsv pal[] = { col::NiceRed, col::NiceGreen, col::NiceBlue, col::NiceAmber };

    for (int i=0; i<NumLeds; ++i)
    {
        const int led = i + led_offs;

        FRgb col = pal[kXmasPal[led]].toRgb();
        FRgb out(col);

        {
            float t = cosf(float(led) * (6.28f / NumLeds) + now * 0.4f);
            t = 0.6f * std::max(0.f, t);
            t += 0.4f * perlinNoise1D(float(led) * 3.f + now * 4.f);
            out = out * t;
        }

        set_pixel(string, i, out);
    }
}


void updateLights(float now)
{
    updateString(now, gNeoPixelsA);

#if V_BOARD == 1
    updateString(now, gNeoPixelsB, NumLeds);
#endif
}

} // namespace xmas

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// C L A S S I C
//

namespace classic
{

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
        gNeoPixelsA.set_hsv_scaled(i, col.h, col.s, col.v, RScale, GScale, BScale);
    }
    sleep_us(100);

#if V_BOARD == 1
    for (int i=0; i<NumLeds; ++i)
    {
        FHsv col = calc_light_col_hsv(99 - i, now);
        gNeoPixelsB.set_hsv_scaled(i, col.h, col.s, col.v, RScale, GScale, BScale);
    }
#endif
}

} // namespace classic


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// W I N T E R
//

namespace winter
{
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
        Rgb rgb = col.toRgbScaled(RScale, GScale, BScale);
        gNeoPixelsA.set_rgb(i, rgb.r, rgb.g, rgb.b);
    }
}

} // namespace winter


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// S H I M M E R
//

namespace shimmer
{

FHsv gentle_shimmer(int i, float now)
{
    return {
        0.1f,
        0.4f,
        0.05f + 0.25f * perlinNoise1D(float(i) * 3.f + now * 3.f)
    };
}


enum class EParticlePhase : uint8_t { SLEEPING, WAKING, COOLING, };
struct FParticle
{
    int Pos;
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
    p.Pos = -1;
    p.CurrHeat = -1.f;
    p.Col = { 0.1f, 0.1f, 0.5f };
    p.Phase = EParticlePhase::SLEEPING;
}

constexpr int MaxParticles = (TotalLeds / 10) + 1;
FParticle g_particles[MaxParticles];

float g_timeToNextParticle = 0.f;

void init_particles()
{
    for (FParticle& p : g_particles)
        init(p);

    g_timeToNextParticle = 0.02f;
}

FParticle* get_particle_to_recycle()
{
    for (FParticle& p : g_particles)
    {
        if (p.Phase == EParticlePhase::SLEEPING)
            return &p;
    }

    return nullptr;
}
bool try_spawn_particle()
{
    FParticle* p = get_particle_to_recycle();
    if (!p)
        return false; 

    p->Phase = EParticlePhase::WAKING;

    p->Pos = int(randRange(float(TotalLeds) * 0.01f, float(TotalLeds) * 0.99f));

    p->CurrHeat = 0.05f;

    p->Col = pick(kParticlePalette);
    p->Col.h += randRange(-0.1f, 0.1f);
    p->Col.h = std::clamp(p->Col.h, 0.f, 1.f);

    return true;
}

void update_particles(float deltaTime)
{
    // tick all existing particles
    for (FParticle& p : g_particles)
    {
        switch (p.Phase)
        {
            case EParticlePhase::WAKING:
                p.CurrHeat += deltaTime * 0.5f;
                if (p.CurrHeat >= 1.f)
                {
                    p.CurrHeat = 1.f;
                    p.Phase = EParticlePhase::COOLING;
                }
                break;

            case EParticlePhase::COOLING:
                p.CurrHeat -= deltaTime * 0.2f;
                if (p.CurrHeat <= 0.f)
                    p.Phase = EParticlePhase::SLEEPING;
                break;
        }
    }

    // spawn a new particle if it's time and we have particles free
    g_timeToNextParticle -= deltaTime;
    if (g_timeToNextParticle <= 0.f)
    {
        if (try_spawn_particle())
        {
            g_timeToNextParticle = randRange(0.1f, 0.25f);
            gpio_put(LedPin, 0);
        }
        else
        {
            gpio_put(LedPin, 1);
        }
    }
}

void updateString(float now, auto& ledString, int offsetLedIx, bool flippedIndices)
{
    constexpr int MaxI = NumLeds - 1;
    for (int i=0; i<NumLeds; ++i)
    {
        const int ledIx = offsetLedIx + (flippedIndices ? MaxI - i : i); 

        FHsv shimmer_base = gentle_shimmer(ledIx, now);

        FHsv col = shimmer_base;

        for (const FParticle& p : g_particles)
        {
            if (p.Pos != ledIx || p.Phase == EParticlePhase::SLEEPING)
                continue;

            FHsv pcol = p.Col;
            pcol.v *= 0.9f * perlinNoise1D(float(ledIx) * 3.f + now * 3.f);

            col = lerp(col, pcol, p.CurrHeat);
        }

        ledString.set_hsv_scaled(i, col.h, col.s, col.v, RScale, GScale, BScale);
    }
}

float g_lastNow = 0.f;

void updateLights(float now)
{
    float deltaTime = now - g_lastNow;
    g_lastNow = now;

    sleep_us(1000);

    update_particles(deltaTime);

    updateString(now, gNeoPixelsA, 0, false);

#if V_BOARD == 1
    updateString(now, gNeoPixelsB, NumLeds, true);
#endif
}

} // namespace shimmer



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// C O R E
//

void loop()
{
    uint64_t usSinceBoot = to_us_since_boot(get_absolute_time());
    // choose to wrap rather than have precision degrade
    usSinceBoot &= 0xfffffffffull;
    float now = (float(usSinceBoot) * (1.f / (1000.f * 1000.f)));

#ifdef XMAS
    xmas::updateLights(now);
#else
    shimmer::updateLights(now);
#endif

    gNeoPixelsA.update();
    sleep_us(500);

#if V_BOARD == 1
    gNeoPixelsB.update();
#endif
    sleep_us(500);
}

int main()
{
    gpio_init(LedPin);
    gpio_set_dir(LedPin, GPIO_OUT);
    gpio_put(LedPin, 1);

    // we have hardware random bits, let's use some!
    g_randGenerator.seed(get_rand_32());

    //gNeoPixelsA.set_grb_order(board4);  // board4 has pixels with GRB order addressing :|
    // update: seems like perhaps this is something that's nondeterministic. reading the data sheet, i think it's expecting only 24bits per pixel, not 32

    //stdio_usb_init();

    winter::init_particles();
    shimmer::init_particles();

    for(;;)
    {
        loop();
    }

    return 0;
}
