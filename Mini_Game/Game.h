#pragma once
#include <vector>
#include "Map.h"
#include "LevelData.h"

class Game {
private:
    std::vector<LevelData> levels;
    int currentLevel;
    Map map;
    std::shared_ptr<Player> player;

public:
    Game();

    void run();
};
