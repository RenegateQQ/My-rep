#pragma once
#include "Entity.h"

class Player : public Entity {
private:
    int damage;

public:
    Player(int x, int y);

    std::string getType() const override;
    int getDamage() const;
    void increaseDamage(int bonus);
};
