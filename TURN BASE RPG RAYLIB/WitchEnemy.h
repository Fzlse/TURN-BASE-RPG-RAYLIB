// WitchEnemy.h
#pragma once
#include "Enemy.h"

class WitchEnemy : public Enemy {
    int level;
public:
    WitchEnemy(int lvl) : level(lvl) {}
    std::string GetName() const override { return "Witch"; }
    int GetMaxHP() const override { return 60 + (level * 8); }
    int GetAttack() const override { return 9 + (level * 2); }
    int GetDefense() const override { return 3 + (level * 1); }
};
