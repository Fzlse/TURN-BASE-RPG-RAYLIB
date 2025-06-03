#pragma once

class Game;

class Command {
public:
    virtual ~Command() {}
    virtual void Execute(Game& game) = 0;
};
