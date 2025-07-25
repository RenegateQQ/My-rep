#pragma once

#include <string>

enum class ItemType {
    Heal,
    Weapon
};

class Item {
private:
    int x, y;
    char symbol;
    std::string name;
    ItemType type;
    int value; 

public:
    Item(int x, int y, std::string name, ItemType type, int value);

    int getX() const;
    int getY() const;
    char getSymbol() const;
    std::string getName() const;
    ItemType getType() const;
    int getValue() const;
};
