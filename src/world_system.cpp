#include "world_system.hpp"
#include "pause_state.hpp"
#include "cesspit_map.hpp"
#include "goomba_logic.hpp"
#include "ai_system.hpp"
#include "region_factory.hpp"
#include <game_over_screen.hpp>

#include "birdmantown_map.hpp" //testing
#include "serialize.hpp"
#include "common.hpp"
#include "boss_ai.hpp"
#include "great_boss_ai.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include "cutscene.hpp"

bool Show_FPS = true;
bool isChickenDead = false;
bool isGreatBirdDead = false;
bool start_from_checkpoint = false;
std::unordered_map<TEXTURE_ASSET_ID, Sprite>* g_texture_paths = nullptr;
std::default_random_engine rng = std::default_random_engine(std::random_device( )());
std::uniform_real_distribution<float> uniform_dist(0, 1);

WorldSystem::WorldSystem() : a_pressed(false), d_pressed(false) {
    regionManager = std::make_unique<RegionManager>();

    std::unordered_map<TEXTURE_ASSET_ID, Sprite> temp_texture_paths = loadTextures();

    texture_paths = std::make_unique<std::unordered_map<TEXTURE_ASSET_ID, Sprite>>(std::move(temp_texture_paths));
    g_texture_paths = texture_paths.get();

    font_color = glm::vec3(1.0, 1.0, 1.0);
    font_trans = glm::mat4(1.0f);
    //font_trans = glm::scale(font_trans, glm::vec3(0.5, 0.5, 1.0));
}

WorldSystem::~WorldSystem() {
    g_texture_paths = nullptr;
	WorldSystem::cleanup();
}

void WorldSystem::init() {
    // Create a new entity and register it in the ECSRegistry
    SaveFile sf;
    readFromSaveFile(SAVE_FILE_PATH, sf);
    is_init = false;
    heartPowerUp_0 = sf.heart_power_up_0;
    heartPowerUp_1 = sf.heart_power_up_1;
    swordPowerUp_0 = sf.sword_power_up_0;
    isChickenDead = sf.is_chicken_dead;
    isGreatBirdDead = false;
    start_from_checkpoint = sf.start_from_checkpoint;
    saved_this_instance = false;
    /*isChickenDead = readBoolFromFile(SAVE_FILE_PATH, static_cast<int>(SAVEFILE_LINES::IS_CHICKEN_DEAD), false);
    heartPowerUp_0 = readBoolFromFile(SAVE_FILE_PATH, static_cast<int>(SAVEFILE_LINES::HEART_POWER_UP_0), false);
    heartPowerUp_1 = readBoolFromFile(SAVE_FILE_PATH, static_cast<int>(SAVEFILE_LINES::HEART_POWER_UP_1), false);
    swordPowerUp_0 = readBoolFromFile(SAVE_FILE_PATH, static_cast<int>(SAVEFILE_LINES::HEART_POWER_UP_1), false);
    saved_this_instance = readBoolFromFile(SAVE_FILE_PATH, static_cast<int>(SAVEFILE_LINES::SAVED_THIS_INSTANCE), false);
    start_from_checkpoint = readBoolFromFile(SAVE_FILE_PATH, static_cast<int>(SAVEFILE_LINES::START_FROM_CHECKPOINT),false);*/
    // Player

    // Add the Player component to the player entity    
    registry.players.emplace(m_player, Player());
    physics.setPlayer(m_player);

    // Create and initialize a Motion component for the player
    Motion playerMotion;
    playerMotion.position = glm::vec2(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f);
    playerMotion.velocity = glm::vec2(0, 0);
    playerMotion.scale = { WALKING_BB_WIDTH, WALKING_BB_HEIGHT };
    registry.motions.emplace(m_player, playerMotion);

    // Add the Weapon component to the sword entity
    registry.weapons.emplace(m_sword, Weapon());

    // Create and initialize a damage component for the sword
    Damage swordDamage;
    swordDamage.damage_dealt = sf.sword_damage;
    registry.damages.emplace(m_sword, swordDamage);

    // Create and initialize a Health component for the player
    Health playerHealth;
    playerHealth.current_health = sf.player_current_health;
    playerHealth.max_health = sf.player_max_health;
    registry.healths.emplace(m_player, playerHealth);

    // Create the HealthFlask for the player to heal with
    HealthFlask healthFlask;
    healthFlask.num_uses = sf.health_flask_uses;
    registry.healthFlasks.emplace(m_player, healthFlask);

    // Add gravity to the Player
    registry.gravity.emplace(m_player, Gravity());

    // Add Combat to Player
    registry.combat.emplace(m_player, Combat());

    // Create and initialize the Animation component

    Animation<PlayerState> playerAnimations(IDLE);

    registry.bounding_box.emplace(m_player);
      
    Sprite idleSprite = g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_WALK_3);

    playerAnimations.addState(PlayerState::WALKING, std::vector<Sprite> {
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_WALK_1),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_WALK_2),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_WALK_3),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_WALK_4),
    });
    playerAnimations.addState(PlayerState::IDLE, std::vector<Sprite>{idleSprite});
    playerAnimations.addState(PlayerState::JUMPING, std::vector<Sprite> {
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_JUMP_1),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_JUMP_2),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_JUMP_3),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_JUMP_4),
    });
    playerAnimations.addState(PlayerState::ATTACKING, std::vector<Sprite> {
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_ATTACK_1),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_ATTACK_2),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_ATTACK_3),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_ATTACK_4),
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_ATTACK_5),
    });
    playerAnimations.addState(PlayerState::HIT, std::vector<Sprite> {
            g_texture_paths->at(TEXTURE_ASSET_ID::PLAYER_HIT),
    });
    registry.playerAnimations.emplace(m_player, std::move(playerAnimations));


    // Create and initialize a TransformComponent for the player
    TransformComponent playerTransform;
    playerTransform.position = glm::vec3(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f, 0.0f);
    playerTransform.scale = glm::vec3(WALKING_BB_WIDTH, WALKING_BB_HEIGHT, 1.0f);
    playerTransform.rotation = 0.0f;
    registry.transforms.emplace(m_player, playerTransform);

    // load mesh for player
    renderSystem.loadPlayerMeshes(m_player);

    
    init_status_bar();
    

    init_flame_thrower();
     
    // Initialize the region
    regionManager->init();
    current_room = regionManager->setRegion(makeRegion<Cesspit>);
    //testing bmt
    //current_room = regionManager->setRegion(makeRegion<Birdmantown>);
    next_map = regionManager->setRegion(makeRegion<Birdmantown>);
    physics.setRoom(current_room);


    std::vector<bool> heartPowerUps;
    heartPowerUps.push_back(heartPowerUp_0);
    heartPowerUps.push_back(heartPowerUp_1);
    int x = 0;
    int y = 0;
    while (x < heartPowerUps.size()) {
        if (heartPowerUps[x]) {
            registry.remove_all_components_of(registry.heartPowerUp.entities[y]);
        } else {
            y++;
        }
        x++;
    }

    //TODO: sword
    if (swordPowerUp_0) {
        registry.remove_all_components_of(registry.swordPowerUp.entities[0]);
    }
    

    // esc instruction sprite
    Sprite escSprite(renderSystem.loadTexture("tutorial/esc_key.PNG"));
    registry.sprites.emplace(m_esc, escSprite);
    registry.transforms.emplace(m_esc, TransformComponent{
        vec3(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.94f, 0.f),
        vec3(escSprite.width * 0.3f, escSprite.height * 0.3f, 1.f), 0.f
        });

    Mix_ReserveChannels(4);
    footstep_sound = Mix_LoadWAV(audio_path("footstep.wav").c_str());
    sword_sound = Mix_LoadWAV(audio_path("sword.wav").c_str());
    hurt_sound = Mix_LoadWAV(audio_path("hurt.wav").c_str());
    save_sound = Mix_LoadWAV(audio_path("save.wav").c_str());
    gun_click_sound = Mix_LoadWAV(audio_path("gun_click.wav").c_str());
    heal_sound = Mix_LoadWAV(audio_path("heal.wav").c_str());
    flame_beak_shoot_sound = Mix_LoadWAV(audio_path("flame-beak-shoot.wav").c_str());
    if (!(footstep_sound && sword_sound && hurt_sound && save_sound && gun_click_sound && flame_beak_shoot_sound && heal_sound)) {
        std::cerr << "Failed to load WAV file: " << Mix_GetError() << std::endl;
    }
}

void WorldSystem::update(float deltaTime) {
    deltaTime = min(deltaTime, max_delta_time); // so if there's a lag spike the movement doesn't become so large you phase through walls
    ws_delta_time = deltaTime;
    AISystem::swarm_goomba_step(current_room);
    AISystem::flying_goomba_step(m_player, current_room, deltaTime);
    handle_ai();
    handle_connections(deltaTime);
    handle_motions(deltaTime);
    handle_collisions();
    handle_invinciblity(deltaTime);
    handle_plus_heart(deltaTime);
    handle_bad_timers(deltaTime);
    update_damaged_player_sprites(deltaTime);
    handle_saving();
    handle_hostiles_in_doors();
    handle_flamethrower(deltaTime);
    
    GoombaLogic::update_goomba_projectile_timer(deltaTime, current_room);
    GoombaLogic::update_damaged_goomba_sprites(deltaTime);
    // Only step if the player is in the Chicken boss room
    if (registry.rooms.has(current_room) && registry.rooms.get(current_room).id == ROOM_ID::CP_BOSS) {
        BossAISystem::step(m_player, deltaTime);
        BossAISystem::update_damaged_chicken_sprites(deltaTime);
    }

    if (registry.rooms.has(current_room) && registry.rooms.get(current_room).id == ROOM_ID::LN_BOSS) {
        GreatBossAISystem::step(m_player, deltaTime, current_room);
        GreatBossAISystem::update_damaged_gb_sprites(deltaTime);
    }
    
    // look for specific rooms with restrictions
    if (registry.rooms.has(current_room) && registry.rooms.get(current_room).id == ROOM_ID::BMT_3 && !registry.rooms.get(current_room).clear) {
        handle_bmt3();
    }

    // TODO: make this its own function too??
    //Update bounding boxes for all the entities
    auto & bounding_boxes = registry.bounding_box;
    for(int i = 0; i < bounding_boxes.size(); i++){
        Entity e1 = bounding_boxes.entities[i];
        updateBoundingBox(e1);
    }

    // Update physics, modify gamestate to handle this elsewhere
    physics.step(deltaTime);

    // update music
    if (continue_music && registry.rooms.has(current_room)) {
        Room& r = registry.rooms.get(current_room);
        if (r.id != ROOM_ID::CP_BOSS) {
            std::shared_ptr<Mix_Music> music = r.music;
            Mix_PlayMusic(music.get(), 1);
            continue_music = false;
        }
    }
}

