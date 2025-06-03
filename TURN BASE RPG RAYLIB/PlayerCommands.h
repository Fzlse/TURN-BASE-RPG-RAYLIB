#pragma once
#include "Command.h"
#include "Game.h"

class AttackCommand : public Command {
public:
    void Execute(Game& game) override { game.PlayerAttack(); }
};

class SkillCommand : public Command {
public:
    void Execute(Game& game) override { game.PlayerSkill(); }
};

class BlockCommand : public Command {
public:
    void Execute(Game& game) override { game.isBlocking = true; game.ShowNotification("You block incoming attack!"); }
};

class RunCommand : public Command {
public:
    void Execute(Game& game) override { game.ShowNotification("You fled from battle."); game.state = GameState::Arena; }
};
