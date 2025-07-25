#include "Item.h"

Item::Item(int x, int y, std::string name, ItemType type, int value)
    : x(x), y(y), name(name), type(type), value(value), symbol('*') {}

int Item::getX() const { return x; }
int Item::getY() const { return y; }
char Item::getSymbol() const {
    switch (type) {
        case ItemType::Heal:
            return 'H'; 
        case ItemType::Weapon:
            return 'W'; 
        default:
            return '*';
    }
}

std::string Item::getName() const { return name; }
ItemType Item::getType() const { return type; }
int Item::getValue() const { return value; }