void WorldSystem::handle_connections(float deltaTime) {
    auto& playerMotion = registry.motions.get(m_player);
    if (registry.doorList.has(current_room)) {
        ConnectionList& list = registry.doorList.get(current_room);
        vec2 dir;
        vec2 over;
        for (auto& connection : list.doors) {
            // collision but only if player is in walking state
            auto& a = registry.playerAnimations.get(m_player);
            if (physics.checkForCollision(m_player, connection.door, dir, over) 
                && a.getState() != PlayerState::ATTACKING && playerMotion.velocity != vec2(0.f, 0.f)) {
                // check if in boss room and if boss is dead
                if (registry.rooms.get(current_room).id != ROOM_ID::CP_BOSS 
                    || (registry.rooms.get(current_room).id == ROOM_ID::CP_BOSS && isChickenDead)) {
                    // set next room
                    // check for switching map
                    if (!connection.switchMap) {
                        current_room = connection.nextRoom;
                    }
                    else {
                        Entity next_room = next_map;
                        next_map = current_room;
                        current_room = next_room;
                    }
                    physics.setRoom(current_room);
                    AISystem::init_aim();

                    // set spawn point of player in new room
                    playerMotion.position = connection.nextSpawn;
                    
                    // handle music
                    Room& r = registry.rooms.get(current_room);
                    std::shared_ptr<Mix_Music> music = r.music;
                    if (music != nullptr && r.id == ROOM_ID::CP_BOSS && !isChickenDead) {
                        Mix_PlayMusic(music.get(), 1);
                        continue_music = true;
                    }

                    // No need to check if boss is alive, the game ends when it dies
                    if (music != nullptr && r.id == ROOM_ID::LN_BOSS) {
                        Mix_PlayMusic(music.get(), 1);
                    }
                
                }
            }
        }
    }
}

void WorldSystem::handle_motions(float deltaTime) {
    static PlayerState lastState = PlayerState::WALKING; // Track the player's last state
    // Loop through all entities that have motion components
    for (auto entity : registry.motions.entities) {
        if (registry.transforms.has(entity) && registry.motions.has(entity)) {
            auto& t = registry.transforms.get(entity);
            auto& m = registry.motions.get(entity);

            // Step 1: Apply gravity if not grounded
            if (registry.gravity.has(entity) && ( registry.players.has(entity) || 
                (registry.rooms.has(current_room) && registry.rooms.get(current_room).has(entity)))) {
                auto& g = registry.gravity.get(entity);
                m.velocity.y += g.acceleration * deltaTime;
            }

            // Handle fireball motion
            if (registry.projectiles.has(entity) && registry.projectiles.get(entity).type == ProjectileType::FIREBALL) {
                // Update fireball position
                m.position += m.velocity * deltaTime;

                // Remove fireball if it goes out of screen bounds
                if (m.position.x < 0 || m.position.x > window_width_px) {
                    registry.remove_all_components_of(entity);
                }
            }

            // Step 2: Update position based on velocity
            if (registry.players.has(entity)) {
                if (a_pressed) {
                    m.velocity.x = -player_speed;
                } else if (d_pressed) {
                    m.velocity.x = player_speed;
                }
                // Make the player's position stop once its head reaches the top of the window
                if (m.velocity[0] != 0 && isGrounded) {
                    Mix_PlayChannel(-1, footstep_sound, 0);
                }
                if ((m.position[1] + m.velocity[1] * deltaTime) > 100) {
                    m.position += m.velocity * deltaTime;
                }
                else {
                    // Makes sure the player starts to drop immiediately cuz of gravity
                    m.velocity[1] = 0;
                }
            }
            else {
                if (registry.rooms.has(current_room) && registry.rooms.get(current_room).has(entity)) {
                    //registry.list_all_components_of(entity);
                    m.position += m.velocity * deltaTime;
                }
            }

            // If this is the player, reset canJump before handling collisions
            if (entity == m_player) {
                coyoteTimer -= deltaTime;
                if (coyoteTimer < 0.0f) {
                    coyoteTimer = 0.0f;
                }
            }

            // Step 3: Prevent falling out of the screen for all entities
            if (m.position[1] > window_height_px) {
                m.position[1] = window_height_px;
                m.velocity.y = 0;

                // Only the player can be grounded and jump
                if (entity == m_player) {
                    isGrounded = true;
                    coyoteTimer = MAX_COYOTE_TIME;
                }
            }

            // Guard against the swarm goombas for flying out of the top of the screen
            if (m.position[1] < 25 && registry.hostiles.has(entity) && registry.hostiles.get(entity).type == HostileType::GOOMBA_SWARM) {
                m.position[1] = 25;
                if (m.velocity.y < 0) {
                    m.velocity.y = 200;
                }
            }

            m.position[0] = clamp(m.position[0], 0, window_width_px);

            // Step 4: Flip the texture based on movement direction for all entities
            if (m.velocity[0] < 0) {
                m.scale.x = -std::abs(m.scale.x);
            }
            else if (m.velocity[0] > 0) {
                m.scale.x = std::abs(m.scale.x);
            }

            // moving platform specific, keep platform within bounds
            if (registry.movingPlatform.has(entity) && registry.rooms.get(current_room).has(entity)) {
                auto& mp = registry.movingPlatform.get(entity);
                if (mp.moving) {
                    float mp_xpos = window_width_px;
                    float mp_ypos = window_height_px;
                    // vertical platforms
                    if (mp.vertical) {
                        // start = upper left
                        if (m.position.y < (mp.startPos.y * mp_ypos)) {
                            m.velocity.y = std::abs(m.velocity.y);
                        }
                        // end = lower right
                        else if (m.position.y > (mp.endPos.y * mp_ypos)) {
                            m.velocity.y = -std::abs(m.velocity.y);
                        }
                    }
                    else { // horizontal platforms
                        if (m.position.x < (mp.startPos.x * mp_xpos) || m.position.x >(mp.endPos.x * mp_xpos)) m.velocity *= -1.f;
                    }
                }
            }

            // Player-specific logic
            if (entity == m_player && registry.playerAnimations.has(m_player) && registry.combat.has(m_player)) {
                auto& a = registry.playerAnimations.get(m_player);
                auto& c = registry.combat.get(m_player);

                // Attack handling
                if (c.frames > 0 && !canAttack) {
                    c.frames -= 1;
                }
                else {
                    canAttack = true;
                }

                // Step 6: Handle player state (JUMPING, WALKING, ATTACKING)
                PlayerState currentState = a.getState();
                if (registry.recentDamageTimers.has(m_player)) {
                    currentState = PlayerState::HIT;
                }
                else if (c.frames > 0 && !canAttack) {
                    currentState = PlayerState::ATTACKING;
                }
                else if (m.velocity[0] != 0) {
                    currentState = PlayerState::WALKING;
                }
                else if (!isGrounded) {
                    currentState = PlayerState::JUMPING;
                }
                else {
                    currentState = PlayerState::IDLE;
                }

                // Step 7: Update bounding box size based on state
                switch (currentState) {
                case WALKING:
                case IDLE:
                    m.scale = vec2(WALKING_BB_WIDTH * signof(m.scale.x), WALKING_BB_HEIGHT);
                    break;
                case JUMPING:
                    m.scale = vec2(JUMPING_BB_WIDTH * signof(m.scale.x), JUMPING_BB_HEIGHT);
                    break;
                case ATTACKING:
                    m.scale = vec2(ATTACKING_BB_WIDTH * signof(m.scale.x), ATTACKING_BB_HEIGHT);
                    break;
                case HIT:
                    m.scale = vec2(WALKING_BB_WIDTH * signof(m.scale.x), WALKING_BB_HEIGHT);
                    break;
                }

                // Step 8: Update the player animation state if it has changed
                if (currentState != lastState) {
                    a.setState(currentState);
                    lastState = currentState;
                }
                else {
                    if ((a.currentState == HIT && registry.recentDamageTimers.has(m_player))||
                        (a.currentState == JUMPING && !isGrounded) ||
                        (a.currentState == WALKING && m.velocity[0] != 0) ||
                        (a.currentState == IDLE && m.velocity[0] == 0) ||
                        (a.currentState == ATTACKING)) {
                        a.next(deltaTime);  // Advance the animation frame
                    }
                }

                if (currentState == PlayerState::ATTACKING && a.isAnimationComplete()) {
                    // Reset attack state and set the player back to IDLE or WALKING
                    c.frames = 0;  // Reset attack frames
                    canAttack = true;  // Allow another attack
                    currentState = isGrounded ? PlayerState::IDLE : PlayerState::WALKING;  // Switch back to IDLE or WALKING
                    a.setState(currentState);  // Update animation state
                    registry.players.get(m_player).attacking = false;
                }

                if (currentState == PlayerState::HIT) {
                    m.velocity.x = 0.f;
                }

                if (registry.motions.has(m_flameThrower) && registry.transforms.has(m_flameThrower)) {
                    auto& flameThrowerMotion = registry.motions.get(m_flameThrower);
                    auto& flameThrowerTransform = registry.transforms.get(m_flameThrower);
                    auto& playerMotion = registry.motions.get(m_player);

                    if (playerMotion.scale.x > 0) {
                        flameThrowerMotion.position = playerMotion.position + glm::vec2(std::abs(playerMotion.scale.x) / 2, 0);
                        flameThrowerTransform.rotation = 0.0f;
                    } else {
                        flameThrowerMotion.position = playerMotion.position - glm::vec2(std::abs(playerMotion.scale.x) / 2, 0);
                        flameThrowerTransform.rotation = M_PI;
                    }

                    flameThrowerMotion.velocity = m.velocity;
                }
            }
            t = m;
        }
    }
}

