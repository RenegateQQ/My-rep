#include "Enemy.h"

Enemy::Enemy(int x, int y)
    : Entity(x, y, 5, 'E'), dirX(1), dirY(0) {}

std::string Enemy::getType() const {
    return "Enemy";
}

int Enemy::getDirX() const { return dirX; }
int Enemy::getDirY() const { return dirY; }
void Enemy::setDirection(int dx, int dy) {
    dirX = dx;
    dirY = dy;
}
