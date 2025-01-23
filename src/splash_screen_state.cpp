#include "splash_screen_state.hpp"
#include "components.hpp"
#include "world_system.hpp"
#include "options_menu.hpp"
#include "cutscene.hpp"
#include "serialize.hpp"
#include <iostream>

std::unique_ptr<Sprite> backgroundImage = nullptr;

SplashScreenState::SplashScreenState() : hasLoaded(false), music(Mix_LoadMUS(audio_path("title.wav").c_str())) {
    backgroundImage = std::make_unique<Sprite>(renderSystem.loadTexture("background.png"));
}

SplashScreenState::~SplashScreenState() {
    if (music != nullptr) {
        Mix_FreeMusic(music);
        music = nullptr;
    }
    SplashScreenState::cleanup();
}

void SplashScreenState::init()
{
    Sprite splashSprite = renderSystem.loadTexture("splash_screen.png");

    registry.transforms.emplace(splashScreenEntity, TransformComponent{
        vec3(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f - 100.f, 0.0f),
        vec3(splashSprite.width, splashSprite.height, 1.0f), 0.0
        });
    registry.sprites.emplace(splashScreenEntity, splashSprite);

    Sprite namesSprite = renderSystem.loadTexture("names.png");
    registry.transforms.emplace(namesEntity, TransformComponent{
        vec3(renderSystem.getWindowWidth() / 2.f, 50.f, 0.f),
        vec3(namesSprite.width / 2.f, namesSprite.height / 2.f, 1.f), 0.f
        });
    registry.sprites.emplace(namesEntity, namesSprite);

    MenuItem optionsComponent(renderSystem.loadTexture("menu/options_active.png"), renderSystem.loadTexture("menu/options_inactive.png"),
        renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f + 150.f);
    registry.menuItems.emplace(optionsEntity, optionsComponent);

    MenuItem quitComponent(renderSystem.loadTexture("menu/quit_active.png"), renderSystem.loadTexture("menu/quit_inactive.png"),
        renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f + 150.f + optionsComponent.transformInactive.scale.y * 3);
    registry.menuItems.emplace(quitEntity, quitComponent);

    Sprite escSprite(renderSystem.loadTexture("tutorial/esc_key.PNG"));
    registry.sprites.emplace(esc_key, escSprite);
    registry.transforms.emplace(esc_key, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.94f, 0.f),
        vec3(escSprite.width * 0.3f, escSprite.height * 0.3f, 1.f), 0.f
        });
    Mix_PlayMusic(music, -1);
}

void SplashScreenState::on_key(int key, int, int action, int)
{
    SaveFile sf;
    readFromSaveFile(SAVE_FILE_PATH, sf);
    if (sf.is_init && action == GLFW_PRESS && !hasLoaded)
    {
        hasLoaded = true;
        renderSystem.getGameStateManager()->changeState<OpeningCutscene>();
    }
    else if (action == GLFW_PRESS && !hasLoaded) {
        hasLoaded = true;
        renderSystem.getGameStateManager()->changeState<WorldSystem>();
    }
}

void SplashScreenState::update(float )
{
}

void SplashScreenState::on_mouse_click(int button, int action, const glm::vec2& position, int) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (registry.menuItems.get(optionsEntity).isPointWithin(mouse_pos)) {
            renderSystem.getGameStateManager()->pauseState<OptionsMenu>();
        }
        else if (registry.menuItems.get(quitEntity).isPointWithin(mouse_pos)) {
            renderSystem.closeWindow();
        }
    }
}

void SplashScreenState::render() {
    MenuState::render();

    {
        // Retrieve the Sprite and TransformComponent using the registry
        auto& sprite = registry.sprites.get(splashScreenEntity);
        auto& transform = registry.transforms.get(splashScreenEntity);

        // Use the render system to draw the entity
        renderSystem.drawEntity(sprite, transform);
    }
    {
        // Retrieve the Sprite and TransformComponent using the registry
        auto& sprite = registry.sprites.get(namesEntity);
        auto& transform = registry.transforms.get(namesEntity);

        // Use the render system to draw the entity
        renderSystem.drawEntity(sprite, transform);
    }
    renderMenuItem(registry.menuItems.get(optionsEntity), mouse_pos);
    renderMenuItem(registry.menuItems.get(quitEntity), mouse_pos);

    renderSystem.renderText("Press Any Key To Start", window_width_px * 0.4f, window_height_px * 0.80f, 1.0f, vec3(1), mat4(1));

    renderSystem.drawEntity(registry.sprites.get(esc_key), registry.transforms.get(esc_key));
    renderSystem.renderText("For Pause Menu", window_width_px * 0.1f, window_height_px * 0.05f, 0.5f, vec3(1), mat4(1));
}

void SplashScreenState::cleanup() {
    registry.remove_all_components_of(splashScreenEntity);
    registry.remove_all_components_of(namesEntity);
    registry.remove_all_components_of(quitEntity);
    registry.remove_all_components_of(optionsEntity);
    registry.remove_all_components_of(esc_key);
}