void WorldSystem::handle_collisions() {
    auto& collisionsRegistry = registry.collisions;
    for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
        Entity entity = collisionsRegistry.entities[i];
        Entity entity_other = collisionsRegistry.components[i].other;
        vec2 direction = collisionsRegistry.components[i].direction;
        vec2 overlap = collisionsRegistry.components[i].overlap;

        if (!registry.motions.has(entity) || !registry.motions.has(entity_other)) {
            continue;
        }
        Motion& thisMotion = registry.motions.get(entity);
        Motion& otherMotion = registry.motions.get(entity_other);

        // trigger bad ending when boss dies
        if (registry.players.has(entity) && registry.endingTriggers.has(entity_other) && isGreatBirdDead) {
            renderSystem.getGameStateManager()->pauseState<EndingCutscene<1>>();
        }

        // Check first - Skip handling collision if it's between spit and fireball projectiles
        if (registry.projectiles.has(entity) && registry.projectiles.has(entity_other)) {
            auto projectileType1 = registry.projectiles.get(entity).type;
            auto projectileType2 = registry.projectiles.get(entity_other).type;

            if ((projectileType1 == ProjectileType::SPIT && projectileType2 == ProjectileType::FIREBALL) ||
                (projectileType1 == ProjectileType::FIREBALL && projectileType2 == ProjectileType::SPIT) || 
                (projectileType1 == ProjectileType::SPIT && projectileType2 == ProjectileType::SPEAR) ||
                (projectileType1 == ProjectileType::SPEAR && projectileType2 == ProjectileType::SPIT) ||
                (projectileType1 == ProjectileType::FIREBALL && projectileType2 == ProjectileType::SPEAR) ||
                (projectileType1 == ProjectileType::SPEAR && projectileType2 == ProjectileType::FIREBALL)) {
                continue;
            }
        }

        // gaurd against moving platform and flying goombas because i'm tired
        if (registry.grounds.has(entity_other) && !(registry.movingPlatform.has(entity_other) && registry.flyingGoombaAnimations.has(entity))) {
            if (direction.x != 0 && thisMotion.velocity.x != 0) {
                if (registry.hostiles.has(entity) && registry.hostiles.get(entity).type == HostileType::GOOMBA_SWARM &&
                    registry.healths.has(entity) && registry.patrol_ais.has(entity)) {
                    if (direction.x > 0) {
                        registry.patrol_ais.get(entity).movingRight = false;
                    }
                    else {
                        registry.patrol_ais.get(entity).movingRight = true;
                    }
                }
                else {
                    thisMotion.velocity.x = 0;
                }
                
                if (registry.players.has(entity)) {
                    thisMotion.position.x -= overlap.x;
                }
                else {
                    if (direction.x > 0) {
                    thisMotion.position.x -= overlap.x;
                    }
                    else {
                        thisMotion.position.x += overlap.x;
                    }
                }
            }
            else if (registry.players.has(entity)) {
                if (a_pressed) {
                    thisMotion.velocity.x = -player_speed;
                } else if (d_pressed) {
                    thisMotion.velocity.x = player_speed;
                }
            }
            if (direction.y != 0) {
                if (direction.y > 0 && thisMotion.velocity.y > 0) {
                    // Downward collision
                    thisMotion.position.y -= overlap.y;
                    if (registry.hostiles.has(entity) && registry.hostiles.get(entity).type == HostileType::GOOMBA_SWARM &&
                        registry.healths.has(entity)) {
                        thisMotion.velocity.y *= -1;
                    }
                    else {
                        thisMotion.velocity.y = 0;
                    }
                    if (registry.players.has(entity)) {
                        // Player has collided with the ground
                        isGrounded = true;
                        coyoteTimer = MAX_COYOTE_TIME; // Reset coyote timer
                    }
                }
                else if (direction.y < 0 && thisMotion.velocity.y < 0) {
                    thisMotion.position.y += overlap.y;

                    if (registry.players.has(entity)) {
                        isGrounded = false;
                        thisMotion.velocity.y = std::max(thisMotion.velocity.y, 0.f);
                    }
                }
            }
        }

        if ((registry.bosses.has(entity_other) && registry.chickenAnimations.has(entity) &&
            registry.chickenAnimations.get(entity_other).currentState != CHICKEN_DEATH)) {
            if (direction.x != 0) {
                if (direction.x > 0 && thisMotion.velocity.x > 0) {
                    thisMotion.position.x -= overlap.x;
                }
                else if (direction.x < 0 && thisMotion.velocity.x < 0) {
                    thisMotion.position.x += overlap.x;
                }
            }
        }
        /*if ((registry.bosses.has(entity_other) && registry.gbAnimations.has(entity) &&
            registry.gbAnimations.get(entity_other).currentState != GB_DEATH)) {
            if (direction.x != 0) {
                if (direction.x > 0 && thisMotion.velocity.x > 0) {
                    thisMotion.position.x -= overlap.x;
                }
                else if (direction.x < 0 && thisMotion.velocity.x < 0) {
                    thisMotion.position.x += overlap.x;
                }
            }
        }*/

        // Make the swarm goomba bounce off the ground
        if (registry.hostiles.has(entity) && registry.hostiles.get(entity).type == HostileType::GOOMBA_SWARM
            && registry.grounds.has(entity_other)) {
            if (thisMotion.velocity.y > 0) {
                thisMotion.velocity.y *= -1;
            }
        }

        // Handle the goombas when they collide with a wall
        if(registry.walls.has(entity) && registry.hostiles.has(entity_other)){
            Motion& m_fying_goomba = registry.motions.get(entity_other);
            Motion& m_wall = registry.motions.get(entity);
            if(registry.patrol_ais.has(entity_other)){
                Patrol_AI& patrol = registry.patrol_ais.get(entity_other);
                float change = 0;
                bool movingRight = patrol.movingRight;
                if(movingRight){
                    if (registry.hostiles.get(entity_other).type == HostileType::GOOMBA_FLYING) {
                        change = -200;
                    }
                    else if (registry.hostiles.get(entity_other).type == HostileType::GOOMBA_LAND) {
                        change = -150;
                    }
                    else if (registry.hostiles.get(entity_other).type == HostileType::GOOMBA_SWARM) {
                        change = -75;
                    }

                }else {
                    if (registry.hostiles.get(entity_other).type == HostileType::GOOMBA_FLYING) {
                        change = 200;
                    }
                    else if (registry.hostiles.get(entity_other).type == HostileType::GOOMBA_LAND) {
                        change = 150;
                    }
                    else if (registry.hostiles.get(entity_other).type == HostileType::GOOMBA_SWARM) {
                        change = 75;
                    }
                }
                m_fying_goomba.position.x = m_wall.position.x + change;
                patrol.movingRight = !patrol.movingRight;
            }
        }

        // change the flying goomba's animation when it impacts the ground
        if (registry.hostiles.has(entity) && registry.hostiles.get(entity).type == HostileType::GOOMBA_FLYING 
            && registry.healths.has(entity) && (registry.grounds.has(entity_other) || registry.doors.has(entity_other)) && !registry.movingPlatform.has(entity_other)) {
            auto& goombaFlyingAnimation = registry.flyingGoombaAnimations.get(entity);
            goombaFlyingAnimation.setState(FlyingGoombaState::FLYING_GOOMBA_IDLE);
            GoombaFlyingState& g_state = registry.goombaFlyingStates.get(entity);
            g_state.current_state = FlyingGoombaState::FLYING_GOOMBA_IDLE;
            g_state.animationDone = true;
            Motion& g_motion = registry.motions.get(entity);
            g_motion.scale = GOOMBA_FLYING_FLY_SCALE;
            //g_motion.velocity.x = g_motion.old_velocity.x;
        }

        if (registry.players.has(entity) && registry.damages.has(entity_other)) {
            if (registry.projectiles.has(entity_other) && registry.projectiles.get(entity_other).type == ProjectileType::FIREBALL) {
                continue;
            }
            if (registry.players.get(m_player).attacking) {
                if (registry.bosses.has(entity_other) && !isChickenDead) {
                    Boss& boss = registry.bosses.get(entity_other);
                    boss.boxType = BoxType::HIT_BOX;
                    BossAISystem::chicken_get_damaged(m_sword, isChickenDead, a_pressed, d_pressed, m_player);
                }
                else if (registry.bosses.has(entity_other) && isChickenDead) {
                    Boss& boss = registry.bosses.get(entity_other);
                    boss.boxType = BoxType::HIT_BOX;
                    GreatBossAISystem::gb_get_damaged(m_sword, isGreatBirdDead, a_pressed, d_pressed, m_player);
                } else {
                    GoombaLogic::goomba_get_damaged(entity_other, m_sword, current_room);
                }

                if (!registry.invinciblityTimers.has(m_player)) {
                    InvincibilityTimer& timer = registry.invinciblityTimers.emplace(m_player);
                    timer.counter_ms = 250.f;
                }
                registry.players.get(m_player).attacking = false;
            } else {
                if (registry.bosses.has(entity_other) && !isChickenDead) {
                    Boss& boss = registry.bosses.get(entity_other);
                    boss.boxType = BoxType::ATTACK_BOX;
                }
                if (!registry.invinciblityTimers.has(entity) && registry.damages.get(entity_other).damage_dealt > 0) {
#ifndef GOD_MODE
                    player_get_damaged(entity_other);
#endif
                }
            }
        }

        // Handle damage from fireball if flame thrower is equipped
        if (registry.projectiles.has(entity) && registry.projectiles.get(entity).type == ProjectileType::FIREBALL && isFlameThrowerEquipped) {
            if (registry.hostiles.has(entity_other) && registry.damages.has(entity)) {
                if (registry.healths.has(entity_other) && registry.healths.get(entity_other).current_health > 0) {
                    GoombaLogic::goomba_get_damaged(entity_other, entity, current_room);
                    if (registry.hostiles.get(entity_other).type != HostileType::GOOMBA_SWARM) {
                        registry.remove_all_components_of(entity);  // Remove fireball upon hit unless its a swarm goomba
                    }
                }
            }
            if (registry.bosses.has(entity_other)) {
                Boss& boss = registry.bosses.get(entity_other);
                boss.boxType = BoxType::BODY_BOX;

                if (!isChickenDead) {
                    BossAISystem::chicken_get_damaged(entity, isChickenDead, a_pressed, d_pressed, m_player);
                    registry.remove_all_components_of(entity);
                }
                else // if Great Bird 
                {
                    bool mock;
                    GreatBossAISystem::gb_get_damaged(entity, mock, a_pressed, d_pressed, m_player);
                    registry.remove_all_components_of(entity);
                }
            }
        }

        // Remove the spit attack from ceiling goomba or the spear attack from the birdman after it has hit the player or the ground
        if (registry.projectiles.has(entity) && (registry.projectiles.get(entity).type == ProjectileType::SPIT || registry.projectiles.get(entity).type == ProjectileType::SPEAR)
        && (registry.players.has(entity_other) || registry.grounds.has(entity_other) || registry.doors.has(entity_other))) {
            registry.remove_all_components_of(entity);
        }

        // TODO: Remove the fireball after it has hit the breakable door
        if (registry.projectiles.has(entity) && registry.projectiles.get(entity).type == ProjectileType::FIREBALL
            && registry.grounds.has(entity_other)) {
            if (registry.projectiles.has(entity_other) && registry.projectiles.get(entity).type == ProjectileType::SPIT) {
                continue;
            }
            registry.remove_all_components_of(entity);
        }

        // Once the ceiling or swarm goomba is dead. change its sprite to the dead sprite
        if (registry.hostiles.has(entity) && (registry.hostiles.get(entity).type == HostileType::GOOMBA_CEILING || registry.hostiles.get(entity).type == HostileType::GOOMBA_SWARM) &&
            !registry.healths.has(entity) && registry.grounds.has(entity_other)) {
            GoombaLogic::goomba_ceiling_swarm_splat(entity);
        }

        // handle extra heart powerup, restore all health and remove heart entity
        // TODO: add extra heart life
        if (registry.players.has(entity) && registry.heartPowerUp.has(entity_other)) {
            if (!heartPowerUp_0 || !heartPowerUp_1) {
                upgrade_player_health();
                if (registry.heartPowerUp.get(entity_other).number == 0) {
                    heartPowerUp_0 = true;
                }
                else if (registry.heartPowerUp.get(entity_other).number == 1) {
                    heartPowerUp_1 = true;
                }
                registry.remove_all_components_of(entity_other);
            }
        }

        // TODO: sword
        if (registry.players.has(entity) && registry.swordPowerUp.has(entity_other)) {
            swordPowerUp_0 = true;
            registry.remove_all_components_of(entity_other);
            // increase attack
            Damage& d = registry.damages.get(m_sword);
            d.damage_dealt = 2;
        }
    }
    registry.collisions.clear();
}

