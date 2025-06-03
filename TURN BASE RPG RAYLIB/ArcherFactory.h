// ArcherFactory.h
#pragma once
#include "EnemyFactory.h"
#include "ArcherEnemy.h"

class ArcherFactory : public EnemyFactory {
public:
    Enemy* CreateEnemy(int level) const override {
        return new ArcherEnemy(level);
    }
};