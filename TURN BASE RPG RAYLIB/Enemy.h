// Enemy.h
#pragma once
#include <string>

class Enemy {
public:
    virtual ~Enemy() = default;

    virtual std::string GetName() const = 0;
    virtual int GetMaxHP() const = 0;
    virtual int GetAttack() const = 0;
    virtual int GetDefense() const = 0;
};