void WorldSystem::handle_invinciblity(float deltaTime) {
    std::vector<Entity> to_remove;
    for (auto& e : registry.invinciblityTimers.entities) {
        auto& i = registry.invinciblityTimers.get(e);
        i.counter_ms -= deltaTime * 1000;
        if (i.counter_ms <= 0) {
            to_remove.push_back(e);
        }
    }

    for (auto& e : to_remove) {
        registry.invinciblityTimers.remove(e);
    }
}

void WorldSystem::handle_plus_heart(float deltaTime) {
    std::vector<Entity> to_remove;
    for (auto& e : registry.plusHeartTimers.entities) {
        auto& i = registry.plusHeartTimers.get(e);
        i.counter_ms -= deltaTime * 1000;
        if (i.counter_ms <= 0) {
            to_remove.push_back(e);
        }
    }

    for (auto& e : to_remove) {
        registry.plusHeartTimers.remove(e);
    }
}
          
          
void WorldSystem::handle_bad_timers(float deltaTime) {
    std::vector<Entity> to_remove;
    for (auto& e : registry.badObjTimers.entities) {
        auto& i = registry.badObjTimers.get(e);
        i.elapsed_time += deltaTime * 1000;
        if (i.elapsed_time > i.max_time) {
            to_remove.push_back(e);
        }
    }

    for (auto& e : to_remove) {
        registry.remove_all_components_of(e);
    }
}

// TODO for Kuter: possibly need to add guards here once the goomba is tied to a room
// to not step the ai of enemies in invisible rooms.
void WorldSystem::handle_ai() {
    AISystem::step(m_player, current_room);
    for (auto& e : registry.patrol_ais.entities) {
        auto& p = registry.patrol_ais.get(e);
        if (registry.motions.has(e)) {
            auto& m = registry.motions.get(e);
            if (std::abs(m.position.x - renderSystem.getWindowWidth()) < 10) {
                p.movingRight = false;
            }
            else if (std::abs(m.position.x - 0) < 10) {
                p.movingRight = true;
            }
            if (p.movingRight) {
                if (p.chasing) {
                    m.velocity.x = 3 * TPS;
                }
                else {
                    if (!(registry.hostiles.has(e) && registry.hostiles.get(e).type == HostileType::GOOMBA_FLYING &&
                        registry.goombaFlyingStates.has(e) && registry.goombaFlyingStates.get(e).current_state == FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE)) {
                        m.velocity.x = 1 * TPS;
                    }
                }
            }
            else {
                if (p.chasing) {
                    m.velocity.x = -3 * TPS;
                }
                else {
                    if (!(registry.hostiles.has(e) && registry.hostiles.get(e).type == HostileType::GOOMBA_FLYING &&
                        registry.goombaFlyingStates.has(e) && registry.goombaFlyingStates.get(e).current_state == FlyingGoombaState::FLYING_GOOMBA_THROW_PROJECTILE)) {
                        m.velocity.x = -1 * TPS;
                    }
                }
            }
        }
    }
}

void WorldSystem::handle_saving() {
    if (!registry.rooms.has(current_room)) {
        return;
    }
    Room& room = registry.rooms.get(current_room);
    for (Entity sp : registry.savePoints.entities) {
        if (room.has(sp)) {
            // check if the player is within range of the savepoint
            Motion player_motion = registry.motions.get(m_player);
            Motion save_point_motion = registry.motions.get(sp);
            float save_point_lower_bound_x = save_point_motion.position.x - save_point_motion.scale.x;
            float save_point_upper_bound_x = save_point_motion.position.x + save_point_motion.scale.x;
            float save_point_lower_bound_y = save_point_motion.position.y - save_point_motion.scale.y;
            float save_point_upper_bound_y = save_point_motion.position.y + save_point_motion.scale.y;
            if (save_point_lower_bound_x <= player_motion.position.x && player_motion.position.x < save_point_upper_bound_x
                && save_point_lower_bound_y < player_motion.position.y && player_motion.position.y < save_point_upper_bound_y) {
                if (do_save) {
                    // TODO MAYBE INSERT A SAVE SOUND
                    start_from_checkpoint = true;
                    Mix_PlayChannel(SAVE_SOUND_CHANNEL, save_sound, 0);
                    if (!saved_this_instance) {
                        Health& health = registry.healths.get(m_player);
                        health.current_health = health.max_health;
                        HealthFlask& healthFlask = registry.healthFlasks.get(m_player);
                        healthFlask.num_uses = healthFlask.max_uses;
                        //saved_this_instance = true;
                    }
                   
                    write_to_save_file();
                }
            }
        }
    }
    do_save = false;
}

void WorldSystem::handle_bmt3() {
    bool cleared = true;
    for (auto entity : registry.rooms.get(current_room).entities) {
        // check all enemy goombas are dead, should not be in patrolAI reg
        if (registry.patrol_ais.has(entity) ) {
            cleared = false;
        }
    }
    if (cleared) {
        for (auto entity : registry.rooms.get(current_room).entities) {
            if (registry.movingPlatform.has(entity)) {
                registry.movingPlatform.get(entity).moving = true;
                registry.motions.get(entity).velocity = glm::vec2(0, 100.f);
            }
        }
        registry.rooms.get(current_room).clear = true;
    }
}

// move this elsewhere later
std::string pelicanDialogue[9] = { "You, you! I thought you were a bird!",
"But, no! That is but a mask that you wear.",
"A human in the Realm of Birds?",
"You seek the Laurel Throne, then? To ursurp the Great Bird?",
"Alas, you landed too far, hahaha!",
"And to get to the throne, you must traverse the Old Sewers.",
"But the birds yearn for flesh.",
"Oh, Blazing Master! Have mercy on the poor fool.",
"Hahahahahaha!" };

