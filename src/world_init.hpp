#pragma once
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <components.hpp>

inline std::unordered_map<TEXTURE_ASSET_ID, std::string> createTextureAssetMap() {
    return {
        {TEXTURE_ASSET_ID::PLAYER_WALK_1, "walk_1.png"},
        {TEXTURE_ASSET_ID::PLAYER_WALK_2, "walk_2.png"},
        {TEXTURE_ASSET_ID::PLAYER_WALK_3, "walk_3.png"},
        {TEXTURE_ASSET_ID::PLAYER_WALK_4, "walk_4.png"},
        {TEXTURE_ASSET_ID::PLAYER_JUMP_1, "jump_1.png"},
        {TEXTURE_ASSET_ID::PLAYER_JUMP_2, "jump_2.png"},
        {TEXTURE_ASSET_ID::PLAYER_JUMP_3, "jump_3.png"},
        {TEXTURE_ASSET_ID::PLAYER_JUMP_4, "jump_4.png"},
        {TEXTURE_ASSET_ID::PLAYER_ATTACK_1, "attack_1.png"},
        {TEXTURE_ASSET_ID::PLAYER_ATTACK_2, "attack_2.png"},
        {TEXTURE_ASSET_ID::PLAYER_ATTACK_3, "attack_3.png"},
        {TEXTURE_ASSET_ID::PLAYER_ATTACK_4, "attack_4.png"},
        {TEXTURE_ASSET_ID::PLAYER_ATTACK_5, "attack_5.png"},
        {TEXTURE_ASSET_ID::PLAYER_HIT, "hit.png"},
        {TEXTURE_ASSET_ID::GOOMBA_WALK_ATTACK, "goomba_walk_attack.PNG"},
        {TEXTURE_ASSET_ID::GOOMBA_WALK_HIT, "goomba_walk_hit.PNG"},
        {TEXTURE_ASSET_ID::GOOMBA_WALK_IDLE, "goomba_walk_idle.PNG"},
        {TEXTURE_ASSET_ID::GOOMBA_WALK_NOTICE, "goomba_walk_notice.PNG"},
        {TEXTURE_ASSET_ID::GOOMBA_DEAD, "goomba_dead.PNG"},
        {TEXTURE_ASSET_ID::CEILING_FALL, "ceiling_fall.png"},
        {TEXTURE_ASSET_ID::CEILING_HIT, "ceiling_hit.png"},
        {TEXTURE_ASSET_ID::CEILING_IDLE, "ceiling_idle.png"},
        {TEXTURE_ASSET_ID::CEILING_SPIT, "ceiling_spit.png"},
        {TEXTURE_ASSET_ID::SPLASH_SCREEN, "splash_screen.png"},
        {TEXTURE_ASSET_ID::DEMO_GROUND, "demo_ground.png"},
        {TEXTURE_ASSET_ID::DEMO_GROUND_SMASH, "demo_ground_smash.png"},
        {TEXTURE_ASSET_ID::DEMO_WALL, "demo_wall.png"},
        {TEXTURE_ASSET_ID::DEMO_CEILING, "demo_ceiling.png"},
        {TEXTURE_ASSET_ID::HEART_3, "heart_3.png"},
        {TEXTURE_ASSET_ID::HEART_2, "heart_2.png"},
        {TEXTURE_ASSET_ID::HEART_1, "heart_1.png"},
        {TEXTURE_ASSET_ID::HEART_0, "heart_0.png"},
        {TEXTURE_ASSET_ID::HEART_4_4, "heart_4_4.png"},
        {TEXTURE_ASSET_ID::HEART_4_3, "heart_4_3.png"},
        {TEXTURE_ASSET_ID::HEART_4_2, "heart_4_2.png"},
        {TEXTURE_ASSET_ID::HEART_4_1, "heart_4_1.png"},
        {TEXTURE_ASSET_ID::HEART_4_0, "heart_4_0.png"},
        {TEXTURE_ASSET_ID::HEART_5_5, "heart_5_5.png"},
        {TEXTURE_ASSET_ID::HEART_5_4, "heart_5_4.png"},
        {TEXTURE_ASSET_ID::HEART_5_3, "heart_5_3.png"},
        {TEXTURE_ASSET_ID::HEART_5_2, "heart_5_2.png"},
        {TEXTURE_ASSET_ID::HEART_5_1, "heart_5_1.png"},
        {TEXTURE_ASSET_ID::HEART_5_0, "heart_5_0.png"},
        {TEXTURE_ASSET_ID::CESSPIT_BG, "cesspit_bg.png"},
        {TEXTURE_ASSET_ID::ENTRANCE_BG, "entrance_bg.PNG"},
        {TEXTURE_ASSET_ID::SPACESHIP, "spaceship.PNG"},
        {TEXTURE_ASSET_ID::PIPES, "pipes.PNG"},
        {TEXTURE_ASSET_ID::CESSPIT_BOSS_BG, "cesspit_boss_bg.PNG"},
        {TEXTURE_ASSET_ID::FLAME_THROWER, "flame_thrower.png"},
        {TEXTURE_ASSET_ID::FIREBALL, "Fireball.png"},
        {TEXTURE_ASSET_ID::DOOR, "door.PNG"},
        {TEXTURE_ASSET_ID::BMT_BG, "BMTown_bg.PNG"},
        {TEXTURE_ASSET_ID::CP_WALL, "cp_wall.PNG"},
        {TEXTURE_ASSET_ID::CP_WALL_SHORT, "cp_wall_short.PNG"},
        {TEXTURE_ASSET_ID::BMT_WALL, "bmt_wall.PNG"},
        {TEXTURE_ASSET_ID::BMT_WALL_SHORT, "bmt_wall_short.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_CHARGE, "birdman_charge.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_DEAD, "birdman_dead.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_HIT, "birdman_hit.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_FLY1, "birdman_fly1.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_FLY2, "birdman_fly2.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_FLY3, "birdman_fly3.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_FLY4, "birdman_fly4.PNG"},
        {TEXTURE_ASSET_ID::CHECKPOINT, "checkpoint.png"},
        {TEXTURE_ASSET_ID::LN_THRONE_BG, "LNThrone_bg.PNG"},
        {TEXTURE_ASSET_ID::LN_BG, "LN_bg.PNG"},
        {TEXTURE_ASSET_ID::PELICAN_IDLE, "PelicanIdle.PNG"},
        {TEXTURE_ASSET_ID::PELICAN_TALK, "PelicanTalk.PNG"},
        {TEXTURE_ASSET_ID::EXTRA_HEART, "extra_heart.png"},
        {TEXTURE_ASSET_ID::ARROW, "arrow.PNG"},
        {TEXTURE_ASSET_ID::SWORD_POWERUP, "sword_powerup.png"},
        {TEXTURE_ASSET_ID::GREATBIRD_PLATFORM, "greatbird_platform.PNG"},
        {TEXTURE_ASSET_ID::GREATBIRD_PLATFORM_SMASH, "greatbird_platform_smash.PNG"},
        {TEXTURE_ASSET_ID::BIRDMAN_ELDER, "BirdmanElder.png"},
        {TEXTURE_ASSET_ID::OGRE_KAT_1, "kat1.png"},
        {TEXTURE_ASSET_ID::OGRE_KAT_2, "kat2.png"},
        {TEXTURE_ASSET_ID::DEMO_GROUND_SMASH, "demo_ground_smash.PNG"},
        {TEXTURE_ASSET_ID::SPIKE, "greatbird_spike.PNG"},
        {TEXTURE_ASSET_ID::GREATBIRD_IDLE, "greatbird_idle.PNG"},
        {TEXTURE_ASSET_ID::TEXT_BOX, "tutorial/box.PNG"},
        {TEXTURE_ASSET_ID::PLUS_HEART, "plus_heart.png"},
    };
}

inline void drawLoadingScreen(int count, size_t total) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << (static_cast<float>(count) / total * 100.0f);
    renderSystem.renderText("Loading: " + ss.str() + "%", window_width_px * 0.43f, window_height_px / 2.f,
        1.f, vec3(1), mat4(1));
    glfwSwapBuffers(renderSystem.getWindow());
    glfwPollEvents();
}

inline std::unordered_map<TEXTURE_ASSET_ID, Sprite> loadTextures() {
    auto textures_id_paths = createTextureAssetMap();
    std::unordered_map<TEXTURE_ASSET_ID, std::future<Image>> futures;
    std::atomic<int> count{ 0 };
    for (const auto& pair : textures_id_paths) {
        futures.emplace(pair.first, loadImageData(pair.second, count));
    }

    std::unordered_map<TEXTURE_ASSET_ID, Sprite> results;
    renderSystem.captureScreen();
    for (auto& pair : futures) {
        results.emplace(pair.first, bindTexture(pair.second.get()));
        renderSystem.doGlassBreakTransition(count.load(), futures.size());
    }

    return results;
}
