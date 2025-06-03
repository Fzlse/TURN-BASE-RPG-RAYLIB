// PaladinEnemy.h
#pragma once
#include "Enemy.h"

class PaladinEnemy : public Enemy {
    int level;
public:
    PaladinEnemy(int lvl) : level(lvl) {}
    std::string GetName() const override { return "Paladin"; }
    int GetMaxHP() const override { return 90 + (level * 15); }
    int GetAttack() const override { return 8 + (level * 1); }
    int GetDefense() const override { return 6 + (level * 2); }
};