std::string elderDialogue[9] = {
"So it was you who felled the Blazing Lord.",
"I am Tipp, Birdman Elder, and I seek the next monarch.",
"This was once a prosperous realm,",
"When my master, the Great Bird, was yet mighty.",
"Alas, my Lord has lost his wits, and so has his minions.",
"Strong human, you would make a fine heir.",
"Ascend the Laurel Nest, give my Lord an honorable end.",
"He too would have wished it, in his right mind.",
"Take the throne and make the realm prosper."};

std::string ogreDialogue[9] = {
"Oh! If I didn't see the human face behind your mask...",
"I might have smashed it in. Haha!",
"I am Kat, Ogre Warrior!! Enemy of the Great Bird and his kin.",
"They conquered and burned my homeland.",
"The tyrant, so called, Great Bird, must be put down!",
"You seem to be a strong one. Why not lend me a hand?",
"Ascend thee Laural Nest, overthrow the tyrant,",
"And leave this wicked realm to rot!",
"May the Bird Kind falter and perish!!"
};

// npc stuff // TODO KUTER
void WorldSystem::handle_pelican() {
    // check if plaican is alive?
    if (!registry.rooms.has(current_room)) {
        return;
    }
    Room& room = registry.rooms.get(current_room);
    for (Entity sp : registry.pelican.entities) {
        if (room.has(sp)) {
            // check if the player is within range of the savepoint
            Motion player_motion = registry.motions.get(m_player);
            Motion pelican_point_motion = registry.motions.get(sp);
            float  pelican_point_lower_bound_x = pelican_point_motion.position.x - pelican_point_motion.scale.x;
            float pelican_point_upper_bound_x = pelican_point_motion.position.x + pelican_point_motion.scale.x;
            float pelican_point_lower_bound_y = pelican_point_motion.position.y - pelican_point_motion.scale.y;
            float pelican_point_upper_bound_y = pelican_point_motion.position.y + pelican_point_motion.scale.y;
            if (pelican_point_lower_bound_x <= player_motion.position.x && player_motion.position.x < pelican_point_upper_bound_x
                && pelican_point_lower_bound_y < player_motion.position.y && player_motion.position.y < pelican_point_upper_bound_y) {
                if (pelican_talk) {
                    double position_x = pelican_point_motion.position.x - 500.f;
                    double position_y = pelican_point_motion.position.y + 625.f;
                    Sprite& pelican_sprite = registry.sprites.get(sp);
                    pelican_sprite = g_texture_paths->at(TEXTURE_ASSET_ID::PELICAN_TALK);
                    Sprite boxSprite = g_texture_paths->at(TEXTURE_ASSET_ID::TEXT_BOX);
                    TransformComponent boxTransform;
                    boxTransform.position = glm::vec3(renderSystem.getWindowWidth() - 400.f, renderSystem.getWindowHeight() - 950.f, 0.0);
                    boxTransform.scale = glm::vec3(boxSprite.width / 8.f, boxSprite.height / 15.f, 1.0);
                    boxTransform.rotation = 0.0f;
                    renderSystem.drawEntity(boxSprite, boxTransform);

                    renderSystem.renderText(pelicanDialogue[pelicanIndex], static_cast<float>(position_x), static_cast<float>(position_y),
                        0.5f, font_color, font_trans);
                }
            }
            else {
                Sprite& pelican_sprite = registry.sprites.get(sp);
                pelican_sprite = g_texture_paths->at(TEXTURE_ASSET_ID::PELICAN_IDLE);
                pelican_talk = false;
            }
        }
    }
}

// npc stuff // TODO KUTER
void WorldSystem::handle_elder() {
    // check if plaican is alive?
    if (!registry.rooms.has(current_room)) {
        return;
    }
    Room& room = registry.rooms.get(current_room);
    for (Entity sp : registry.elder.entities) {
        if (room.has(sp)) {
            // check if the player is within range of the savepoint
            Motion player_motion = registry.motions.get(m_player);
            Motion elder_point_motion = registry.motions.get(sp);
            float elder_point_lower_bound_x = elder_point_motion.position.x - elder_point_motion.scale.x;
            float elder_point_upper_bound_x = elder_point_motion.position.x + elder_point_motion.scale.x;
            float elder_point_lower_bound_y = elder_point_motion.position.y - elder_point_motion.scale.y;
            float elder_point_upper_bound_y = elder_point_motion.position.y + elder_point_motion.scale.y;
            if (elder_point_lower_bound_x <= player_motion.position.x && player_motion.position.x < elder_point_upper_bound_x
                && elder_point_lower_bound_y < player_motion.position.y && player_motion.position.y < elder_point_upper_bound_y) {
                if (elder_talk) {
                    double position_x = elder_point_motion.position.x - 500.f;
                    double position_y = elder_point_motion.position.y - 425.f;
                    Sprite& elder_sprite = registry.sprites.get(sp);
                    Sprite boxSprite = g_texture_paths->at(TEXTURE_ASSET_ID::TEXT_BOX);
                    TransformComponent boxTransform;
                    boxTransform.position = glm::vec3(renderSystem.getWindowWidth() - 360.f, renderSystem.getWindowHeight() - 450.f, 0.0);
                    boxTransform.scale = glm::vec3(boxSprite.width / 8.f, boxSprite.height / 15.f, 1.0);
                    boxTransform.rotation = 0.0f;
                    renderSystem.drawEntity(boxSprite, boxTransform);

                    renderSystem.renderText(elderDialogue[elderIndex], static_cast<float>(position_x), static_cast<float>(position_y),
                        0.5f, font_color, font_trans);
                }
            }
            else {
                // Sprite& pelican_sprite = registry.sprites.get(sp);
                // pelican_sprite = g_texture_paths->at(TEXTURE_ASSET_ID::PELICAN_IDLE);
                elder_talk = false;
            }
        }
    }
}

// npc stuff // TODO KUTER
void WorldSystem::handle_ogre() {
    // check if plaican is alive?
    if (!registry.rooms.has(current_room)) {
        return;
    }
    Room& room = registry.rooms.get(current_room);
    for (Entity sp : registry.kat.entities) {
        if (room.has(sp)) {
            // check if the player is within range of the savepoint
            Motion player_motion = registry.motions.get(m_player);
            Motion kat_point_motion = registry.motions.get(sp);
            float kat_point_lower_bound_x = kat_point_motion.position.x - kat_point_motion.scale.x;
            float kat_point_upper_bound_x = kat_point_motion.position.x + kat_point_motion.scale.x;
            float kat_point_lower_bound_y = kat_point_motion.position.y - kat_point_motion.scale.y;
            float kat_point_upper_bound_y = kat_point_motion.position.y + kat_point_motion.scale.y;
            if (kat_point_lower_bound_x <= player_motion.position.x && player_motion.position.x < kat_point_upper_bound_x
                && kat_point_lower_bound_y < player_motion.position.y && player_motion.position.y < kat_point_upper_bound_y) {
                if (ogre_talk) {
                    double position_x = kat_point_motion.position.x - 300.f;
                    double position_y = kat_point_motion.position.y - 280.f;
                    Sprite& kat_sprite = registry.sprites.get(sp);
                    kat_sprite = g_texture_paths->at(TEXTURE_ASSET_ID::OGRE_KAT_2);

                    Sprite boxSprite = g_texture_paths->at(TEXTURE_ASSET_ID::TEXT_BOX);
                    TransformComponent boxTransform;
                    boxTransform.position = glm::vec3(renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() - 550.f, 0.0);
                    boxTransform.scale = glm::vec3(boxSprite.width / 8.f, boxSprite.height / 15.f, 1.0);
                    boxTransform.rotation = 0.0f;
                    renderSystem.drawEntity(boxSprite, boxTransform);
                    renderSystem.renderText(ogreDialogue[ogreIndex], static_cast<float>(position_x), static_cast<float>(position_y),
                        0.5f, font_color, font_trans);
                }
            }
            else {
                Sprite& kat_sprite = registry.sprites.get(sp);
                kat_sprite = g_texture_paths->at(TEXTURE_ASSET_ID::OGRE_KAT_1);
                ogre_talk = false;
            }
        }
    }
}

void WorldSystem::handle_hostiles_in_doors() {
    if (registry.rooms.has(current_room)) {
        for (Entity entity : registry.rooms.get(current_room).entities) {
            if (registry.hostiles.has(entity) && registry.motions.has(entity) && !registry.healths.has(entity)) {
                Motion hostile_motion = registry.motions.get(entity);
                if (hostile_motion.position.y > renderSystem.getWindowHeight() - 10) {
                    registry.remove_all_components_of(entity);
                }
            }
        }
    }
}

void WorldSystem::handle_flamethrower(float deltaTime) {
    if (registry.weapons.has(m_flameThrower)) {
        auto& weapon = registry.weapons.get(m_flameThrower);
        if (weapon.cooldown <= 0) {
            flameThrower_enabled = true;
        }
        else {
            weapon.cooldown -= deltaTime;
        }
    }
}

