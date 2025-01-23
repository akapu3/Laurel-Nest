#include "pause_state.hpp"
#include "ecs_registry.hpp"
#include "splash_screen_state.hpp"
#include "options_menu.hpp"

void MenuState::on_key(int key, int, int action, int) {
    if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        renderSystem.getGameStateManager()->resumeState();
    }
}

void MenuState::on_mouse_move(const vec2& position) {
    mouse_pos = position;
}

void MenuState::render() {
    // Clear the screen
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    TransformComponent transform{ vec3(window_width_px / 2.f, window_height_px / 2.f, 0.f), vec3(window_width_px, window_height_px, 1.f), 0.f };
    renderSystem.drawEntity(*backgroundImage, transform);
}

PauseState::PauseState(): timePassed(0) {}

PauseState::~PauseState() {
    PauseState::cleanup();
}

void PauseState::spline(float t, const std::vector<float>& controlPoints, int which) const {
    t = clamp(t, 0.0f, 1.0f);

    size_t numPoints = controlPoints.size();
    if (numPoints < 4) {
        throw std::invalid_argument("Spline requires at least 4 control points");
    }

    float scaledT = t * (numPoints - 3);
    int i1 = static_cast<int>(scaledT) + 1;  
    int i0 = i1 - 1;                        
    int i2 = i1 + 1;                        
    int i3 = i1 + 2;                        

    i0 = std::max(i0, 0);
    i1 = std::max(i1, 0);
    i2 = std::min(i2, static_cast<int>(numPoints - 1));
    i3 = std::min(i3, static_cast<int>(numPoints - 1));

    float localT = scaledT - (i1 - 1);

    float p0 = controlPoints[i0];
    float p1 = controlPoints[i1];
    float p2 = controlPoints[i2];
    float p3 = controlPoints[i3];

    float tension = 0.5f;  
    if (i1 <= 1 || i1 >= numPoints - 2) {
        tension = 0.f;
    }

    const float a0 = (-tension * p0 + (2.0f - tension) * p1 +
        (tension - 2.0f) * p2 + tension * p3);
    const float a1 = (2.0f * tension * p0 + (tension - 3.0f) * p1 +
        (3.0f - 2.0f * tension) * p2 - tension * p3);
    const float a2 = (-tension * p0 + tension * p2);
    const float a3 = p1;

    float result = a0 * localT * localT * localT +
        a1 * localT * localT +
        a2 * localT +
        a3;

    registry.transforms.get(pauseScreenEntity).position[which] = result;
}

void PauseState::init() {
    Sprite pauseSprite = renderSystem.loadTexture("pause_screen.png");

    registry.transforms.emplace(pauseScreenEntity, TransformComponent{
        vec3(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.f - 100, 0.0f),
        vec3(pauseSprite.width, pauseSprite.height, 1.0f), 0.f
    });
    registry.sprites.emplace(pauseScreenEntity, pauseSprite);

    MenuItem optionsComponent(renderSystem.loadTexture("menu/options_active.png"), renderSystem.loadTexture("menu/options_inactive.png"),
        renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f + 150.f);
    registry.menuItems.emplace(optionsEntity, optionsComponent);
    MenuItem quitComponent{renderSystem.loadTexture("menu/quit_active.png"), renderSystem.loadTexture("menu/quit_inactive.png"),
    renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f + 150.f + optionsComponent.transformInactive.scale.y * 3};
    registry.menuItems.emplace(quitEntity, quitComponent);

    Sprite escSprite(renderSystem.loadTexture("tutorial/esc_key.PNG"));
    registry.sprites.emplace(esc_key, escSprite);
    registry.transforms.emplace(esc_key, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.94f, 0.f),
        vec3(escSprite.width * 0.3f, escSprite.height * 0.3f, 1.f), 0.f
        });
}

void PauseState::update(float deltaTime) {
    timePassed += deltaTime;

    /*
    (0, 0), (0, height), (width, height), (width 0), (0, 0), middle
    */
    std::vector<float> controlPointsY = {
        100.0f,                              
        window_height_px - 100, 
        window_height_px - 100, 
        100.f, 100.f,
        window_height_px / 2.f,
        window_height_px / 2.f
    };
    std::vector<float> controlPointsX = {
        100.0f,
        100.f,
        window_width_px - 100,
        window_width_px - 100, 100.f,
        window_width_px / 2.f,
        window_width_px / 2.f
    };

    spline(timePassed, controlPointsX, 0);
    spline(timePassed, controlPointsY, 1);
}

void PauseState::cleanup() {
    registry.remove_all_components_of(pauseScreenEntity);
    registry.remove_all_components_of(quitEntity);
    registry.remove_all_components_of(optionsEntity);
}

void PauseState::render() {
    MenuState::render();

    if (registry.sprites.has(pauseScreenEntity) &&
        registry.transforms.has(pauseScreenEntity))
    {
        // Retrieve the Sprite and TransformComponent using the registry
        auto& sprite = registry.sprites.get(pauseScreenEntity);
        auto& transform = registry.transforms.get(pauseScreenEntity);

        // Use the render system to draw the entity
        renderSystem.drawEntity(sprite, transform);
    }
    renderMenuItem(registry.menuItems.get(optionsEntity), mouse_pos);
    renderMenuItem(registry.menuItems.get(quitEntity), mouse_pos);

    renderSystem.drawEntity(registry.sprites.get(esc_key), registry.transforms.get(esc_key));
    renderSystem.renderText("To Resume Game", window_width_px * 0.1f, window_height_px * 0.05f, 0.5f, vec3(1), mat4(1));
}

void PauseState::on_mouse_click(int button, int action, const glm::vec2& position, int mods) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (registry.menuItems.get(optionsEntity).isPointWithin(mouse_pos)) {
            renderSystem.getGameStateManager()->pauseState<OptionsMenu>();
        }
        else if (registry.menuItems.get(quitEntity).isPointWithin(mouse_pos)) {
            renderSystem.getGameStateManager()->resetPausedStates<SplashScreenState>();
        }
    }
}
