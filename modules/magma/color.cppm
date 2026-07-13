module;
#include <native/macros.h>

// ============================ //
//     MAGMA - COLOR MODULE     //
// ============================ //

export module Obsidian.Magma:Color;
import std;
import :Vector;

EXPORT(obsidian)

struct alignas(16) Color
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    constexpr Color() = default;
    constexpr Color(float red, float green, float blue, float alpha = 1.0f) : r(red), g(green), b(blue), a(alpha) { }

    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;

    [[nodiscard]] auto luminance() const -> float
    {
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
    }
};

inline constexpr Color Color::White       = { 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Color Color::Black       = { 0.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::Red         = { 1.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::Green       = { 0.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Color Color::Blue        = { 0.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Color Color::Yellow      = { 1.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Color Color::Magenta     = { 1.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Color Color::Cyan        = { 0.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Color Color::Transparent = { 0.0f, 0.0f, 0.0f, 0.0f };

struct ToneMapper
{
    static auto reinhard(const Color& color, float white_point = -1.0f) -> Color
    {
        if (white_point > 0.0f)
        {
            float white_point2 = white_point * white_point;
            return Color(
                (color.r * (1.0f + color.r / white_point2)) / (1.0f + color.r),
                (color.g * (1.0f + color.g / white_point2)) / (1.0f + color.g),
                (color.b * (1.0f + color.b / white_point2)) / (1.0f + color.b),
                color.a
            );
        }

        return Color(
            color.r / (1.0f + color.r),
            color.g / (1.0f + color.g),
            color.b / (1.0f + color.b),
            color.a
        );
    }

    static auto ACES(const Color& color) -> Color
    {
        constexpr float a = 2.51f;
        constexpr float b = 0.03f;
        constexpr float c = 2.43f;
        constexpr float d = 0.59f;
        constexpr float e = 0.14f;

        auto tone_map = [](float x) -> float
        {
            return std::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
        };

        return Color(
            tone_map(color.r),
            tone_map(color.g),
            tone_map(color.b),
            color.a
        );
    }

    static auto exposure(const Color& color, float exposure) -> Color
    {
        return Color(
            1.0f - std::exp(-color.r * exposure),
            1.0f - std::exp(-color.g * exposure),
            1.0f - std::exp(-color.b * exposure),
            color.a
        );
    }

    static auto uncharted(const Color& color) -> Color
    {
        constexpr float A = 0.15f;
        constexpr float B = 0.50f;
        constexpr float C = 0.10f;
        constexpr float D = 0.20f;
        constexpr float E = 0.02f;
        constexpr float F = 0.30f;
        constexpr float W = 11.2f;

        auto tone_map = [](float x) -> float
        {
            return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - (E / F);
        };

        constexpr float white_scale_inv = 1.0f / tone_map(W);

        auto process_channel = [&](const float c) -> float
        {
            float exposed = c * 2.0f;                       // Exposure bias
            float mapped = tone_map(exposed);               // Apply curve
            float normalized = mapped * white_scale_inv;    // Normalize to white point
            return normalized; 
        };

        return Color(
            process_channel(color.r),
            process_channel(color.g),
            process_channel(color.b),
            color.a
        );
    }
};

auto rgb_to_linear(const Color& color) -> Color
{
    return Color(
        std::pow(color.r, 2.2f),
        std::pow(color.g, 2.2f),
        std::pow(color.b, 2.2f),
        color.a
    );
}

auto linear_to_rgb(const Color& color) -> Color
{
    return Color(
        std::pow(color.r, 1.0f / 2.2f),
        std::pow(color.g, 1.0f / 2.2f),
        std::pow(color.b, 1.0f / 2.2f),
        color.a
    );
}

auto hex_to_color(std::string_view hex) -> Color
{
    if (hex.size() != 7 && hex.size() != 9)
    {
        throw std::invalid_argument("Invalid hex color format. Expected #RRGGBB or #RRGGBBAA");
    }

    unsigned int r = 0, g = 0, b = 0, a = 255;
    std::from_chars(hex.data() + 1, hex.data() + 3, r, 16);
    std::from_chars(hex.data() + 3, hex.data() + 5, g, 16);
    std::from_chars(hex.data() + 5, hex.data() + 7, b, 16);
    
    if (hex.size() == 9)
    {
        std::from_chars(hex.data() + 7, hex.data() + 9, a, 16);
    }

    return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

auto color_to_hex(const Color& color) -> std::string
{
    auto toHex = [](float value) -> std::string
    {
        int intValue = static_cast<int>(std::round(value * 255.0f));
        char buffer[3];
        std::snprintf(buffer, sizeof(buffer), "%02X", intValue);
        return std::string(buffer);
    };

    return "#" + toHex(color.r) + toHex(color.g) + toHex(color.b) + toHex(color.a);
}

auto hsv_to_rgb(Vector<float> hsv) -> Color
{
    float h = hsv.x();
    float s = hsv.y();
    float v = hsv.z();

    int i = static_cast<int>(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    switch (i % 6)
    {
        case 0: return Color(v, t, p);
        case 1: return Color(q, v, p);
        case 2: return Color(p, v, t);
        case 3: return Color(p, q, v);
        case 4: return Color(t, p, v);
        case 5: return Color(v, p, q);

        default: return Color(0.0f, 0.0f, 0.0f);
    }
}

auto rgb_to_hsv(const Color& color) -> Vector<float>
{
    float r = color.r;
    float g = color.g;
    float b = color.b;

    float max = std::max({ r, g, b });
    float min = std::min({ r, g, b });
    float delta = max - min;

    float h = 0.0f;
    if (delta > 0.0f)
    {
        if (max == r)
        {
            h = 60.0f * (std::fmod(((g - b) / delta), 6.0f));
        }
        else if (max == g)
        {
            h = 60.0f * (((b - r) / delta) + 2.0f);
        }
        else if (max == b)
        {
            h = 60.0f * (((r - g) / delta) + 4.0f);
        }
    }

    float s = (max == 0.0f) ? 0.0f : (delta / max);
    float v = max;

    return Vector<float>(h / 360.0f, s, v);
}

auto kelvin_to_rgb(float kelvin) -> Color
{
    float temperature = kelvin / 100.0f;
    float r, g, b;

    if (temperature <= 66.0f)
    {
        r = 255.0f;
        g = std::clamp(99.4708025861f * std::log(temperature) - 161.1195681661f, 0.0f, 255.0f);
        b = (temperature <= 19.0f) ? 0.0f : std::clamp(138.5177312231f * std::log(temperature - 10.0f) - 305.0447927307f, 0.0f, 255.0f);
    }
    else
    {
        r = std::clamp(329.698727446f * std::pow(temperature - 60.0f, -0.1332047592f), 0.0f, 255.0f);
        g = std::clamp(288.1221695283f * std::pow(temperature - 60.0f, -0.0755148492f), 0.0f, 255.0f);
        b = 255.0f;
    }

    return Color(r / 255.0f, g / 255.0f, b / 255.0f);
}

EXPORT_END