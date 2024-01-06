#pragma once

#include <algorithm>
#include <cstdint>


struct Rgb { uint8_t r, g, b; };
namespace col
{
    constexpr Rgb CalmWhite{130, 70, 20};
    constexpr Rgb NiceRed{180, 0, 0};
    constexpr Rgb NiceGreen{5, 85, 0};
    constexpr Rgb NiceBlue{5, 0, 40};
    constexpr Rgb NiceAmber{130, 35, 0};
    constexpr Rgb NicePink{140, 20, 40};

    constexpr Rgb Dark{0, 0, 2};
};


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


