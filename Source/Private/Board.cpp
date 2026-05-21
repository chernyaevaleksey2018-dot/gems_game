#include "Board.h"

#include <ctime>
#include <algorithm>
#include <map>

Board::Board() : m_Rng(static_cast<unsigned>(std::time(nullptr))) {
  Generate();
}

void Board::Generate() {
  m_Grid.resize(Height);

  for (int y = 0; y < Height; y++) {
    m_Grid[y].resize(Width);

    for (int x = 0; x < Width; x++) {

      std::vector<GemColor> forbidden;

      // Проверяем два квадрата слева
      if (x >= 2 && m_Grid[y][x - 1].has_value() &&
          m_Grid[y][x - 2].has_value()) {
        if (m_Grid[y][x - 1]->color == m_Grid[y][x - 2]->color) {
          forbidden.push_back(m_Grid[y][x - 1]->color);
        }
      }

      // Проверяем два квадрата сверху
      if (y >= 2 && m_Grid[y - 1][x].has_value() &&
          m_Grid[y - 2][x].has_value()) {
        if (m_Grid[y - 1][x]->color == m_Grid[y - 2][x]->color) {
          forbidden.push_back(m_Grid[y - 1][x]->color);
        }
      }

      // Генерируем случайный цвет, избегая forbidden
      Gem gem;
      do {
        gem = RandomGem();
      } while (std::find_if(forbidden.begin(), forbidden.end(),
                            [gem](const Gem& g) {
                              return g.color == gem.color;
                            }) != forbidden.end());

      gem.visualPosition = {static_cast<float>(x * TileSize),
                            static_cast<float>(y * TileSize)};

      gem.targetPosition = gem.visualPosition;

      m_Grid[y][x] = gem;
    }
  }
}

Gem Board::RandomGem() {
  std::uniform_int_distribution<int> dist(0, static_cast<int>(GemColor::Count) -
                                                 1);

  return Gem(static_cast<GemColor>(dist(m_Rng)));
}

bool Board::IsInside(int x, int y) const {
  return x >= 0 && x < Width && y >= 0 && y < Height;
}

void Board::Draw(sf::RenderWindow &window,
                 const std::optional<sf::Vector2i> &selected) {
  for (int y = 0; y < Height; y++) {
    for (int x = 0; x < Width; x++) {

      sf::RectangleShape bg;

      bg.setSize(
          {static_cast<float>(TileSize - 1), static_cast<float>(TileSize - 1)});

      bg.setPosition(
          {static_cast<float>(x * TileSize), static_cast<float>(y * TileSize)});

      bg.setFillColor(sf::Color(20, 20, 20));

      window.draw(bg);

      if (!m_Grid[y][x].has_value())
        continue;

      auto &gem = *m_Grid[y][x];

      sf::RectangleShape rect;

      rect.setSize(
          {static_cast<float>(TileSize - 6), static_cast<float>(TileSize - 6)});

      rect.setPosition(
          {gem.visualPosition.x + 3.f, gem.visualPosition.y + 3.f});

      sf::Color color = ToSFMLColor(gem.color);

      if (gem.marked) {

        float alpha = std::max(0.f, gem.destroyTimer) * 255.f;

        color.a = (alpha);
      }

      rect.setFillColor(color);

      if (selected.has_value()) {

        if (selected->x == x && selected->y == y) {
          rect.setOutlineThickness(4.f);
          rect.setOutlineColor(sf::Color::White);
        }
      }

      window.draw(rect);
    }
  }
}

bool Board::Swap(int x1, int y1, int x2, int y2) {
  if (!IsInside(x1, y1) || !IsInside(x2, y2)) {
    return false;
  }

  int dx = std::abs(x1 - x2);
  int dy = std::abs(y1 - y2);

  if (dx + dy != 1)
    return false;

  std::swap(m_Grid[y1][x1], m_Grid[y2][x2]);

  auto FixPosition = [&](int x, int y) {
    if (!m_Grid[y][x].has_value())
      return;

    m_Grid[y][x]->targetPosition = {static_cast<float>(x * TileSize),
                                    static_cast<float>(y * TileSize)};
  };

  FixPosition(x1, y1);
  FixPosition(x2, y2);

  if (!FindMatches()) {

    std::swap(m_Grid[y1][x1], m_Grid[y2][x2]);

    FixPosition(x1, y1);
    FixPosition(x2, y2);

    return false;
  }

  StartDestroy();

  return true;
}

