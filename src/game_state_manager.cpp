#include "game_state_manager.hpp"

void GameStateManager::resumeState() {
    if (!pausedState.empty()) {
        currentState = std::move(pausedState.top()); // do not init
        pausedState.pop();
    }
}

void GameStateManager::on_key(int key, int scancode, int action, int mods)
{
    if (currentState)
    {
        currentState->on_key(key, scancode, action, mods);
    }
}

void GameStateManager::on_mouse_move(const glm::vec2& position) {
    if (currentState) {
        currentState->on_mouse_move(position);
    }
}

void GameStateManager::on_mouse_click(int button, int action, const glm::vec2& position, int mods) {
    if (currentState) {
        currentState->on_mouse_click(button, action, position, mods);
    }
}

void GameStateManager::update(float deltaTime)
{
    if (currentState)
    {
        currentState->update(deltaTime);
    }
}

void GameStateManager::render()
{
    if (currentState)
    {
        currentState->render();
    }
}

GameState* GameStateManager::getCurrentState() const
{
    return currentState.get();
}
