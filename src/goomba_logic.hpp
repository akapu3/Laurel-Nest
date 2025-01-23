#pragma once

#include "render_system.hpp"
#include "ecs_registry.hpp"

constexpr vec2 GOOMBA_LAND_IDLE_SCALE = {116, 109};
constexpr vec2 GOOMBA_LAND_HIT_SCALE = { 149, 178 };
constexpr vec2 GOOMBA_LAND_NOTICE_SCALE = { 149, 178 };
constexpr vec2 GOOMBA_LAND_ATTACK_SCALE = { 149, 178 };
constexpr vec2 GOOMBA_LAND_DEAD_SCALE = { 115, 73 };

constexpr vec2 GOOMBA_CEILING_IDLE_SCALE = { 104, 78 };
constexpr vec2 GOOMBA_CEILING_HIT_SCALE = { 104, 80 };
constexpr vec2 GOOMBA_CEILING_FALL_SCALE = { 141, 92 };
constexpr vec2 GOOMBA_CEILING_SPIT_SCALE = { 38, 38 };
constexpr vec2 GOOMBA_CEILING_DEAD_SCALE = { 115, 73 };

constexpr vec2 GOOMBA_FLYING_CHARGE_SCALE = { 948/3, 461/3 };
constexpr vec2 GOOMBA_FLYING_HIT_SCALE = { 753/3, 683/3 };
constexpr vec2 GOOMBA_FLYING_FLY_SCALE = { 908/3, 817/3 };
constexpr vec2 GOOMBA_FLYING_DEAD_SCALE = { 776/3, 215/3 };

class GoombaLogic
{
public:
    void static spawn_ceiling_goomba_spit(Entity ceilingGoomba, Entity current_room);
    void static goomba_get_damaged(Entity hostile, Entity m_sword, Entity current_room);
    void static goomba_ceiling_death(Entity hostile);
    void static goomba_land_death(Entity hostile);
    void static goomba_swarm_death(Entity hostile, Entity current_room);
    void static update_damaged_goomba_sprites(float delta_time);
    void static update_goomba_projectile_timer(float delta_time, Entity current_room);
    void static goomba_ceiling_swarm_splat(Entity hostile);
    void static goomba_flying_death(Entity hostile);
    void static goomba_flying_render(Entity hostile);

private:

};
