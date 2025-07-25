#pragma once
#include "Entity.h"

class Enemy : public Entity {
private:
    int dirX, dirY;

public:
    Enemy(int x, int y);

    std::string getType() const override;

    int getDirX() const;
    int getDirY() const;
    void setDirection(int dx, int dy);
};