void WorldSystem::render() {
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the entity if it exists and has the required components
    // also check if it is in the current room
    // Loop twice to ensure the background gets rendered first

    Room& room = registry.rooms.get(current_room);

    // unfortunately we need to have 3 for loops to ensure drawing order, could be optimized I guess - kuter
    for (auto& obj : room.entities) {
        // Draw Objects
        if (registry.envObject.has(obj) && registry.transforms.has(obj) && registry.sprites.has(obj))
        {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            renderSystem.drawEntity(sprite, transform);
        }
    }

    // unfortunately we need to have 3 for loops to ensure drawing order, could be optimized I guess - kuter
    for (auto& obj : room.entities) {
        // Draw Bosses
        if (registry.bosses.has(obj)) {

            if (room.id == ROOM_ID::CP_BOSS) {
                BossAISystem::render();
            }
            else {
                GreatBossAISystem::render();
            }
        }
    }

    // unfortunately we need to have 3 for loops to ensure drawing order, could be optimized I guess - kuter
    for (auto& obj : room.entities) {
        // Draw bad objects
        if (registry.badObjs.has(obj) && registry.badObjTimers.has(obj) && registry.sprites.has(obj))
        {
            BadObjTimer& bt = registry.badObjTimers.get(obj);
            if (bt.elapsed_time > bt.stall) {
                auto& transform = registry.transforms.get(obj);
                auto& sprite = registry.sprites.get(obj);
                renderSystem.drawEntity(sprite, transform);
                if (!bt.isActive) {
                    bt.isActive = true;
                    registry.damages.emplace(obj, std::move(Damage{ bt.damage }));
                }
            }
        }
    }


    for (auto& obj : room.entities) {
        // Draw the savepoints
        if (registry.savePoints.has(obj) && registry.transforms.has(obj) && registry.sprites.has(obj)) {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            renderSystem.drawEntity(sprite, transform);

            // check if the player is within range of the savepoint
            Motion player_motion = registry.motions.get(m_player);
            Motion save_point_motion = registry.motions.get(obj);
            float save_point_lower_bound_x = save_point_motion.position.x - save_point_motion.scale.x;
            float save_point_upper_bound_x = save_point_motion.position.x + save_point_motion.scale.x;
            float save_point_lower_bound_y = save_point_motion.position.y - save_point_motion.scale.y;
            float save_point_upper_bound_y = save_point_motion.position.y + save_point_motion.scale.y;
            if (save_point_lower_bound_x <= player_motion.position.x && player_motion.position.x < save_point_upper_bound_x
                && save_point_lower_bound_y < player_motion.position.y && player_motion.position.y < save_point_upper_bound_y) {
                double position_x = save_point_motion.position.x * 0.95;
                double position_y = (renderSystem.getWindowHeight() -  save_point_motion.position.y) * 1.7;
                renderSystem.renderText("Press V To Save", static_cast<float>(position_x), static_cast<float>(position_y), 0.5f, font_color, font_trans);
            }
        }

        // Draw Pelican
        if (registry.pelican.has(obj) && registry.transforms.has(obj) && registry.sprites.has(obj)) {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            renderSystem.drawEntity(sprite, transform);
            draw_npc_interact(obj);
        }
        // Draw Elder
        if (registry.elder.has(obj) && registry.transforms.has(obj) && registry.sprites.has(obj)) {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            renderSystem.drawEntity(sprite, transform);
            draw_npc_interact(obj);
        }

        // Draw Ogre
        if (registry.kat.has(obj) && registry.transforms.has(obj) && registry.sprites.has(obj)) {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            
            renderSystem.drawEntity(sprite, transform);
            draw_npc_interact(obj);
            
        }

        GoombaLogic::goomba_flying_render(obj);

        // Draw the goombas
        if (registry.hostiles.has(obj) && registry.transforms.has(obj) && registry.sprites.has(obj))
        {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            renderSystem.drawEntity(sprite, transform);
        }

        // draw the plus heart sprite
        if (registry.plusHeartTimers.has(obj) && registry.transforms.has(obj) && registry.sprites.has(obj)) {
            auto& transform = registry.transforms.get(obj);
            auto& sprite = registry.sprites.get(obj);
            renderSystem.drawEntity(sprite, transform);
        }
    }

    // Draw the player entity if it exists and has the required components
    if (registry.playerAnimations.has(m_player) &&
        registry.transforms.has(m_player) )
    {
        auto& animation = registry.playerAnimations.get(m_player);
        auto& transform = registry.transforms.get(m_player);
        renderSystem.drawEntity(animation.getCurrentFrame(), transform);
    }


    // Draw the hearts
    if (registry.transforms.has(m_hearts) && registry.heartSprites.has(m_hearts))
    {
        auto& health = registry.healths.get(m_player);
        update_status_bar(health.current_health);
    }

    HealthFlask& flask = registry.healthFlasks.get(m_player);
    std::string num_uses = std::to_string(flask.num_uses);
    std::string uses_string = "Health Flask uses: " + num_uses;
    renderSystem.renderText(uses_string, static_cast<float>(window_width_px * 0.045), static_cast<float>(window_height_px * 0.80), 0.75f, font_color, font_trans);

    // text for sword power up
    if (swordPowerUp_0) {
        std::string uses_string_sword = "2X Attack Power!";
        renderSystem.renderText(uses_string_sword, static_cast<float>(window_width_px * 0.045), static_cast<float>(window_height_px * 0.77), 0.75f, font_color, font_trans);
    }

    // Draw the flame thrower if the boss is killed
    if (registry.transforms.has(m_flameThrower) && registry.sprites.has(m_flameThrower))
    {
        if (isChickenDead && isFlameThrowerEquipped) {
            auto &flameThrowerTransform = registry.transforms.get(m_flameThrower);
            auto &flameThrowerSprite = registry.sprites.get(m_flameThrower);
            renderSystem.drawEntity(flameThrowerSprite, flameThrowerTransform);
        }
    }

    for (const auto& entity : registry.projectiles.entities) {
        if (room.has(entity)) {
            if (registry.projectiles.get(entity).type == ProjectileType::FIREBALL) {
                if (registry.sprites.has(entity) && registry.transforms.has(entity)) {
                    if (isChickenDead && isFlameThrowerEquipped) {
                        auto& fireballSprite = registry.sprites.get(entity);
                        auto& fireballTransform = registry.transforms.get(entity);
                        renderSystem.drawEntity(fireballSprite, fireballTransform);
                    }
                }
            }
        }
    }

    handle_pelican();
    handle_elder();
    handle_ogre();

    // draw the text that appears when healing
    if (registry.healTimers.has(m_player) && interrupted_heal) {
        Motion player_motion = registry.motions.get(m_player);
        float x_pos = player_motion.position.x - 90.f;
        float y_pos = renderSystem.getWindowHeight() - player_motion.position.y - (WALKING_BB_HEIGHT / 2) - 20;
        renderSystem.renderText("Heal Is Interrupted",x_pos, y_pos, 0.5f, vec3(1), mat4(1));
    }
    else if (registry.healTimers.has(m_player)) {
        Motion player_motion = registry.motions.get(m_player);
        float x_pos = player_motion.position.x - 85.f;
        float y_pos = renderSystem.getWindowHeight() - player_motion.position.y - (WALKING_BB_HEIGHT / 2) - 20;
        std::stringstream ss;
        auto& healTimer = registry.healTimers.get(m_player);
        ss << "Hold to Heal: " << std::fixed << std::setprecision(2)
        << clamp((HealTimer::max_time - healTimer.counter_ms) / healTimer.max_time * 100.f, 0, 100) << "%";
        renderSystem.renderText(ss.str(), x_pos, y_pos, 0.5f, vec3(1), mat4(1));
    }

    // lower left instructions to open pause menue
    renderSystem.drawEntity(registry.sprites.get(m_esc), registry.transforms.get(m_esc));
    renderSystem.renderText("For Pause Menu", window_width_px * 0.1f, window_height_px * 0.05f, 0.5f, vec3(1), mat4(1));

    // PRESS C TO ASCEND THE THRONE text
    if (registry.rooms.has(current_room) && registry.rooms.get(current_room).id == ROOM_ID::LN_BOSS && isGreatBirdDead) {
        vec3 color;
        auto& playerMotion = registry.motions.get(m_player);
        if (playerMotion.position.x >= window_width_px * 0.33f && playerMotion.position.x <= window_width_px * 0.66f) {
            color = vec3(1);
        } else {
            color = vec3(0.5);
        }
        renderSystem.renderText("Press C to Ascend the Throne", window_width_px * 0.4f, window_height_px * 0.5f, 1.f, color, mat4(1));
    }
}

