#pragma once

#include <vector>

#include "ecs_registry.hpp"
#include "common.hpp"


// At some point mkae this class abstract
class BossAISystem
{
public:
    static Entity init(Entity bossRoom);
    void static step(Entity player,  float elapsed_time);
    void static render();
    void static chicken_get_damaged(Entity weapon, bool& isDead, bool& a_pressed, bool& d_pressed, Entity& player);
    void static update_damaged_chicken_sprites(float delta_time);
    void static flame_attack(float x_pos);
};