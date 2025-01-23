#include "goomba_logic.hpp"
#include "ai_system.hpp"
#include "world_system.hpp"
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

void GoombaLogic::goomba_ceiling_death(Entity hostile) {
    Sprite& goombaCeilingSprite = registry.sprites.get(hostile);
    goombaCeilingSprite = g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_FALL);

    //Motion& goombaCeilingMotion = registry.motions.get(hostile);
    //goombaCeilingMotion.scale = GOOMBA_CEILING_FALL_SCALE;

    registry.gravity.emplace(hostile, std::move(Gravity()));
    registry.damages.remove(hostile);
    registry.healths.remove(hostile);
    registry.bounding_box.remove(hostile);
    registry.projectileTimers.remove(hostile);
}

// Update the ceiling goomba's falling sprite to its dead sprite
void GoombaLogic::goomba_ceiling_swarm_splat(Entity hostile) {
    Sprite& goombaCeilingSprite = registry.sprites.get(hostile);
    goombaCeilingSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_DEAD);
}

void GoombaLogic::goomba_swarm_death(Entity hostile, Entity current_room) {
    Sprite& goombaCeilingSprite = registry.sprites.get(hostile);
    goombaCeilingSprite = g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_FALL);

    Motion& hostile_motion = registry.motions.get(hostile);
    hostile_motion.velocity = { 0,0 };
    
    registry.gravity.emplace(hostile, std::move(Gravity()));

    Room& room = registry.rooms.get(current_room);
    room.swarm_goombas.erase(hostile);

    registry.damages.remove(hostile);
    registry.patrol_ais.remove(hostile);
    registry.healths.remove(hostile);
}

void GoombaLogic::goomba_land_death(Entity hostile) {
    Sprite& goombaLandSprite = registry.sprites.get(hostile);
    goombaLandSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_DEAD);

    Motion& hostile_motion = registry.motions.get(hostile);
    hostile_motion.velocity = { 0,0 };
    hostile_motion.scale = GOOMBA_LAND_IDLE_SCALE;

    registry.bounding_box.remove(hostile);
    registry.patrol_ais.remove(hostile);
    registry.damages.remove(hostile);
    registry.healths.remove(hostile);
}

void GoombaLogic::goomba_flying_death(Entity hostile) {
    Motion& hostile_motion = registry.motions.get(hostile);
    hostile_motion.velocity = { 0,0 };
    hostile_motion.scale = GOOMBA_FLYING_DEAD_SCALE;

    GoombaFlyingState& fg_state = registry.goombaFlyingStates.get(hostile);
    fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_DEAD;

    auto& flyingGoomba_Animation = registry.flyingGoombaAnimations.get(hostile);
    flyingGoomba_Animation.setState(FlyingGoombaState::FLYING_GOOMBA_DEAD);

    registry.gravity.emplace(hostile, std::move(Gravity()));

    registry.bounding_box.remove(hostile);
    registry.patrol_ais.remove(hostile);
    registry.damages.remove(hostile);
    registry.healths.remove(hostile);
}

void GoombaLogic::goomba_get_damaged(Entity hostile, Entity m_weapon, Entity current_room) {
    if (registry.healths.has(hostile)) {
        Health& hostile_health = registry.healths.get(hostile);
        Damage damage = registry.damages.get(m_weapon);

        hostile_health.current_health -= damage.damage_dealt;

        // If the goomba isnt dead yet, change their current sprite to their hit sprite
        if (hostile_health.current_health > 0) {
            if (!registry.recentDamageTimers.has(hostile)) {
                registry.recentDamageTimers.emplace(hostile, std::move(RecentlyDamagedTimer()));
            }
            Motion& goombaMotion = registry.motions.get(hostile);
            if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_FLYING) {
                registry.flyingGoombaAnimations.get(hostile).setState(FlyingGoombaState::FLYING_GOOMBA_HIT);
                registry.goombaFlyingStates.get(hostile).current_state = FlyingGoombaState::FLYING_GOOMBA_HIT;
                goombaMotion.scale = GOOMBA_FLYING_HIT_SCALE;
            }
            else if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_CEILING) {
                Sprite& goombaSprite = registry.sprites.get(hostile);
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_HIT);
                goombaMotion.scale = GOOMBA_CEILING_HIT_SCALE;
            }
            // Change the landGoombas sprite
            else {
                Sprite& goombaSprite = registry.sprites.get(hostile);
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_WALK_HIT);
                goombaMotion.scale = GOOMBA_LAND_HIT_SCALE;
            }
        }
        else {
            if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_SWARM) {
                goomba_swarm_death(hostile, current_room);
            } else if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_FLYING) {
                goomba_flying_death(hostile);
            } else if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_CEILING) {
                goomba_ceiling_death(hostile);
            } else {
                goomba_land_death(hostile);
            }
        }
    }
}

// If the goomba is currently using its damaged sprite, revert it back to its idle sprite
void GoombaLogic::update_damaged_goomba_sprites(float delta_time) {
    for (Entity entity : registry.recentDamageTimers.entities) {
        if (!registry.hostiles.has(entity)) {
            continue;
        }
        RecentlyDamagedTimer& damaged_timer = registry.recentDamageTimers.get(entity);
        damaged_timer.counter_ms -= delta_time;
        if (damaged_timer.counter_ms <= 0 && registry.healths.has(entity)) {
            Motion& goombaMotion = registry.motions.get(entity);
            if (registry.hostiles.get(entity).type == HostileType::GOOMBA_FLYING) {
                GoombaFlyingState& fg_state = registry.goombaFlyingStates.get(entity);
                auto& fg_animation = registry.flyingGoombaAnimations.get(entity);
                
                fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_IDLE;
                fg_state.animationDone = true;
                fg_animation.setState(FlyingGoombaState::FLYING_GOOMBA_IDLE);
                goombaMotion.scale = GOOMBA_FLYING_FLY_SCALE;
            } 
            // change ceiling goombas sprite
            else if (registry.hostiles.get(entity).type == HostileType::GOOMBA_CEILING) {
                Sprite& goombaSprite = registry.sprites.get(entity);
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_IDLE);
                goombaMotion.scale = GOOMBA_CEILING_IDLE_SCALE;
            }
            else {
                Sprite& goombaSprite = registry.sprites.get(entity);
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_WALK_IDLE);
                goombaMotion.scale = GOOMBA_LAND_IDLE_SCALE;
            }
            registry.recentDamageTimers.remove(entity);
        }
    }
}

// Counts down to when the ceiling goomba can attack again
void GoombaLogic::update_goomba_projectile_timer(float delta_time, Entity current_room) {
    for (Entity entity : registry.projectileTimers.entities) {
        if (registry.rooms.has(current_room) && registry.rooms.get(current_room).has(entity)) {
            ProjectileTimer& projectile_counter = registry.projectileTimers.get(entity);
            projectile_counter.elapsed_time -= delta_time;
            // TODO for Kuter: should this remain here?
            if (projectile_counter.elapsed_time <= 0) {
                AISystem::ceiling_goomba_attack(entity, current_room);
                projectile_counter.elapsed_time = projectile_counter.max_time;
            }
        }
    }
}

void GoombaLogic::goomba_flying_render(Entity hostile) {
    if (registry.flyingGoombaAnimations.has(hostile) &&
        registry.transforms.has(hostile)) {
        auto& animation = registry.flyingGoombaAnimations.get(hostile);
        auto& transform = registry.transforms.get(hostile);
        renderSystem.drawEntity(animation.getCurrentFrame(), transform);
    }
}


