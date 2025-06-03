// EnemyFactory.h
#pragma once
#include "Enemy.h"

class EnemyFactory {
public:
    virtual ~EnemyFactory() = default;
    virtual Enemy* CreateEnemy(int level) const = 0;
};
