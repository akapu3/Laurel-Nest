// internal
#include "ai_system.hpp"
#include "world_system.hpp"
#include "goomba_logic.hpp"

bool gb = false;
bool aim = false;
//bool group_detection = false;
//int second_charge;
//
//float can_charge_timer = 0.5f;
constexpr float swarmGoomba_visualRange = 200.f;


void AISystem::step(Entity player_entity, Entity current_room)
{
    size_t size = registry.patrol_ais.size();
    const float chaseRange = 400.0f;
    const float dashDistance = 1000.0f;


    for (int i = 0; i < size; i++) {
        Patrol_AI& patrol_component = registry.patrol_ais.components[i];
        Entity entity = registry.patrol_ais.entities[i];
        Motion& motion = registry.motions.get(entity);
        Motion& motion_player = registry.motions.get(player_entity);
        

        if (registry.hostiles.has(entity)) {
            if (registry.hostiles.get(entity).type == HostileType::GOOMBA_LAND) {
                float player_distance_x = abs(motion_player.position.x - motion.position.x);
                float player_distance_y = abs(motion_player.position.y - motion.position.y);
                if (!patrol_component.landed && player_distance_y < 100) {
                    patrol_component.landed = true;
                }
                if (patrol_component.landed) {
                    if (!patrol_component.chasing && player_distance_x < chaseRange && player_distance_y <= 100) {
                        patrol_component.chasing = true;
                        Room& r = registry.rooms.get(current_room);
                        if(r.has(entity)){
                            group_behaviour(player_entity);
                        }
                        //                if (motion_player.position.x < motion.position.x) {
                        //                    motion.velocity.x = -5.0f;
                        //                } else {2
                        //                    motion.velocity.x = 5.0f;
                        //                }
                        patrol_component.dashStartX = motion.position.x;
                    }

                    if (patrol_component.chasing) {
                        float dash_traveled_distance = abs(motion.position.x - patrol_component.dashStartX);
                        if (dash_traveled_distance >= dashDistance || player_distance_x >= chaseRange) {
                            patrol_component.chasing = false;
                            if (motion.position.x >= patrol_component.patrolMaxX || motion.position.x <= patrol_component.patrolMinX) {
                                patrol_component.direction *= -1;
                            }
                            motion.velocity.x = patrol_component.direction * 2.0f;
                        }
                    }
                }
            }
            //else if (registry.hostiles.get(entity).type == HostileType::GOOMBA_FLYING) {}
        }
    }
}


void AISystem::ceiling_goomba_attack(Entity& ceilingGoomba, Entity& current_room) {
    Entity spit = Entity();
    // 8 is the roomID for BMT_4
    if (registry.rooms.has(current_room) && registry.rooms.get(current_room).id == ROOM_ID::BMT_4){
        aim = true;
    }

    registry.sprites.emplace(spit, g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_SPIT));

    Motion ceilingGoombaMotion = registry.motions.get(ceilingGoomba);
    Motion goombaMotion;
    goombaMotion.position = ceilingGoombaMotion.position;
    goombaMotion.scale = GOOMBA_CEILING_SPIT_SCALE;

    if(aim){
        Entity player = registry.players.entities[0];
        Motion player_motion = registry.motions.get(player);
        vec2 dif = {abs(player_motion.position.x - goombaMotion.position.x), abs(player_motion.position.y - goombaMotion.position.y)};

        goombaMotion.angle = get_angle(player, ceilingGoomba);
        if(goombaMotion.position.x < registry.motions.get(player).position.x){
            goombaMotion.velocity.x += 5 * dif.x/window_width_px * TPS;
        } else{
            goombaMotion.velocity.x += -5 * dif.x/window_width_px * TPS;
        }
    }
    registry.motions.emplace(spit, std::move(goombaMotion));

    TransformComponent spit_transform;
    registry.transforms.emplace(spit, std::move(spit_transform));

    registry.projectiles.emplace(spit, std::move(Projectile{ ProjectileType::SPIT }));
    registry.gravity.emplace(spit, std::move(Gravity()));
    registry.damages.emplace(spit, std::move(Damage{ 1 }));
    registry.hostiles.emplace(spit, std::move(Hostile()));
    
    // TODO: maybe do this differently
    registry.rooms.get(current_room).insert(spit);
    aim = false;
}

