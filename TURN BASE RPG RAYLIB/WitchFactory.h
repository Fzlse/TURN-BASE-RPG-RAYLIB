// WarriorFactory.h
#pragma once
#include "EnemyFactory.h"
#include "WitchEnemy.h"

class WitchFactory : public EnemyFactory {
public:
    Enemy* CreateEnemy(int level) const override {
        return new WitchEnemy(level);
    }
};
