// ArcherEnemy.h
#pragma once
#include "Enemy.h"

class ArcherEnemy : public Enemy {
    int level;
public:
    ArcherEnemy(int lvl) : level(lvl) {}
    std::string GetName() const override { return "Archer"; }
    int GetMaxHP() const override { return 50 + (level * 10); }
    int GetAttack() const override { return 10 + (level * 2); }
    int GetDefense() const override { return 2 + (level * 1); }
};
