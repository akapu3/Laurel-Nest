#pragma once
#include "game_state.hpp"
#include "render_system.hpp"

class MenuState : public GameState {
public:
    void on_key(int key, int, int action, int) override;
    void on_mouse_move(const vec2& position) override;
    void render() override;

protected:
    vec2 mouse_pos;
};

class PauseState : public MenuState {
public:
    PauseState();
    ~PauseState();

    void init() override;
    void on_mouse_click(int button, int action, const glm::vec2& position, int mods) override;
    void update(float deltaTime) override;
    void render() override;
    void cleanup() override;

private:
    float timePassed;
    Entity pauseScreenEntity;
    Entity quitEntity;
    Entity optionsEntity;
    Entity esc_key;
    void spline(float t, const std::vector<float>& controlPoints, int which) const;
};