#pragma once
#include "common.hpp"
#include <utility>
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <memory>
#include <unordered_set>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <boost/optional.hpp>

struct Sprite {
    std::shared_ptr<GLuint> textureID;
    float width;
    float height;

    Sprite(GLuint id, float width, float height) : width(width), height(height) {
        textureID = std::shared_ptr<GLuint>(new GLuint(id), [](GLuint* id) {
            if (glIsTexture(*id)) {
                glDeleteTextures(1, id);
            }
            delete id;
        });
    }
};

enum PlayerState {
    IDLE,
    WALKING,
    ATTACKING,
    JUMPING,
    HIT,
};

enum ChickenState {
    CHICKEN_IDLE,
    CHICKEN_WALK,
    CHICKEN_PECK,
    CHICKEN_FLAME,
    CHICKEN_HIT,
    CHICKEN_DEATH
};

enum GBState {
    GB_IDLE,
    GB_SMASH,
    GB_HIT,
    GB_DEATH
};


enum FlyingGoombaState {
    FLYING_GOOMBA_IDLE,
    FLYING_GOOMBA_CHARGE,
    FLYING_GOOMBA_THROW_PROJECTILE,
    FLYING_GOOMBA_HIT,
    FLYING_GOOMBA_DEAD
};

enum class HostileType {
    GOOMBA_LAND,
    GOOMBA_CEILING,
    GOOMBA_FLYING,
    GOOMBA_SWARM
};

/* Template Animation component for animated sprites
Each supports different animation states, such as walking, jumping, etc.
*/

template<typename State>
struct Animation {
    std::unordered_map<State, std::vector<Sprite>> frames;
    float frameDuration;
    float currentTime;
    size_t currentFrame;
    State currentState;

    Animation(State s) : frameDuration(0.1f), currentTime(0.0f), currentFrame(0), currentState(s) {}

    // add a new animation state
    void addState(State newState, const std::vector<Sprite>&& newSprites) {
        frames[newState] = std::move(newSprites);
    }

    // get the next frame for the current state
    void next(float deltaTime) {
        currentTime += deltaTime;
        if (currentTime >= frameDuration) {
            currentFrame = (currentFrame + 1) % frames[currentState].size();
            currentTime = 0.0f;
        }
    }

    void setState(State newState) {
        if (frames.find(newState) != frames.end()) {
            currentState = newState;
            currentFrame = 0;
            currentTime = 0.0f;
        }
    }

    bool isAnimationComplete() const {
        // Check if the current frame is the last one
        return currentFrame >= frames.at(currentState).size() - 1;
    }

    PlayerState getState() const {
        return currentState;
    }

    const Sprite& getCurrentFrame() const {
        return frames.at(currentState)[currentFrame];
    }
};

struct Motion {
    vec2 position;
    float angle = 0;
    vec2 velocity = { 0, 0 };
    vec2 old_velocity = { 0, 0 };
    vec2 scale = { 0, 0 };
    vec2 acceleration = { 0, 0 };
    boost::optional<vec2> boundingBox;
};

struct TransformComponent {
    glm::vec3 position;
    glm::vec3 scale;
    float rotation;

    TransformComponent& operator=(const Motion& m) {
        position.x = m.position.x;
        position.y = m.position.y;
        position.z = 0;
        scale.x = m.scale.x;
        scale.y = m.scale.y;
        scale.z = 1.f;
        rotation = m.angle;
        return *this;
    }
};

// Player component
struct Player{
    bool attacking = false;
};

// Health component
struct Health
{
    int max_health;
    int current_health;
};

struct Damage
{
    int damage_dealt;
};

struct RecentlyDamagedTimer
{
    float counter_ms = 0.3f;
};

struct SavePoint
{

};

// NPCs
struct Pelican {
    bool isAlive;
};

struct Elder {
};

struct Kat {
};

// spears and yellow ground for the Gb
struct BadObj {
};

