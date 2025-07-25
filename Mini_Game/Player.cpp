#include "Player.h"

Player::Player(int x, int y)
    : Entity(x, y, 10, '@'), damage(2) {}

std::string Player::getType() const {
    return "Player";
}

int Player::getDamage() const {
    return damage;
}

void Player::increaseDamage(int bonus) {
    damage += bonus;
}
