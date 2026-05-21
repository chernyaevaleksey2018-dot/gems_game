#pragma once

#include <SFML/Graphics.hpp>

enum class GemColor {
    Red,
    Green,
    Blue,
    Yellow,
    Purple,
    Count
};

struct Gem {
    GemColor color;

    bool marked = false;

    sf::Vector2f visualPosition;
    sf::Vector2f targetPosition;

    float destroyTimer = 0.f;

    Gem() = default;

    Gem(GemColor c)
        : color(c) {}
};

inline sf::Color ToSFMLColor(GemColor c) {
    switch (c) {
        case GemColor::Red:    return sf::Color::Red;
        case GemColor::Green:  return sf::Color::Green;
        case GemColor::Blue:   return sf::Color::Blue;
        case GemColor::Yellow: return sf::Color::Yellow;
        case GemColor::Purple: return sf::Color(180, 0, 255);
        default: return sf::Color::White;
    }
}
