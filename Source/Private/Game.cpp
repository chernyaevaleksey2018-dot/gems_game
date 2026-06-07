#include "Game.h"

Game::Game()
    : m_Window(sf::VideoMode({Board::Width * Board::TileSize, Board::Height * Board::TileSize}), "GEMS") {}

void Game::Run() { //запуск
  sf::Clock clock;
  while (m_Window.isOpen()) {
    float dt = clock.restart().asSeconds();
    ProcessEvents();
    m_Board.Update(dt);
    Render();
  }
}

void Game::ProcessEvents() { 
  while (const std::optional event = m_Window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) { m_Window.close(); }

    if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
      int x = mouseEvent->position.x / Board::TileSize;

      int y = mouseEvent->position.y / Board::TileSize;

      if (!m_Selected.has_value()) {
        m_Selected = sf::Vector2i(x, y);
      } else {
        m_Board.Swap(m_Selected->x, m_Selected->y, x, y);
        m_Selected.reset();
      }
    }
  }
}

void Game::Render() {
  m_Window.clear(sf::Color::Black);
  m_Board.Draw(m_Window, m_Selected);
  m_Window.display();
}
