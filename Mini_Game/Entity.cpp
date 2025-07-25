#include "Entity.h"

Entity::Entity(int x, int y, int hp, char symbol)
    : x(x), y(y), hp(hp), symbol(symbol) {}


int Entity::getX() const { return x; }
int Entity::getY() const { return y; }
char Entity::getSymbol() const { return symbol; }
int Entity::getHP() const { return hp; }

void Entity::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

void Entity::takeDamage(int dmg) {
    hp -= dmg;
}

bool Entity::isAlive() const {
    return hp > 0;
}