struct BadObjTimer {
    float max_time;
    float elapsed_time = 0.f;
    float stall;
    bool isActive = false;
    int damage;
};

// A timer that will be associated to when the player can get damaged again 
struct InvincibilityTimer
{
    float counter_ms = 2000;
};

struct ProjectileTimer
{
    float max_time;
    float elapsed_time;
    
};

struct HealTimer
{
    static constexpr float max_time = 0.25f;
    float counter_ms = 0.25;
};

struct PlusHeartTimer
{
    float counter_ms = 500;

};

enum class ProjectileType {
    FIREBALL,
    SPIT,
    SPEAR
};

struct GoombaFlyingState {
    FlyingGoombaState current_state;
    FlyingGoombaState last_attack;
    bool detectedPlayer = false;
    bool can_charge = false;
    bool can_throw_projectile = false;
    bool animationDone = false;
    float idle_flying_altitude;
};

struct Projectile
{
    ProjectileType type;
};

struct HealthFlask
{
    int max_uses = 3;
    int num_uses = max_uses;
};

struct Weapon
{
    float cooldown = 0.0f;
};




// anything that is hostile to the player
struct Hostile
{
    HostileType type;
};

enum class BoxType {
    HIT_BOX,
    ATTACK_BOX,
    BODY_BOX
};

struct Boss
{
    BoxType boxType = BoxType::BODY_BOX;
    vec2 hitbox;
    vec2 attackbox;
    vec2 bodybox;
};

// struct for attacking
struct Combat
{
    int max_frames = 240;
    int frames = max_frames;
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
    float counter_ms = 3000;
};

struct Collision
{
    // Note, the first object is stored in the ECS container.entities
    Entity other; // the second object involved in the collision
    vec2 direction;
    vec2 overlap;
    Collision(Entity& other, const vec2& direction, const vec2& overlap) : direction(direction), overlap(overlap)
    { this->other = other; };
};

struct ColoredVertex
{
    vec3 position;
    vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
    vec3 position;
    vec2 texcoord;
};

using Edge = std::pair<uint16_t, uint16_t>; // Put the smaller index first!

// Mesh data structure for storing vertex and index buffers
struct Mesh
{
    static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, std::vector<Edge>& out_edges, vec2& out_size);
    vec2 original_size = {1,1};
    std::vector<ColoredVertex> vertices;
    std::vector<uint16_t> vertex_indices;
    std::vector<Edge> edges;
};

struct PlayerMeshes {
    std::unordered_map<PlayerState, Mesh> stateMeshes;
};

// Sets the brightness of the screen
struct ScreenState
{
    float darken_screen_factor = -1;
};

