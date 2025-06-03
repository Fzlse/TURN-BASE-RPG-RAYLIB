// WarriorEnemy.h
#pragma once
#include "Enemy.h"

class WarriorEnemy : public Enemy {
    int level;
public:
    WarriorEnemy(int lvl) : level(lvl) {}
    std::string GetName() const override { return "Warrior"; }
    int GetMaxHP() const override { return 70 + (level * 12); }
    int GetAttack() const override { return 12 + (level * 2); }
    int GetDefense() const override { return 4 + (level * 1); }
};
