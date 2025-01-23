#include "game_over_screen.hpp"
#include "ecs_registry.hpp"
#include "game_state_manager.hpp"
#include "splash_screen_state.hpp"
#include <limits>

#include "world_system.hpp"

static float lerp(float a, float b, float t) {
    t = clamp(t, 0, 1);
    return (1 - t) * a + t * b;
}

GameOverScreen::GameOverScreen() : time(0.f), transparency(0.f) {}

GameOverScreen::~GameOverScreen() {
    GameOverScreen::cleanup();
}

void GameOverScreen::init()
{
    Sprite splashSprite = renderSystem.loadTexture("game_over.png");

    TransformComponent splashTransform;
    splashTransform.position = glm::vec3(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f, 0.0f);

    splashTransform.scale = glm::vec3(splashSprite.width, splashSprite.height, 1.0f);
    splashTransform.rotation = 0.0f;

    registry.transforms.emplace(gameOverEntity, std::move(splashTransform));

    registry.sprites.emplace(gameOverEntity, std::move(splashSprite));
}

void GameOverScreen::cleanup() {
    registry.remove_all_components_of(gameOverEntity);
}

void GameOverScreen::on_key(int key, int, int action, int) {
    if (action == GLFW_PRESS && transparency + std::numeric_limits<float>::epsilon() >= 1) {
        renderSystem.getGameStateManager()->changeState<WorldSystem>();
    }
}

void GameOverScreen::on_mouse_move(const vec2&) {}

void GameOverScreen::on_mouse_click(int button, int action, const vec2&, int) {}

void GameOverScreen::render() {
    // Clear the screen
    MenuState::render();

    if (registry.sprites.has(gameOverEntity) &&
        registry.transforms.has(gameOverEntity))
    {
        // Retrieve the Sprite and TransformComponent using the registry
        auto& sprite = registry.sprites.get(gameOverEntity);
        auto& transform = registry.transforms.get(gameOverEntity);

        // Use the render system to draw the entity
        renderSystem.drawEntity(sprite, transform, transparency);
    }

    if (transparency >= 1 - std::numeric_limits<float>::epsilon()) {
        renderSystem.renderText("Press Any Key To Continue", window_width_px * 0.37f, window_height_px * 0.80f, 1.0f, vec3(1), mat4(1));
    }
}

void GameOverScreen::update(float elapsed_ms) {
    time += elapsed_ms;
    transparency = lerp(0.f, 1.f, time);
}