enum class TEXTURE_ASSET_ID {
    PLAYER_WALK_1,                        // walk_1.png
    PLAYER_WALK_2,                        // walk_2.png
    PLAYER_WALK_3,                        // walk_3.png
    PLAYER_WALK_4,                        // walk_4.png
    PLAYER_JUMP_1,                        // jump_1.png
    PLAYER_JUMP_2,                        // jump_2.png
    PLAYER_JUMP_3,                        // jump_3.png
    PLAYER_JUMP_4,                        // jump_4.png
    PLAYER_ATTACK_1,                      // attack_1.png
    PLAYER_ATTACK_2,                      // attack_2.png
    PLAYER_ATTACK_3,                      // attack_3.png
    PLAYER_ATTACK_4,                      // attack_4.png
    PLAYER_ATTACK_5,                      // attack_5.png
    PLAYER_HIT,                           // hit.png
    GOOMBA_WALK_ATTACK,                   // goomba_walk_attack.PNG
    GOOMBA_WALK_HIT,                      // goomba_walk_hit.PNG
    GOOMBA_WALK_IDLE,                     // goomba_walk_idle.PNG
    GOOMBA_WALK_NOTICE,                   // goomba_walk_notice.PNG
    GOOMBA_DEAD,                          // goomba_dead.PNG
    CEILING_FALL,                         // ceiling_fall.png
    CEILING_HIT,                          // ceiling_hit.png
    CEILING_IDLE,                         // ceiling_idle.png
    CEILING_SPIT,                         // ceiling_spit.png 
    SPLASH_SCREEN,                        // splash_screen.png
    DEMO_GROUND,                          // demo_ground.png
    DEMO_GROUND_SMASH,
    DEMO_WALL,                            // demo_wall.png
    DEMO_CEILING,                         // demo_ceiling.png
    HEART_3,                              // heart_3.png
    HEART_2,                              // heart_2.png
    HEART_1,                              // heart_1.png
    HEART_0,                              // heart_0.png
    HEART_4_4,                            // heart_4_4.png
    HEART_4_3,                            // heart_4_3.png
    HEART_4_2,                            // heart_4_2.png
    HEART_4_1,                            // heart_4_1.png
    HEART_4_0,                            // heart_4_0.png
    HEART_5_5,                            // heart_5_5.png
    HEART_5_4,                            // heart_5_4.png
    HEART_5_3,                            // heart_5_3.png
    HEART_5_2,                            // heart_5_2.png
    HEART_5_1,                            // heart_5_1.png
    HEART_5_0,                            // heart_5_0.png
    CESSPIT_BG,                           // cesspit_bg.png
    ENTRANCE_BG,                          // entrance_bg.png
    SPACESHIP,                            // spaceship.png
    PIPES,                                // pipes.png
    CESSPIT_BOSS_BG,                      // cesspit_boss_bg
    FLAME_THROWER,                        // flame_thrower.png
    FIREBALL,                             // Fireball.png
    DOOR,                                 // Door.PNG
    BMT_BG,                               // BMTown_bg.PNG
    CP_WALL,
    CP_WALL_SHORT,
    BMT_WALL,
    BMT_WALL_SHORT,
    BIRDMAN_CHARGE,
    BIRDMAN_DEAD,
    BIRDMAN_HIT,
    BIRDMAN_FLY1,
    BIRDMAN_FLY2,
    BIRDMAN_FLY3,
    BIRDMAN_FLY4,
    CHECKPOINT,
    LN_THRONE_BG,                         // LNThrone_bg.PNG
    LN_BG,                                // LN_bg.PNG
    PELICAN_IDLE,
    PELICAN_TALK,
    EXTRA_HEART,                          // extra_heart.png
    ARROW,                              // arrow.png
    SWORD_POWERUP,
    GREATBIRD_PLATFORM,
    GREATBIRD_PLATFORM_SMASH,
    GREATBIRD_IDLE,
    BIRDMAN_ELDER,
    OGRE_KAT_1,
    OGRE_KAT_2,
    PLUS_HEART,
    SPIKE,
    TEXT_BOX,
    TEXTURE_COUNT                         // Count of all textures
};
constexpr int texture_count = static_cast<int>(TEXTURE_ASSET_ID::TEXTURE_COUNT);

