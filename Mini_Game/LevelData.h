#pragma once
#include <vector>
#include "Enemy.h"
#include "Item.h"

struct LevelData {
    int width;
    int height;
    std::vector<std::pair<int, int>> walls;
    std::vector<std::pair<int, int>> enemyPositions;
    std::vector<Item> items;
    std::pair<int, int> playerStart;
    std::pair<int, int> exitPosition;
};
