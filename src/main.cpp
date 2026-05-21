#include <SFML/Graphics.hpp>
#include "source.h"

using namespace sf;

int main() {
  sf::RenderWindow window(VideoMode({800, 600}), "SFML Triangle");

  auto triangle = createTriangle();

  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      if (event->is<Event::Closed>()) {
        window.close();
      }
    }

    window.clear();
    window.draw(triangle);
    window.display();
  }

  return 0;
}