enum class EFFECT_ASSET_ID {
    PLAYER_EFFECT = 0,
    EFFECT_COUNT = PLAYER_EFFECT + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
    PLAYER_IDLE_MESH = 0,
    PLAYER_WALK_MESH = PLAYER_IDLE_MESH + 1,
    PLAYER_JUMP_MESH = PLAYER_WALK_MESH + 1,
    PLAYER_ATTACK_MESH = PLAYER_JUMP_MESH + 1,
    SPRITE = PLAYER_ATTACK_MESH + 1,
    GEOMETRY_COUNT = SPRITE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class ROOM_ID {
    CP_ENTRANCE,
    CP_1,
    CP_2,
    CP_3,
    CP_4,
    CP_BOSS,
    CP_EXIT,
    BMT_ENTRANCE,
    BMT_1,
    BMT_2,
    BMT_3,
    BMT_4,
    BMT_5,
    NPC_1,
    NPC_2,
    LN_1,
    LN_2,
    LN_BOSS
};

struct RenderRequest {
    TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
    GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

struct Gravity {
    float acceleration = 9.8f * TPS;
};


struct Patrol_AI {
    int direction = -1;
    float patrolMinX = 0;
    float patrolMaxX = 0;
    bool movingRight = true;
    bool chasing = false;
    float dashStartX = 0.0f;
    bool landed = false;
};

struct BoundingBox {
    vec2 p1; // Top Left
    vec2 p2; // Bottom Left
    vec3 p3; // Bottom Right
    vec4 p4; // Top Right
    float width = 1;
    float height = 1;
};

struct Environment {
    bool door = false;
};

struct Ground {

};

struct MovingPlatform {
    bool vertical;
    vec2 startPos;
    vec2 endPos;
    bool moving;
};

// Components used for Maps and Rooms
// For performance, consider:
// - having different vectors for different types of components
namespace std {
    template<>
    struct hash<Entity> {
        std::size_t operator()(const Entity& entity) const noexcept {
            return hash<unsigned int>()(entity);
        }
    };
}
struct Room {
    ROOM_ID id;
    bool clear;
    std::set<Entity> entities;
    std::set<Entity> swarm_goombas;
    std::shared_ptr<Mix_Music> music;

    void setMusic(Mix_Music* m) {
        music = std::shared_ptr<Mix_Music>(m, [](Mix_Music* music) {
            if (music != nullptr) {
                Mix_FreeMusic(music);
            }
        });
    }

    void insert(Entity entity) {
        if (!has(entity)) {
            entities.insert(entity);
        }
    }

    bool has(Entity entity) {
        return entities.count(entity) > 0;
    }

    void insert_swarm_goomba(Entity entity) {
        if (swarm_goombas.count(entity) == 0) {
            swarm_goombas.insert(entity);
        }
    }

    bool has_swarm_goombas() {
        return !swarm_goombas.empty();
    }
};

struct Connection {
    Entity door;
    Entity nextRoom;
    vec2 nextSpawn;
    bool switchMap;
};

struct ConnectionList {
    std::vector<Connection> doors;
};    

// font character structure
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
    char character;
};

struct HeartPowerUp {
    int number;
};

struct SwordPowerUp {
};

struct MenuItem {
    Sprite active;
    Sprite inactive;
    TransformComponent transformActive;
    TransformComponent transformInactive;

    MenuItem(Sprite&& activeSprite, Sprite&& inactiveSprite, float x, float y) : active(activeSprite), inactive(inactiveSprite) {
        transformActive = TransformComponent{ vec3(x, y, 0.f), vec3(active.width, active.height, 1.f), 0.f };
        transformInactive = TransformComponent{vec3(x, y, 0.f), vec3(inactive.width, inactive.height, 1.f), 0.f};
    }

    bool isPointWithin(const vec2& pos) const {
        vec2 halfbb = vec2( transformActive.scale.x, transformActive.scale.y ) / 2.f;
        float minx = transformActive.position.x - halfbb.x;
        float maxx = transformActive.position.x + halfbb.x;
        float miny = transformActive.position.y - halfbb.y;
        float maxy = transformActive.position.y + halfbb.y;
        return minx <= pos.x && pos.x <= maxx && miny <= pos.y && pos.y <= maxy;
    }
};

struct Wall{

};

struct EndingTriggers {
    Motion position;
    bool isThroneEnding;

    EndingTriggers(float width, float height, float xPos, float yPos, bool isThroneEnding_) :
    isThroneEnding(isThroneEnding_) {
        if (yPos == 0.f) {
            position.position = glm::vec2(window_width_px * xPos, window_height_px * yPos + 10.f);
        } else {
            position.position = glm::vec2(window_width_px * xPos, window_height_px * yPos);
        }
        width *= 1203;
        height *= 1168;
        position.scale = { width, height };
        position.angle = 0.f;
    }
};
