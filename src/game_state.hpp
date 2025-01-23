#pragma once

#include "common.hpp"

/*
This abstract class is the parent class for all different states of the game
Eg: splash screen, actual game, menus (tbd) etc.
*/
class GameState {
public:
    virtual ~GameState() = default;

    virtual void init() = 0;
    virtual void on_key(int key, int scancode, int action, int mods) = 0;
    virtual void on_mouse_move(const glm::vec2& position) = 0;
    virtual void on_mouse_click(int button, int action, const glm::vec2& position, int mods) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void cleanup() = 0;
    virtual void pause() {}
    virtual void resume() {};
};