// CREATE A GUARD IN WorldSystem::update()
void AISystem::flying_goomba_step(Entity player, Entity current_room, float elapsed_time) {
    if (registry.rooms.has(current_room)) {
        int count = 0;
        for (Entity entity : registry.rooms.get(current_room).entities) {
            if (registry.hostiles.has(entity) && registry.hostiles.get(entity).type == HostileType::GOOMBA_FLYING) {

                auto& flyingGoomba_Animation = registry.flyingGoombaAnimations.get(entity);
                Motion& flyingGoombaMotion = registry.motions.get(entity);
                Motion& playerMotion = registry.motions.get(player);

                if (registry.goombaFlyingStates.has(entity)) {
                    GoombaFlyingState& fg_state = registry.goombaFlyingStates.get(entity);

                    if (fg_state.current_state != FlyingGoombaState::FLYING_GOOMBA_DEAD) {
                        if (fg_state.animationDone) {
                            fg_state.animationDone = false;

//                            if(group_detection){
//                                can_charge_timer -= elapsed_time;
//                                if(count == second_charge && can_charge_timer < 0){
//                                    fg_state.can_charge = false;
//                                    fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_CHARGE;
//                                    flyingGoomba_Animation.setState(FlyingGoombaState::FLYING_GOOMBA_CHARGE);
//                                    flying_goomba_charge(flyingGoombaMotion, playerMotion);
//                                    group_detection = false;
//                                }
//                            }


                            if (fg_state.current_state == FlyingGoombaState::FLYING_GOOMBA_IDLE) {
                                //The flying goomba can only charge once it has reached its regular y position
                                if (flyingGoombaMotion.position.y > fg_state.idle_flying_altitude) {
                                    flyingGoombaMotion.velocity.y = -TPS * 0.75f;
                                    fg_state.can_charge = false;
                                    fg_state.can_throw_projectile = false;
                                    flyingGoomba_Animation.setState(FlyingGoombaState::FLYING_GOOMBA_IDLE);
                                    fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_IDLE;
                                } else {
                                    flyingGoombaMotion.velocity.y = 0;
                                    fg_state.can_charge = true;
                                    fg_state.can_throw_projectile = true;
                                    if (can_flying_goomba_detect_player(flyingGoombaMotion, playerMotion)) {
                                        if (fg_state.can_charge && fg_state.last_attack == FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE) {
                                            fg_state.can_charge = false;
                                            fg_state.can_throw_projectile = false;
                                            fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_CHARGE;
                                            fg_state.last_attack = FlyingGoombaState::FLYING_GOOMBA_CHARGE;
                                            flyingGoomba_Animation.setState(FlyingGoombaState::FLYING_GOOMBA_CHARGE);
                                            flying_goomba_charge(flyingGoombaMotion, playerMotion);
                                        }
                                        else if (fg_state.can_throw_projectile && fg_state.last_attack == FlyingGoombaState::FLYING_GOOMBA_CHARGE) {
                                            fg_state.can_charge = false;
                                            fg_state.can_throw_projectile = false;
                                            fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE;
                                            fg_state.last_attack = FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE;
                                            flyingGoomba_Animation.setState(FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE);
                                            flyingGoombaMotion.velocity = { signof(flyingGoombaMotion.scale.x)*1,0 };
                                            flying_goomba_throw_spear(flyingGoombaMotion, playerMotion, current_room);
                                        }
                                    }
                                    else {
                                        flyingGoomba_Animation.setState(FlyingGoombaState::FLYING_GOOMBA_IDLE);
                                        fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_IDLE;
                                    }
                                }
                            }
                            else if (fg_state.current_state == FlyingGoombaState::FLYING_GOOMBA_HIT) {
                                if (flyingGoombaMotion.position.y > fg_state.idle_flying_altitude) {
                                    flyingGoombaMotion.velocity.y = -TPS * 0.75f;
                                }
                                else {
                                    flyingGoombaMotion.velocity.y = 0;
                                }
                            }
                            else if (fg_state.current_state == FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE) {
                                //flyingGoombaMotion.velocity = {TPS, 0};
                                fg_state.can_charge = false;
                                fg_state.can_throw_projectile = false;
                                fg_state.current_state = FlyingGoombaState::FLYING_GOOMBA_IDLE;
                                flyingGoomba_Animation.setState(FlyingGoombaState::FLYING_GOOMBA_IDLE);
                            }
                            
                        }
                        if (flyingGoomba_Animation.isAnimationComplete()) {
                            fg_state.animationDone = true;
                        }
                        else {
                            flyingGoomba_Animation.next(elapsed_time);

                        }

                        switch (fg_state.current_state) {
                        case FlyingGoombaState::FLYING_GOOMBA_IDLE:
                            flyingGoombaMotion.scale = GOOMBA_FLYING_FLY_SCALE;
                            break;
                        case FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE:
                            flyingGoombaMotion.scale = GOOMBA_FLYING_FLY_SCALE;
                            break;
                        case FlyingGoombaState::FLYING_GOOMBA_HIT:
                            flyingGoombaMotion.scale = GOOMBA_FLYING_HIT_SCALE;
                            break;
                        case FlyingGoombaState::FLYING_GOOMBA_CHARGE:
                            flyingGoombaMotion.scale = GOOMBA_FLYING_CHARGE_SCALE;
                            break;
                        case FlyingGoombaState::FLYING_GOOMBA_DEAD:
                            flyingGoombaMotion.scale = GOOMBA_FLYING_DEAD_SCALE;
                            break;
                        }
                    }
                }
                count++;
            }
        }
    }
}

