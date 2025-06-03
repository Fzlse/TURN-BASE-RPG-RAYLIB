// WarriorFactory.h
#pragma once
#include "EnemyFactory.h"
#include "PaladinEnemy.h"

class PaladinFactory : public EnemyFactory {
public:
    Enemy* CreateEnemy(int level) const override {
        return new PaladinEnemy(level);
    }
};
