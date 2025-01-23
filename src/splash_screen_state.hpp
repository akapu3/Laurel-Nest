#pragma once

#include "game_state.hpp"
#include "ecs.hpp"
#include "render_system.hpp"
#include "pause_state.hpp"

class SplashScreenState : public MenuState {
public:
    SplashScreenState();
    ~SplashScreenState();
    void init() override;
    void on_key(int key, int scancode, int action, int mods) override;
    void on_mouse_click(int button, int action, const glm::vec2& position, int mods) override;
    void update(float deltaTime) override;
    void render() override;
    void cleanup() override;

private:
    Entity splashScreenEntity;
    Entity namesEntity;
    Entity quitEntity;
    Entity optionsEntity;
    Entity esc_key;
    bool hasLoaded;
    Mix_Music* music;
};

extern std::unique_ptr<Sprite> backgroundImage;