bool AISystem::can_flying_goomba_detect_player(Motion flyingGoombaMotion, Motion playerMotion) {
    int detection_range_x = 500;
    bool lower_bound_x;
    bool upper_bound_x;
    if (signof(flyingGoombaMotion.scale.x) == 1) {
        lower_bound_x = flyingGoombaMotion.position.x < playerMotion.position.x;
        upper_bound_x = playerMotion.position.x < flyingGoombaMotion.position.x + detection_range_x;
    } else {
        lower_bound_x = flyingGoombaMotion.position.x - detection_range_x < playerMotion.position.x;
        upper_bound_x = playerMotion.position.x < flyingGoombaMotion.position.x;
    }  
    return lower_bound_x && upper_bound_x;
}

void AISystem::flying_goomba_charge(Motion& flyingGoombaMotion, Motion playerMotion) {

    vec3 v = AISystem::calculate_velocity(flyingGoombaMotion, playerMotion);
    flyingGoombaMotion.velocity = { v.x, v.y };
    flyingGoombaMotion.angle = v.z;
};

void AISystem::flying_goomba_throw_spear(Motion& flyingGoombaMotion, Motion playerMotion, Entity current_room) {
    vec3 v = AISystem::calculate_velocity(flyingGoombaMotion, playerMotion);
    flyingGoombaMotion.angle = v.z;
    AISystem::spawn_flying_goomba_spear(flyingGoombaMotion, v, current_room, 5.f);
}

