#pragma once

#include <optional>
#include <random>
#include <vector>

#include "Gem.h"

static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

enum BonusType { Recolor, Bomb };

class Board {
public:
  static constexpr int Width = 8;
  static constexpr int Height = 8;
  static constexpr int TileSize = 64;

public:
  Board();

  void Draw(sf::RenderWindow &window, const std::optional<sf::Vector2i> &selected);
  bool Swap(int x1, int y1, int x2, int y2);
  void Update(float dt);
  bool IsInside(int x, int y) const;

private:
  std::vector<std::vector<std::optional<Gem>>> m_Grid;
  std::mt19937 m_Rng;
  bool m_Animating = false;

private:
  void Generate();
  Gem RandomGem();
  bool FindMatches();
  void RemoveMatches();
  void ApplyGravity();
  void FillEmpty();
  void SpawnRandomBonus(int x, int y, GemColor sourceColor);
  void ApplyBomb(int x, int y);
  void ApplyRecolor(int x, int y, GemColor color);
  void StartDestroy();
  bool HasDestroyAnimation() const;
  void RemoveDestroyed();
  void UpdateVisuals(float dt);
};
