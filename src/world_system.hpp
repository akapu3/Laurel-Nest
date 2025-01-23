#pragma once
#include "common.hpp"
#include <memory>
#include <random>
#include "ecs.hpp"
#include "game_state.hpp"
#include "render_system.hpp"
#include "region_manager.hpp"
//#include "serialize.hpp"

constexpr float player_speed = 1.0f * TPS;
constexpr float player_jump_velocity = 3.7f * TPS; // adjust so you can reach the test platform
constexpr float MAX_COYOTE_TIME = 0.2f;

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box

constexpr float WALKING_BB_WIDTH  = 1.2f * 399.f * 0.2f;
constexpr float WALKING_BB_HEIGHT = 1.2f * 712.f * 0.2f;
constexpr float JUMPING_BB_WIDTH  = 1.2f * 464.f * 0.2f;
constexpr float JUMPING_BB_HEIGHT = 1.2f * 714.f * 0.2f;
constexpr float ATTACKING_BB_WIDTH  = 1.2f * 816.f * 0.2f;
constexpr float ATTACKING_BB_HEIGHT = 1.2f * 714.f * 0.2f;
constexpr float HEARTS_THREE_WIDTH = 0.4f * 964.0f;
constexpr float HEARTS_FOUR_WIDTH = 0.4f * 1065.0f;
constexpr float HEARTS_FIVE_WIDTH = 0.4f * 1234.0f;
constexpr float HEARTS_HEIGHT = 0.4f * 366.0f;
constexpr float FLAME_THROWER_WIDTH = 0.2f * 418.f;
constexpr float FLAME_THROWER_HEIGHT = 0.2f * 272.f;
constexpr float FIREBALL_WIDTH = 0.4f * 422.f;
constexpr float FIREBALL_HEIGHT = 0.4f * 339.f;
constexpr float PLUS_HEART_WIDTH = 0.4f * 168.f;
constexpr float PLUS_HEART_HEIGHT = 0.4f * 150.f;

constexpr int SWORD_CHANNEL = 0;
constexpr int HURT_CHANNEL = 1;
constexpr int SAVE_SOUND_CHANNEL = 2;
constexpr int GUN_CLICK_CHANNEL = 3;
constexpr int HEAL_SOUND_CHANNEL = 4;
constexpr int FLAME_BEAK_SHOOT_CHANNEL = 5;

extern bool Show_FPS;
extern bool isChickenDead;
extern bool isGreatBirdDead;
extern bool start_from_checkpoint;

class RegionManager;

class WorldSystem : public GameState {
public:
	WorldSystem();
	~WorldSystem();

	void init() override;
	void on_key(int key, int scancode, int action, int mods) override;
	void on_mouse_move(const glm::vec2& position) override;
	void on_mouse_click(int button, int action, const glm::vec2& position, int mods) override;
	void update(float deltaTime) override;
	void render() override;
	void cleanup() override;
    void processPlayerInput(int key, int action);
    void useFlameThrower();

	void handle_motions(float deltaTime);
	void handle_connections(float deltaTime);
	void handle_collisions();
	void handle_invinciblity(float deltaTime);
	void handle_plus_heart(float deltaTime);
	void handle_bad_timers(float deltaTime);
	void handle_ai();
	void handle_saving();
	void handle_pelican();
	void handle_elder();
	void handle_ogre();
	void handle_bmt3();
	void handle_hostiles_in_doors();
	void handle_flamethrower(float deltaTime);

	void draw_npc_interact(Entity obj);

	Entity switch_map();

private:
	Entity m_player;
	Entity m_hearts;
	Entity m_sword;
    Entity m_flameThrower;
	Entity current_room;
	Entity next_map;
	Entity m_esc;
	std::unique_ptr<RegionManager> regionManager;

	void player_get_damaged(Entity hostile);
	void player_get_healed();
	void create_heal_up_sprite();

    void update_damaged_player_sprites(float delta_time);

	void init_status_bar();
	void init_three_heart_status_bar();
	void init_four_heart_status_bar();
	void init_five_heart_status_bar();
    void init_flame_thrower();

	void update_status_bar(int num_hearts);
	void upgrade_player_health();

	float ws_delta_time;
    float coyoteTimer = 0.f;
    bool isGrounded = false;
	bool canAttack = true;
    bool isFlameThrowerEquipped = false;
    bool flameThrower_enabled = false;
	bool interrupted_heal = false;

	// saveables
	
	bool is_init = true;
	bool heartPowerUp_0;
	bool heartPowerUp_1;
	bool swordPowerUp_0;


    void updateBoundingBox(Entity entity);

	Mix_Chunk* footstep_sound;
	Mix_Chunk* sword_sound;
	Mix_Chunk* hurt_sound;
	Mix_Chunk* save_sound;
	Mix_Chunk* gun_click_sound;
	Mix_Chunk* heal_sound;
	Mix_Chunk* flame_beak_shoot_sound;
    std::unique_ptr<std::unordered_map<TEXTURE_ASSET_ID, Sprite>> texture_paths;

	// Font stuff
	glm::vec3 font_color;
	glm::mat4 font_trans;

	// NPC stuff
	bool skip_dialogue = false;

	bool pelican_talk = false;
	int pelicanIndex = 0;
	
	bool elder_talk = false;
	int elderIndex = 0;

	bool ogre_talk = false;
	int ogreIndex = 0;
	bool ogre_dance = true;

	bool continue_music = true;
	bool do_save = false;
	bool saved_this_instance;

	void write_to_save_file();

	PhysicsSystem physics; // remove when physics is move to GameState

	bool a_pressed;
	bool d_pressed;
};

extern std::unordered_map<TEXTURE_ASSET_ID, Sprite>* g_texture_paths;

// C++ random number generator
extern std::default_random_engine rng;
extern std::uniform_real_distribution<float> uniform_dist; // number between 0..1
