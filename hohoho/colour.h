#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>


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

struct FHsv
{
    float h, s, v;

    FHsv() = default;
    FHsv(float h, float s, float v) : h(h), s(s), v(v) { /**/ }

    FHsv operator*(float k)
    {
        k = std::clamp(k, 0.f, 1.f);
        return {h * k, s * k, v * k};
    }

    Rgb toRgbScaled(float rScale, float gScale, float bScale);
};


inline Rgb FHsv::toRgbScaled(float rScale, float gScale, float bScale)
{
    using u8 = uint8_t;

    const float h6 = h * 6.0f;
    const float i = floor(h6);
    const float f = h6 - i;
    const float v255 = v * 255.0f;
    const float p = v255 * (1.0f - s);
    const float q = v255 * (1.0f - f * s);
    const float t = v255 * (1.0f - (1.0f - f) * s);

    switch (int(i) % 6) {
        case 0:
            return { u8(v255 * rScale), u8(t * gScale), u8(p * bScale) };
        case 1:
            return { u8(q * rScale), u8(v255 * gScale), u8(p * bScale) };
        case 2:
            return { u8(p * rScale), u8(v255 * gScale), u8(t * bScale) };
        case 3:
            return { u8(p * rScale), u8(q * gScale), u8(v255 * bScale) };
        case 4:
            return { u8(t * rScale), u8(p * gScale), u8(v255 * bScale) };
        case 5:
            return { u8(v255 * rScale), u8(p * gScale), u8(q * bScale) };
    }

    return { 128, 128, 128 };
}


/*
if B-A < -0.5
{
    v = t(B + 1 - A) + A
}
else if B-A > 0.5
{
    v = A + 1 + t(B - A - 1)
}
else
{
    v = A + t(B - A)
}
if (v > 1)
    v -= 1


*/


inline FHsv lerp(const FHsv& a, const FHsv& b, float t)
{
    float dh = b.h - a.h;
    float h;
    if (dh < -0.5f)
        h = a.h + (t * (dh + 1.f));
    else if (dh > 0.5f)
        h = a.h + 1.0f + (t * (dh - 1.f));
    else
        h = a.h + t * dh;
    if (h >= 1.f)
        h -= 1.f;

    return FHsv {
        h,
        std::lerp(a.s, b.s, t),
        std::lerp(a.v, b.v, t)
    };
}

