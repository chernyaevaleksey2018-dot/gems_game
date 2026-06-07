#pragma once
#include <SFML/Graphics.hpp>
#include "Board.h"

class Game {
public:
    Game();
    void Run();
private:
    sf::RenderWindow m_Window;
    Board m_Board;
    std::optional<sf::Vector2i> m_Selected;
    sf::Vector2i m_SelectedCell;
private:
    void ProcessEvents();
    void Render();
};