void AISystem::spawn_flying_goomba_spear(Motion flyingGoombaMotion, vec3 X_Y_Angle, Entity current_room, float gap) {
    Entity spear = Entity();

    // GIVE IT A PROPER SPEAR SPRITE AT SOME POINT
    registry.sprites.emplace(spear, g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_SPIT));

    Motion spearMotion;
    spearMotion.position = flyingGoombaMotion.position;
    spearMotion.velocity = { X_Y_Angle.x / (20 + gap), X_Y_Angle.y };
    spearMotion.scale = GOOMBA_CEILING_SPIT_SCALE*2.f;
    spearMotion.angle = X_Y_Angle.z;
    registry.motions.emplace(spear, std::move(spearMotion));

    TransformComponent spear_transform;
    registry.transforms.emplace(spear, std::move(spear_transform));

    registry.projectiles.emplace(spear, std::move(Projectile{ ProjectileType::SPEAR }));
    registry.damages.emplace(spear, std::move(Damage{ 1 }));
    registry.hostiles.emplace(spear, std::move(Hostile()));

    registry.rooms.get(current_room).insert(spear);
}

void AISystem::group_behaviour(Entity player){
    // Make all goombas chase
    // Make all ceiling goombas target
    size_t size_ground_goomba = registry.patrol_ais.size();
    Motion player_motion = registry.motions.get(player);

    gb = true;
    for(int i = 0; i < size_ground_goomba; i++){
        Entity goomba = registry.patrol_ais.entities[i];
        Motion goomba_motion = registry.motions.get(goomba);
        Patrol_AI component = registry.patrol_ais.get(goomba);
        float pos_x = goomba_motion.position.x - player_motion.position.x;
        if(pos_x > 0){
            if(component.movingRight){
                component.movingRight = false;
            }
        } else {
            if(!component.movingRight){
                component.movingRight = true;
            }
        }

    }
    aim = true;
}

void AISystem::swarm_goomba_step(Entity current_room) {
    if (registry.rooms.has(current_room) && registry.rooms.get(current_room).has_swarm_goombas()) {
        std::set<Entity> swarm_goombas = registry.rooms.get(current_room).swarm_goombas;
        for (Entity swarm_goomba : swarm_goombas) {
            AISystem::swarm_goomba_fly_towards_centre(swarm_goomba, swarm_goombas);
            AISystem::swarm_goomba_avoid_others(swarm_goomba, swarm_goombas);
            AISystem::swarm_goomba_match_velocity(swarm_goomba, swarm_goombas);
            //AISystem::swarm_goomba_limit_speed(swarm_goomba);
            //AISystem::swarm_goomba_keep_witihin_bounds(swarm_goomba);
        }
    }
}

void AISystem::swarm_goomba_keep_witihin_bounds(Entity swarmGoomba) {
    Motion& swarmGoombaMotion = registry.motions.get(swarmGoomba);
    vec2 position = swarmGoombaMotion.position;
    vec2 velocity = swarmGoombaMotion.velocity;

    if (position.x < 0) {
        velocity.x *= -1;
    }
    if (position.x > renderSystem.getWindowWidth()) {
        velocity.x *= -1;
    }
    if (position.y < 10 && !signof(velocity.y)) {
        velocity.y *= -1;
    }
    if (position.y > renderSystem.getWindowHeight()) {
        velocity.x *= -1;
    }
}

void AISystem::swarm_goomba_fly_towards_centre(Entity swarmGoomba, std::set<Entity> swarmGoombas) {
    const float centeringFactor = 1.f;

    float center_x = 0;
    float center_y = 0;
    int numNeighbors = 0;
    Motion& thisMotion = registry.motions.get(swarmGoomba);
    for (Entity otherSwarmGoomba : swarmGoombas) {
        Motion otherMotion = registry.motions.get(otherSwarmGoomba);
        if (calculate_distance(thisMotion, otherMotion) < swarmGoomba_visualRange) {
            center_x += otherMotion.position.x;
            center_y += otherMotion.position.y;
            numNeighbors++;
        }
    }

    if (numNeighbors) {
        center_x /= numNeighbors;
        center_y /= numNeighbors;
        thisMotion.velocity.x += (center_x - thisMotion.position.x) * centeringFactor;
        thisMotion.velocity.y += (center_y - thisMotion.position.y) * centeringFactor;
    }
}