void WorldSystem::processPlayerInput(int key, int action) {
    // Escape key to close the window
    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            renderSystem.getGameStateManager()->pauseState<PauseState>();
            break;
        case GLFW_KEY_A:
            a_pressed = false;
            registry.motions.get(m_player).velocity.x = 0.f;
            break;
        case GLFW_KEY_D:
            d_pressed = false;
            registry.motions.get(m_player).velocity.x = 0.f;
            break;
        case GLFW_KEY_H:
            if (registry.healTimers.has(m_player)) {
                registry.healTimers.remove(m_player);
                registry.playerAnimations.get(m_player).setState(PlayerState::IDLE);
                std::cout << "resetting healing" << "\n";
            }
            interrupted_heal = false;
            break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            // move left/right
        case GLFW_KEY_A:
            if (!registry.healTimers.has(m_player)) {
                a_pressed = true;
            }
            break;
        case GLFW_KEY_D:
            if (!registry.healTimers.has(m_player)) {
                d_pressed = true;
            }
            break;
        case GLFW_KEY_W:
        case GLFW_KEY_SPACE:
            if (!registry.healTimers.has(m_player)) {
                if (registry.motions.has(m_player)) {
                    auto& playerMotion = registry.motions.get(m_player);
                    if (coyoteTimer > 0.f) {  // Ensure the player can only jump if grounded
                        playerMotion.velocity[1] = -player_jump_velocity;  // Apply jump velocity
                        coyoteTimer = 0.f;
                        isGrounded = false;
                    }
                }
            }
            break;
        case GLFW_KEY_S:
            if (!registry.healTimers.has(m_player)) {
                if (registry.motions.has(m_player)) {
                    auto& playerMotion = registry.motions.get(m_player);
                    playerMotion.velocity[1] += player_speed * 2.0f; // Increase downward velocity
                }
            }
            break;
            // Equip / unequip flamethrower
        case GLFW_KEY_E:
            if (isChickenDead) {
                if (!registry.healTimers.has(m_player)) {
                    if (!registry.players.get(m_player).attacking) {
                        isFlameThrowerEquipped = true;
                    }
                }
            }
            break;
        case GLFW_KEY_Q:
            if (isChickenDead) {
                if (!registry.healTimers.has(m_player)) {
                    isFlameThrowerEquipped = false;
                }
            }
            break;
            // show/hide FPS counter
        case GLFW_KEY_F:
            Show_FPS = !Show_FPS;
            break;
        case GLFW_KEY_H:
            interrupted_heal = false;
            return;
            // save
        case GLFW_KEY_V:
            do_save = true;
            break;
            // talk
        case GLFW_KEY_T: {
            Room& room = registry.rooms.get(current_room);

            if (room.id == ROOM_ID::CP_ENTRANCE) {
                if (pelican_talk && pelicanIndex < 8) {
                    pelicanIndex++;
                }
                else if (pelicanIndex >= 8 || pelican_talk == false) {
                    pelican_talk = !pelican_talk;
                }
                break;
            }

            if (room.id == ROOM_ID::NPC_1) {
                if (elder_talk && elderIndex < 8) {
                    elderIndex++;
                }
                else if (elderIndex >= 8 || elder_talk == false) {
                    elder_talk = !elder_talk;
                }
                break;
            }

            if (room.id == ROOM_ID::NPC_2) {
                if (ogre_talk && ogreIndex < 8) {
                    ogreIndex++;
                }
                else if (ogreIndex >= 8 || ogre_talk == false) {
                    ogre_talk = !ogre_talk;
                }
                break;
            }
            break;
        }
            case GLFW_KEY_C:
                // good ending
                    if (registry.rooms.has(current_room) && registry.rooms.get(current_room).id == ROOM_ID::LN_BOSS && isGreatBirdDead) {
                        auto& playerMotion = registry.motions.get(m_player);
                        if (playerMotion.position.x >= window_width_px * 0.33f && playerMotion.position.x <= window_width_px * 0.66f) {
                            renderSystem.getGameStateManager()->pauseState<EndingCutscene<2>>();
                        }
                    }
            break;
        }
        interrupted_heal = true;
    }
    else if (action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_H:
            if (registry.playerAnimations.has(m_player) && registry.playerAnimations.get(m_player).getState() == PlayerState::IDLE) {
                if (registry.healths.has(m_player) && registry.healths.get(m_player).current_health != registry.healths.get(m_player).max_health
                    && registry.healthFlasks.has(m_player) && registry.healthFlasks.get(m_player).num_uses > 0) {
                    if (!registry.healTimers.has(m_player)) {
                        registry.healTimers.emplace(m_player, HealTimer());
                    }
                    else if (registry.healTimers.get(m_player).counter_ms <= 0) {
                        player_get_healed();
                        registry.healTimers.remove(m_player);
                    }
                    else {
                        HealTimer& h_timer = registry.healTimers.get(m_player);
                        h_timer.counter_ms -= ws_delta_time;
                        interrupted_heal = false;
                    }
                }
            }
            break;
        default:
            interrupted_heal = true;
        }
    }
}

void WorldSystem::useFlameThrower() {
    Mix_PlayChannel(FLAME_BEAK_SHOOT_CHANNEL, flame_beak_shoot_sound, 0);
    auto& weapon = registry.weapons.get(m_flameThrower);

    Entity m_fireball = Entity();

    Sprite fireballSprite = g_texture_paths->at(TEXTURE_ASSET_ID::FIREBALL);

    registry.sprites.emplace(m_fireball, std::move(fireballSprite));

   Motion playerMotion = registry.motions.get(m_player);
   Motion fireballMotion;
   TransformComponent fireballTransform;

   float offsetDistance = 100.f;
   if (playerMotion.scale.x > 0) {
       fireballMotion.position = playerMotion.position + glm::vec2(offsetDistance, 0);
       fireballMotion.velocity = glm::vec2(2.f * TPS, 0.f);
       fireballMotion.scale = glm::vec2(FIREBALL_WIDTH, FIREBALL_HEIGHT);
   }
   else {
       fireballMotion.position = playerMotion.position - glm::vec2(offsetDistance, 0);
       fireballMotion.velocity = glm::vec2(-2.f * TPS, 0.f);
       fireballMotion.scale = glm::vec2(-FIREBALL_WIDTH, FIREBALL_HEIGHT);
   }

   registry.motions.emplace(m_fireball, fireballMotion);
   fireballTransform = fireballMotion;
   registry.transforms.emplace(m_fireball, std::move(fireballTransform));

   Damage fireballDamage;
   fireballDamage.damage_dealt = 3;
   registry.damages.emplace(m_fireball, fireballDamage);

   BoundingBox fireballBB;
   fireballBB.width = FIREBALL_WIDTH;
   fireballBB.height = FIREBALL_HEIGHT;
   registry.bounding_box.emplace(m_fireball, std::move(fireballBB));

   // Set fireball to expire at window edges
   registry.projectiles.emplace(m_fireball, std::move(Projectile{ProjectileType::FIREBALL}));

   Room& this_room = registry.rooms.get(current_room);
   this_room.entities.insert(m_fireball);

   weapon.cooldown = 3.0f;
   flameThrower_enabled = false;
}

void WorldSystem::on_key(int key, int, int action, int) {
    processPlayerInput(key, action);
}

void WorldSystem::on_mouse_move(const glm::vec2&) {
}

void WorldSystem::on_mouse_click(int button, int action, const glm::vec2&, int) {
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (registry.combat.has(m_player)) {
            if (!registry.healTimers.has(m_player)) {
                if (!isFlameThrowerEquipped) {
                    if (canAttack) {  // Ensure the player can attack
                        // make a call to bounding boxes here
                        std::cout << "is attacking" << std::endl;
                        Mix_PlayChannel(SWORD_CHANNEL, sword_sound, 0);
                        canAttack = false;  // Prevent further attacks for a time
                        auto& c = registry.combat.get(m_player);
                        c.frames = c.max_frames;
                        registry.players.get(m_player).attacking = true;
                    }
                }
                else if (flameThrower_enabled) {
                    useFlameThrower();
                }
                else {
                    Mix_PlayChannel(GUN_CLICK_CHANNEL, gun_click_sound, 0);
                }
            }
        }
    }
}

void WorldSystem::cleanup() {
    // Remove all components of the player entity from the registry
    Mix_HaltMusic();
    if (footstep_sound != nullptr) {
        Mix_FreeChunk(footstep_sound);
        footstep_sound = nullptr;
    }
    if (sword_sound != nullptr) {
        Mix_FreeChunk(sword_sound);
        sword_sound = nullptr;
    }
    if (hurt_sound != nullptr) {
        Mix_FreeChunk(hurt_sound);
        hurt_sound = nullptr;
    }
    if (save_sound != nullptr) {
        Mix_FreeChunk(save_sound);
        save_sound = nullptr;
    }
    if (gun_click_sound != nullptr) {
        Mix_FreeChunk(gun_click_sound);
        gun_click_sound = nullptr;
    }
    if (heal_sound != nullptr) {
        Mix_FreeChunk(heal_sound);
        heal_sound = nullptr;
    }
    if (flame_beak_shoot_sound != nullptr) {
        Mix_FreeChunk(flame_beak_shoot_sound);
        flame_beak_shoot_sound = nullptr;
    }
    registry.clear_all_components();
}

// TODO: move the functions below to their own classes

void WorldSystem::update_damaged_player_sprites(float delta_time) {
    for (Entity entity : registry.recentDamageTimers.entities) {
        if (!registry.players.has(entity) || !registry.playerAnimations.has(entity) || !registry.motions.has(entity)) {
            continue;
        }
        RecentlyDamagedTimer& damaged_timer = registry.recentDamageTimers.get(entity);
        damaged_timer.counter_ms -= delta_time;
        if (damaged_timer.counter_ms <= 0) {
            registry.playerAnimations.get(entity).setState(PlayerState::IDLE);
            registry.motions.get(entity).scale = { WALKING_BB_WIDTH, WALKING_BB_HEIGHT };
            registry.recentDamageTimers.remove(entity);
        }
    }
}

void WorldSystem::player_get_damaged(Entity hostile) {
    Mix_PlayChannel(HURT_CHANNEL, hurt_sound, 0);
    Health& player_health = registry.healths.get(m_player);
    Damage hostile_damage = registry.damages.get(hostile);
    // Make sure to give the player i-frames so that they dont just die from walking into a goomba

    if (!registry.invinciblityTimers.has(m_player)) {
        InvincibilityTimer& timer = registry.invinciblityTimers.emplace(m_player);
    }

    if (player_health.current_health > 0) {
        player_health.current_health -= hostile_damage.damage_dealt;
        update_status_bar(player_health.current_health);
        if (!registry.recentDamageTimers.has(m_player)) {
            registry.recentDamageTimers.emplace(m_player, RecentlyDamagedTimer());
        }
        if (player_health.current_health <= 0) {
            renderSystem.getGameStateManager()->changeState<GameOverScreen>();
        }
    }
}

void WorldSystem::player_get_healed() {
    Health& player_health = registry.healths.get(m_player);
    HealthFlask& health_flask = registry.healthFlasks.get(m_player);

    if (health_flask.num_uses > 0 && player_health.max_health > player_health.current_health) {
        player_health.current_health++;
        health_flask.num_uses--;
        registry.playerAnimations.get(m_player).setState(PlayerState::IDLE);
        Mix_PlayChannel(HEAL_SOUND_CHANNEL, heal_sound, 0);
        create_heal_up_sprite();
        update_status_bar(player_health.current_health);
        printf("You have %d uses of your health flask left \n", health_flask.num_uses);
    }
    else if (player_health.max_health == player_health.current_health){
        printf("You have full health \n");
    }
    else {
        printf("You have no more uses of your health flask \n");
    }
}

