#pragma once

#include <vector>
#include <memory>
#include "Player.h"
#include "Enemy.h"
#include "Item.h"
#include "LevelData.h"

class Map {
private:
    int width;
    int height;
    std::vector<std::vector<char>> grid;

    std::shared_ptr<Player> player;
    std::vector<std::shared_ptr<Enemy>> enemies;
    std::vector<Item> items;

public:
    Map(int width, int height);

    void initialize();
    void render() const;

    std::shared_ptr<Player> getPlayer();
    std::vector<std::shared_ptr<Enemy>>& getEnemies();
    void loadLevel(const LevelData& data, std::shared_ptr<Player> existingPlayer = nullptr);
    bool isExitReached() const;
    bool areAllEnemiesDefeated() const;
    bool isWalkable(int x, int y) const;
    void movePlayer(int dx, int dy);
    void updateEnemies();
    void checkForItemPickup();

private:
    void placeStaticObjects();
};
