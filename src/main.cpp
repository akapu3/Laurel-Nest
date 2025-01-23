#include <iostream>

#include   "render_system.hpp"
#include "splash_screen_state.hpp"

int main()
{
    std::cout << "START" << std::endl;

    GameStateManager gameStateManager;
    renderSystem.setGameStateManager(&gameStateManager);

    if (!renderSystem.initOpenGL(window_width_px, window_height_px, "Game"))
    {
        std::cerr << "Failed to initialize RenderSystem." << std::endl;
        return -1;
    }

    gameStateManager.changeState<SplashScreenState>();

    renderSystem.renderLoop();

    return 0;
}