bool Board::FindMatches() {
  bool found = false;

  auto markCell = [&](int x, int y) {
    if (IsInside(x, y) && m_Grid[y][x].has_value())
      m_Grid[y][x]->marked = true;
  };

  // Горизонтальные линии >= 3
  for (int y = 0; y < Height; y++) {
    int x = 0;
    while (x < Width) {
      if (!m_Grid[y][x].has_value()) { x++; continue; }
      GemColor color = m_Grid[y][x]->color;
      int len = 1;
      while (x + len < Width && m_Grid[y][x + len].has_value() &&
             m_Grid[y][x + len]->color == color)
        len++;
      if (len >= 3) {
        found = true;
        for (int i = 0; i < len; i++) markCell(x + i, y);
      }
      x += len;
    }
  }

  // Вертикальные линии >= 3
  for (int x = 0; x < Width; x++) {
    int y = 0;
    while (y < Height) {
      if (!m_Grid[y][x].has_value()) { y++; continue; }
      GemColor color = m_Grid[y][x]->color;
      int len = 1;
      while (y + len < Height && m_Grid[y + len][x].has_value() &&
             m_Grid[y + len][x]->color == color)
        len++;
      if (len >= 3) {
        found = true;
        for (int i = 0; i < len; i++) markCell(x, y + i);
      }
      y += len;
    }
  }

  // Квадраты 2x2
  for (int y = 0; y < Height - 1; y++) {
    for (int x = 0; x < Width - 1; x++) {
      if (!m_Grid[y][x].has_value()) continue;
      GemColor color = m_Grid[y][x]->color;
      if (m_Grid[y][x + 1].has_value() && m_Grid[y][x + 1]->color == color &&
          m_Grid[y + 1][x].has_value() && m_Grid[y + 1][x]->color == color &&
          m_Grid[y + 1][x + 1].has_value() && m_Grid[y + 1][x + 1]->color == color) {
        found = true;
        markCell(x, y); markCell(x + 1, y);
        markCell(x, y + 1); markCell(x + 1, y + 1);
      }
    }
  }

  return found;
}

void Board::RemoveMatches() {
  std::uniform_real_distribution<float> chance(0.f, 1.f);

  for (int y = 0; y < Height; y++) {
    for (int x = 0; x < Width; x++) {

      if (!m_Grid[y][x].has_value())
        continue;

      if (!m_Grid[y][x]->marked)
        continue;

      GemColor color = m_Grid[y][x]->color;

      m_Grid[y][x] = std::nullopt;

      if (chance(m_Rng) < 0.25f) {
        SpawnRandomBonus(x, y, color);
      }
    }
  }
}

void Board::ApplyGravity() {
    // Для каждого столбца: собираем все существующие гемы снизу вверх,
    // затем расставляем их заподно к низу — пустые слоты остаются сверху.
    for (int x = 0; x < Width; x++) {
        // Собираем гемы снизу вверх (нижний первым)
        std::vector<Gem> gems;
        for (int y = Height - 1; y >= 0; y--) {
            if (m_Grid[y][x].has_value()) {
                gems.push_back(*m_Grid[y][x]);
            }
        }

        // Расставляем: нижние строки заполняем имеющимися гемами
        for (int y = Height - 1; y >= 0; y--) {
            int idx = Height - 1 - y; // 0 = самый нижний слот
            if (idx < (int)gems.size()) {
                m_Grid[y][x] = gems[idx];
                m_Grid[y][x]->targetPosition = {
                    static_cast<float>(x * TileSize),
                    static_cast<float>(y * TileSize)
                };
            } else {
                m_Grid[y][x] = std::nullopt;
            }
        }
    }
}

void Board::FillEmpty() {
    // Заполняем только пустые слоты (они всегда сверху после ApplyGravity).
    // Спавним блоки за верхним краем экрана с разным смещением,
    // чтобы блоки падали сверху по очереди.
    for (int x = 0; x < Width; x++) {
        int spawnOffset = 1;
        for (int y = 0; y < Height; y++) {
            if (!m_Grid[y][x].has_value()) {
                m_Grid[y][x] = RandomGem();
                auto& gem = *m_Grid[y][x];

                // Спавним за экраном сверху, каждый следующий чуть выше
                gem.visualPosition = {
                    static_cast<float>(x * TileSize),
                    -static_cast<float>(TileSize * spawnOffset)
                };
                gem.targetPosition = {
                    static_cast<float>(x * TileSize),
                    static_cast<float>(y * TileSize)
                };
                spawnOffset++;
            }
        }
    }
}

void Board::SpawnRandomBonus(int x, int y, GemColor sourceColor) {
  std::uniform_int_distribution<int> bonusDist(0, 1);

  BonusType type = static_cast<BonusType>(bonusDist(m_Rng));

  if (type == BonusType::Bomb) {
    ApplyBomb(x, y);
  } else {
    ApplyRecolor(x, y, sourceColor);
  }
}

void Board::ApplyBomb(int x, int y) {
  std::uniform_int_distribution<int> dx(0, Width - 1);
  std::uniform_int_distribution<int> dy(0, Height - 1);

  int destroyed = 0;
  int attempts = 0;
  while (destroyed < 5 && attempts < 100) {
    int rx = dx(m_Rng);
    int ry = dy(m_Rng);
    attempts++;
    if (m_Grid[ry][rx].has_value() && !m_Grid[ry][rx]->marked) {
      m_Grid[ry][rx]->marked = true;
      m_Grid[ry][rx]->destroyTimer = 1.f;
      destroyed++;
    }
  }
}

