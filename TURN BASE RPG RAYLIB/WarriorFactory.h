// WarriorFactory.h
#pragma once
#include "EnemyFactory.h"
#include "WarriorEnemy.h"

class WarriorFactory : public EnemyFactory {
public:
    Enemy* CreateEnemy(int level) const override {
        return new WarriorEnemy(level);
    }
};
