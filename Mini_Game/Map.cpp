#include "Map.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

Map::Map(int width, int height)
    : width(width), height(height), grid(height, std::vector<char>(width, ' ')) {}

    
int exitX, exitY;
void Map::loadLevel(const LevelData& data, std::shared_ptr<Player> existingPlayer) {
    width = data.width;
    height = data.height;
    grid.assign(height, std::vector<char>(width, ' '));
    items = data.items;
    enemies.clear();
    for (int x = 0; x < width; ++x) {
        grid[0][x] = '#';            
        grid[height - 1][x] = '#';    
    }
    for (int y = 0; y < height; ++y) {
        grid[y][0] = '#';            
        grid[y][width - 1] = '#';     
    }
    for (const auto& wall : data.walls) {
        grid[wall.second][wall.first] = '#';
    }

    for (const auto& pos : data.enemyPositions) {
        enemies.push_back(std::make_shared<Enemy>(pos.first, pos.second));
    }

    exitX = data.exitPosition.first;
    exitY = data.exitPosition.second;
    grid[exitY][exitX] = 'X';

    if (existingPlayer) {
        player = existingPlayer;
        player->setPosition(data.playerStart.first, data.playerStart.second);
    } else {
        player = std::make_shared<Player>(data.playerStart.first, data.playerStart.second);
    }
}
bool Map::isExitReached() const {
    return player->getX() == exitX && player->getY() == exitY;
}


void Map::checkForItemPickup() {
    auto it = items.begin();
    while (it != items.end()) {
        if (it->getX() == player->getX() && it->getY() == player->getY()) {
            std::cout << "Picked up " << it->getName() << "! ";

            if (it->getType() == ItemType::Heal) {
                std::cout << "+" << it->getValue() << " HP.\n";
                player->takeDamage(-it->getValue());
            } else if (it->getType() == ItemType::Weapon) {
                std::cout << "Damage +" << it->getValue() << "!\n";
                player->increaseDamage(it->getValue());
            }

            it = items.erase(it);
        } else {
            ++it;
        }
    }
}


void Map::render() const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            bool printed = false;

            if (player->isAlive() && player->getX() == x && player->getY() == y) {
                std::cout << '@';
                continue;
            }

            for (const auto& enemy : enemies) {
                if (enemy->isAlive() && enemy->getX() == x && enemy->getY() == y) {
                    std::cout << 'E';
                    printed = true;
                    break;
                }
            }
            if (printed) continue;

            for (const auto& item : items) {
                if (item.getX() == x && item.getY() == y) {
                    std::cout << item.getSymbol();  
                    printed = true;
                    break;
                }
            }
            if (printed) continue;

            
            std::cout << grid[y][x];
        }
        std::cout << "\n";
    }

    std::cout << "HP: " << player->getHP() << " | Damage: " << player->getDamage() << "\n";
}


std::shared_ptr<Player> Map::getPlayer() {
    return player;
}

std::vector<std::shared_ptr<Enemy>>& Map::getEnemies() {
    return enemies;
}

bool Map::isWalkable(int x, int y) const {
    if (x <= 0 || y <= 0 || x >= width - 1 || y >= height - 1)
        return false;
    return grid[y][x] == ' ' || grid[y][x] == 'X';
}

void Map::movePlayer(int dx, int dy) {
    int newX = player->getX() + dx;
    int newY = player->getY() + dy;

    for (auto& enemy : enemies) {
        if (enemy->isAlive() && enemy->getX() == newX && enemy->getY() == newY) {
            enemy->takeDamage(player->getDamage());
            std::cout << "You hit the enemy for " << player->getDamage() << " damage!\n";
            return;
        }
    }

    if (isWalkable(newX, newY)) {
        player->setPosition(newX, newY);
        checkForItemPickup();
    }
}

void Map::updateEnemies() {
    for (auto& enemy : enemies) {
        if (!enemy->isAlive()) continue;

        int dx = 0, dy = 0;

        int distX = abs(enemy->getX() - player->getX());
        int distY = abs(enemy->getY() - player->getY());

        if (distX + distY <= 5) {
            if (enemy->getX() < player->getX()) dx = 1;
            else if (enemy->getX() > player->getX()) dx = -1;
            if (enemy->getY() < player->getY()) dy = 1;
            else if (enemy->getY() > player->getY()) dy = -1;
        } else {
            dx = enemy->getDirX();
            dy = enemy->getDirY();
        }

        int newX = enemy->getX() + dx;
        int newY = enemy->getY() + dy;

        if (newX == player->getX() && newY == player->getY()) {
            player->takeDamage(1);
            std::cout << "Enemy hits you!\n";
        } else if (isWalkable(newX, newY)) {
            enemy->setPosition(newX, newY);
        } else {
            enemy->setDirection(-dx, -dy);
        }
    }
}
bool Map::areAllEnemiesDefeated() const {
    for (const auto& enemy : enemies) {
        if (enemy->isAlive())
            return false;
    }
    return true;
}