void AISystem::swarm_goomba_avoid_others(Entity swarmGoomba, std::set<Entity> swarmGoombas) {
    const float min_distance = 100.f;
    const float avoidFactor = 1.f;
    float move_x = 0;
    float move_y = 0;
    Motion& thisMotion = registry.motions.get(swarmGoomba);
    for (Entity otherSwarmGoomba : swarmGoombas) {
        Motion otherMotion = registry.motions.get(otherSwarmGoomba);
        if (otherSwarmGoomba != swarmGoomba) {
            if (calculate_distance(thisMotion, otherMotion) < min_distance) {
                move_x += thisMotion.position.x - otherMotion.position.x;
                move_y += thisMotion.position.y - otherMotion.position.y;
            }
        }
    }
    thisMotion.velocity.x += move_x * avoidFactor;
    thisMotion.velocity.y += move_y * avoidFactor;
}

void AISystem::swarm_goomba_match_velocity(Entity swarmGoomba, std::set<Entity> swarmGoombas) {
    const float matching_factor = 1.f;

    float avg_dx = 0;
    float avg_dy = 0;
    float numNeighbors = 0;
    Motion& thisMotion = registry.motions.get(swarmGoomba);
    for (Entity otherSwarmGoomba : swarmGoombas) {
        Motion otherMotion = registry.motions.get(otherSwarmGoomba);
        if (calculate_distance(thisMotion, otherMotion) < swarmGoomba_visualRange) {
            avg_dx += otherMotion.velocity.x;
            avg_dy += otherMotion.velocity.y;
            numNeighbors++;
        }
    }

    if (numNeighbors) {
        avg_dx /= numNeighbors;
        avg_dy /= numNeighbors;

        thisMotion.velocity.x += (avg_dx - thisMotion.velocity.x) * matching_factor;
        thisMotion.velocity.y += (avg_dy - thisMotion.velocity.y) * matching_factor;
    }
}

void AISystem::swarm_goomba_limit_speed(Entity swarmGoomba) {
    const float speed_limit = TPS;
    Motion& sg_motion = registry.motions.get(swarmGoomba);
    const float speed = static_cast<float>(sqrt(pow(sg_motion.velocity.x, 2) + pow(sg_motion.velocity.y, 2)));
    
    if (speed > speed_limit) {
        sg_motion.velocity.x = (sg_motion.velocity.x / speed) * speed_limit;
        sg_motion.velocity.y = (sg_motion.velocity.y / speed) * speed_limit;
    }
}

float AISystem::get_angle(Entity e1, Entity e2){
    Motion m1 = registry.motions.get(e1);
    Motion m2 = registry.motions.get(e2);
    float x = abs(m1.position.x - m2.position.x);
    float y = abs(m1.position.y - m2.position.y);
    return atan(x/y);
}


float AISystem::calculate_distance(Motion motion_1, Motion motion_2) {
    return static_cast<float>(sqrt((pow(motion_1.position.x - motion_2.position.x, 2) + pow(motion_1.position.y - motion_2.position.y, 2))));
}

void AISystem::init_aim(){
    aim = false;
}

vec3 AISystem::calculate_velocity(Motion flyingGoombaMotion, Motion playerMotion) {
    vec2 fg_position = flyingGoombaMotion.position;
    vec2 p_position = playerMotion.position;

    float distance = static_cast<float>(sqrt((pow(fg_position.x - p_position.x, 2) + pow(fg_position.y - p_position.y, 2))));
    float angle = atan2(fg_position.y - p_position.y, fg_position.x - p_position.x) * -1;

    float factor_x = 50;
    float factor_y = 1.75f;
    return { distance * cos(angle) * factor_x * -1, distance * sin(angle) * factor_y , angle };
}