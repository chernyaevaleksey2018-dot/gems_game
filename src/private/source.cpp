#include "source.h"

sf::CircleShape createTriangle() {
  sf::CircleShape triangle(200.f, 3);

  triangle.setPosition({250.f, 150.f});
  triangle.setFillColor(sf::Color(200, 100, 50, 255));

  return triangle;
}