void WorldSystem::create_heal_up_sprite() {
    Entity heal_up = Entity();

    registry.sprites.emplace(heal_up, g_texture_paths->at(TEXTURE_ASSET_ID::PLUS_HEART));

    Motion player_motion = registry.motions.get(m_player);
    Motion hp_motion;
    hp_motion.position = vec2(player_motion.position.x , player_motion.position.y - 125.f);
    hp_motion.scale = {PLUS_HEART_WIDTH ,PLUS_HEART_HEIGHT};
    hp_motion.velocity.y = -100.f;
    registry.motions.emplace(heal_up, std::move(hp_motion));

    TransformComponent hp_transform;
    registry.transforms.emplace(heal_up, std::move(hp_transform));

    registry.plusHeartTimers.emplace(heal_up, std::move(PlusHeartTimer()));

    if (registry.rooms.has(current_room)) {
        registry.rooms.get(current_room).insert(heal_up);
    }
}



void WorldSystem::init_status_bar() {
    // Create and initialize the Heart sprites
    Health player_health = registry.healths.get(m_player);
    if (player_health.max_health == 5) {
        init_five_heart_status_bar();
    } else if (player_health.max_health == 4) {
        init_four_heart_status_bar();
    } else {
        init_three_heart_status_bar();
    }
}

void WorldSystem::init_three_heart_status_bar() {
    registry.heartSprites.emplace(m_hearts, std::vector<Sprite> {
        g_texture_paths->at(TEXTURE_ASSET_ID::HEART_0),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_1),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_2),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_3)
    });

    // Create and initialize the a Transform component for the 3 Heart sprites
    TransformComponent heartSpriteTransform;
    heartSpriteTransform.position = glm::vec3(250.0f, 120.0f, 0.0);
    heartSpriteTransform.scale = glm::vec3(HEARTS_THREE_WIDTH, HEARTS_HEIGHT, 1.0);
    heartSpriteTransform.rotation = 0.0f;
    registry.transforms.emplace(m_hearts, std::move(heartSpriteTransform));
}

void WorldSystem::init_four_heart_status_bar() {
    // Add new heart sprites (HEART_4_0 to HEART_4_4)
    if (registry.heartSprites.has(m_hearts)) {
        registry.heartSprites.remove(m_hearts);
    }
    if (registry.transforms.has(m_hearts)) {
        registry.transforms.remove(m_hearts);
    }

    registry.heartSprites.emplace(m_hearts, std::vector<Sprite> {
        g_texture_paths->at(TEXTURE_ASSET_ID::HEART_4_0),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_4_1),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_4_2),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_4_3),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_4_4)
    });


    // Create and initialize a Transform component for the new 4 Heart sprites
    TransformComponent heartSpriteTransform;
    heartSpriteTransform.position = glm::vec3(250.0f, 120.0f, 0.0); // Position remains unchanged
    heartSpriteTransform.scale = glm::vec3(HEARTS_FOUR_WIDTH, HEARTS_HEIGHT, 1.0); // Updated width used
    heartSpriteTransform.rotation = 0.0f;
    registry.transforms.emplace(m_hearts, std::move(heartSpriteTransform));
}

void WorldSystem::init_five_heart_status_bar() {
    if (registry.heartSprites.has(m_hearts)) {
        registry.heartSprites.remove(m_hearts);
    }
    if (registry.transforms.has(m_hearts)) {
        registry.transforms.remove(m_hearts);
    }
    registry.heartSprites.emplace(m_hearts, std::vector<Sprite> {
        g_texture_paths->at(TEXTURE_ASSET_ID::HEART_5_0),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_5_1),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_5_2),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_5_3),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_5_4),
            g_texture_paths->at(TEXTURE_ASSET_ID::HEART_5_5)
    });

    // Create and initialize a Transform component for the new 4 Heart sprites
    TransformComponent heartSpriteTransform;
    heartSpriteTransform.position = glm::vec3(250.0f, 120.0f, 0.0); // Position remains unchanged
    heartSpriteTransform.scale = glm::vec3(HEARTS_FIVE_WIDTH, HEARTS_HEIGHT, 1.0); // Updated width used
    heartSpriteTransform.rotation = 0.0f;
    registry.transforms.emplace(m_hearts, std::move(heartSpriteTransform));
}

void WorldSystem::update_status_bar(int num_hearts) {
    auto& transform = registry.transforms.get(m_hearts);
    auto& heartSprites = registry.heartSprites.get(m_hearts);
    num_hearts = clamp(num_hearts, 0, static_cast<int>(heartSprites.size()));
    Sprite heartSprite = heartSprites[num_hearts];
    renderSystem.drawEntity(heartSprite, transform);
}

void WorldSystem::init_flame_thrower() {
    Sprite flameThrowerSprite = g_texture_paths->at(TEXTURE_ASSET_ID::FLAME_THROWER);
    registry.sprites.emplace(m_flameThrower, std::move(flameThrowerSprite));

    TransformComponent flameThrowerSpriteTransform;
    auto& motion = registry.motions.get(m_player);
    if (motion.scale.x > 0) {
        flameThrowerSpriteTransform.position = glm::vec3(motion.position.x + motion.scale.x / 2, motion.position.y, 0.0);
        flameThrowerSpriteTransform.scale = glm::vec3(FLAME_THROWER_WIDTH, FLAME_THROWER_HEIGHT, 1.0);
        flameThrowerSpriteTransform.rotation = 0.0f;
    } else {
        flameThrowerSpriteTransform.position = glm::vec3(motion.position.x - motion.scale.x / 2, motion.position.y, 0.0);
        flameThrowerSpriteTransform.scale = glm::vec3(FLAME_THROWER_WIDTH, FLAME_THROWER_HEIGHT, 1.0);
        flameThrowerSpriteTransform.rotation = M_PI;
    }
    registry.transforms.emplace(m_flameThrower, flameThrowerSpriteTransform);

    Motion flameThrowerMotion;
    flameThrowerMotion.position = flameThrowerSpriteTransform.position;
    flameThrowerMotion.velocity = motion.velocity;
    flameThrowerMotion.scale = glm::vec2(FLAME_THROWER_WIDTH, FLAME_THROWER_HEIGHT);
    registry.motions.emplace(m_flameThrower, std::move(flameThrowerMotion));

    registry.weapons.emplace(m_flameThrower, Weapon());
}

void WorldSystem::updateBoundingBox(Entity e1) {
    Motion& player_motion = registry.motions.get(e1);
    float box_height = player_motion.scale.y * registry.bounding_box.get(e1).height;
    float y_value_min = player_motion.position.y - box_height/2;
    float y_value_max = player_motion.position.y + box_height/2;
    float box_width = player_motion.scale.x * registry.bounding_box.get(e1).width;
    float x_value_min = player_motion.position.x - box_width/2;
    float x_value_max = player_motion.position.x + box_width/2;
    BoundingBox bounding_box = registry.bounding_box.get(e1);

    //Top Left
    bounding_box.p1.x = x_value_min;
    bounding_box.p1.y = y_value_max;

    //Bottom Left
    bounding_box.p2.x = x_value_min;
    bounding_box.p2.y = y_value_min;

    //Bottom Right
    bounding_box.p3.x = x_value_max;
    bounding_box.p3.y = y_value_min;

    //Top Right
    bounding_box.p4.x = x_value_max;
    bounding_box.p4.y = y_value_max;

}

void WorldSystem::upgrade_player_health()  {
    Health& player_health = registry.healths.get(m_player);
    player_health.max_health ++;
    player_health.current_health = player_health.max_health;
    HealthFlask& health_flask = registry.healthFlasks.get(m_player);
    health_flask.num_uses = health_flask.max_uses;
    if (player_health.max_health == 5) {
        init_five_heart_status_bar();
    } else if (player_health.max_health == 4) {
        init_four_heart_status_bar();
    }
}

void WorldSystem::write_to_save_file() {
    std::fstream saveFile;
    saveFile.open(SAVE_FILE_PATH, std::ios::out); // writing
    if (saveFile.is_open()) {
        Health player_health = registry.healths.get(m_player);
        Damage player_damage = registry.damages.get(m_sword);
        HealthFlask health_flask = registry.healthFlasks.get(m_player);

        saveFile << BoolToString(is_init) << "\n";

        saveFile << player_health.max_health << "\n";

        saveFile << player_health.current_health << "\n";

        saveFile << health_flask.num_uses << "\n";

        saveFile << player_damage.damage_dealt << "\n";

        saveFile << BoolToString(heartPowerUp_0) << "\n";

        saveFile << BoolToString(heartPowerUp_1) << "\n";

        saveFile << BoolToString(swordPowerUp_0) << "\n";

        saveFile << BoolToString(isChickenDead) << "\n";

        saveFile << BoolToString(start_from_checkpoint) << "\n";

        saveFile << BoolToString(saved_this_instance) << "\n";

        saveFile.close();
    }
    else {
        std::cout << "Couldnt write to save file \n";
    }
}

void WorldSystem::draw_npc_interact(Entity obj) {
    // check if the player is within range of the savepoint
    Motion player_motion = registry.motions.get(m_player);
    Motion npc_point_motion = registry.motions.get(obj);
    float npc_point_lower_bound_x = npc_point_motion.position.x - npc_point_motion.scale.x;
    float npc_point_upper_bound_x = npc_point_motion.position.x + npc_point_motion.scale.x;
    float npc_point_lower_bound_y = npc_point_motion.position.y - npc_point_motion.scale.y;
    float npc_point_upper_bound_y = npc_point_motion.position.y + npc_point_motion.scale.y;
    if (npc_point_lower_bound_x <= player_motion.position.x && player_motion.position.x < npc_point_upper_bound_x
        && npc_point_lower_bound_y < player_motion.position.y && player_motion.position.y < npc_point_upper_bound_y) {
        double position_x = npc_point_motion.position.x - 75.f;
        double position_y = renderSystem.getWindowHeight() - npc_point_motion.position.y - 140.f;
        renderSystem.renderText("Press T To Talk", static_cast<float>(position_x), static_cast<float>(position_y),
            0.5f, font_color, font_trans);
    }
}
