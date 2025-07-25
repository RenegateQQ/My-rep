#pragma once

#include <string>

class Entity {
protected:
    int x, y;
    int hp;
    char symbol;

public:
    Entity(int x, int y, int hp, char symbol);
    virtual ~Entity() = default;

    int getX() const;
    int getY() const;
    char getSymbol() const;
    int getHP() const;

    void setPosition(int newX, int newY);
    void takeDamage(int dmg);
    bool isAlive() const;

    virtual std::string getType() const = 0;
};