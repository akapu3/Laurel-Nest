#pragma once

#include <vector>

#include "ecs_registry.hpp"
#include "common.hpp"


class GreatBossAISystem
{
public:
    static Entity init(Entity bossRoom);
    void static step(Entity player, float elapsed_time, Entity current_room);
    void static render();
    void static gb_get_damaged(Entity weapon, bool& isDead, bool& a_pressed, bool& d_pressed, Entity& player);
    void static update_damaged_gb_sprites(float delta_time);


    void static smash_attack(Entity current_room);
    void static spear_attack_stub(float x_pos, Entity current_room);
    void static spear_attack(float x_pos, Entity current_room);

};