void Board::ApplyRecolor(int x, int y, GemColor color) {
  Gem g(color);
  // Ставим visualPosition за верхним краем, чтобы блок упал сверху
  g.visualPosition = { static_cast<float>(x * TileSize), -static_cast<float>(TileSize) };
  g.targetPosition = { static_cast<float>(x * TileSize), static_cast<float>(y * TileSize) };
  m_Grid[y][x] = g;

  int changed = 0;

  std::uniform_int_distribution<int> dx(-3, 3);
  std::uniform_int_distribution<int> dy(-3, 3);

  while (changed < 2) {

    int nx = x + dx(m_Rng);
    int ny = y + dy(m_Rng);

    if (!IsInside(nx, ny))
      continue;

    if (std::abs(nx - x) + std::abs(ny - y) <= 1)
      continue;

    if (!m_Grid[ny][nx].has_value())
      continue;

    if (m_Grid[ny][nx]->marked)
      continue;

    m_Grid[ny][nx]->color = color;

    changed++;
  }
}

void Board::StartDestroy() {
  for (auto &row : m_Grid) {
    for (auto &cell : row) {

      if (cell.has_value() && cell->marked) {
        cell->destroyTimer = 1.f;
      }
    }
  }

  m_Animating = true;
}

bool Board::HasDestroyAnimation() const {
  for (const auto &row : m_Grid) {
    for (const auto &cell : row) {

      if (cell.has_value() && cell->marked) {
        return true;
      }
    }
  }

  return false;
}

void Board::UpdateVisuals(float dt) {
  for (auto &row : m_Grid) {
    for (auto &cell : row) {

      if (!cell.has_value())
        continue;

      auto &gem = *cell;

      gem.visualPosition.x =
          Lerp(gem.visualPosition.x, gem.targetPosition.x, dt * 12.f);

      gem.visualPosition.y =
          Lerp(gem.visualPosition.y, gem.targetPosition.y, dt * 12.f);

      if (gem.marked) {

        gem.destroyTimer -= dt * 4.f;
      }
    }
  }
}

void Board::RemoveDestroyed() {
  std::uniform_real_distribution<float> chance(0.f, 1.f);

  struct BonusEvent { int x, y; GemColor color; };

  // Группируем уничтожаемые блоки по цвету: для каждой группы — один бонус
  std::map<GemColor, std::vector<std::pair<int,int>>> groups;

  for (int y = 0; y < Height; y++) {
    for (int x = 0; x < Width; x++) {
      if (!m_Grid[y][x].has_value()) continue;
      auto &gem = *m_Grid[y][x];
      if (!gem.marked) continue;
      groups[gem.color].push_back({x, y});
    }
  }

  // Удаляем все помеченные блоки
  for (int y = 0; y < Height; y++) {
    for (int x = 0; x < Width; x++) {
      if (m_Grid[y][x].has_value() && m_Grid[y][x]->marked)
        m_Grid[y][x] = std::nullopt;
    }
  }

  // Для каждой группы одного цвета — с шансом 25% один бонус в случайной позиции группы
  std::vector<BonusEvent> bonuses;
  for (auto &[color, cells] : groups) {
    if (chance(m_Rng) < 0.25f) {
      std::uniform_int_distribution<int> pick(0, (int)cells.size() - 1);
      auto [bx, by] = cells[pick(m_Rng)];
      bonuses.push_back({bx, by, color});
    }
  }

  for (auto &b : bonuses) {
    SpawnRandomBonus(b.x, b.y, b.color);
  }
}

void Board::Update(float dt) {
    // 1) Обновляем визуальные позиции всех гемов (плавное падение)
    UpdateVisuals(dt);

    // 2) Проверяем, если анимация разрушения активна
    if (m_Animating) {
        bool animStillPlaying = false;

        for (auto& row : m_Grid) {
            for (auto& cell : row) {
                if (cell.has_value() && cell->marked && cell->destroyTimer > 0.f) {
                    animStillPlaying = true;
                    break;
                }
            }
            if (animStillPlaying) break;
        }

        // Если разрушение закончилось
        if (!animStillPlaying) {
            RemoveDestroyed();   // удаляем отмеченные гемы + применяем бонусы
            ApplyGravity();      // падают квадраты
            FillEmpty();         // сверху спавним новые

            // Бомба могла пометить новые блоки — проверяем
            bool hasBombMarked = false;
            for (auto& row : m_Grid) {
                for (auto& cell : row) {
                    if (cell.has_value() && cell->marked) {
                        hasBombMarked = true;
                        break;
                    }
                }
                if (hasBombMarked) break;
            }

            if (hasBombMarked || FindMatches()) {
                StartDestroy();
            } else {
                m_Animating = false;
            }
        }

        return; // ждём завершения анимации
    }

}
