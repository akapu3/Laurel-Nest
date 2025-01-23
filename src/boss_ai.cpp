#include "boss_ai.hpp"
#include "world_system.hpp"
#include "components.hpp"
#include <iostream>

#include "cutscene.hpp"

// Ecah struct below reprsenets a state transition

float peck_cooldown_timer = 0.3f;
float last_peck = 0;
float can_attack = 1.f;

enum class STATE {
	IDLE = 0,
	WALK = IDLE + 1,
	PECK = WALK + 1,
	FLAME = PECK + 1,
    HIT = FLAME + 1,
	DEATH = HIT + 1
};

static STATE current_state;
Entity boss_room;
Entity chicken;
float flame_cooldown = 0;
//float WALKING_CHICKEN_WIDTH = 500.f;
//float WALKING_CHICKEN_HEIGHT = 500.f;
constexpr float WALKING_CHICKEN_WIDTH = 0.58f * 866.f;
constexpr float WALKING_CHICKEN_HEIGHT = 0.58f * 867.f;
constexpr float IDLE_CHICKEN_WIDTH = 0.625f * 800.f;
constexpr float IDLE_CHICKEN_HEIGHT = 0.625f * 840.f;
constexpr float PECK_CHICKEN_WIDTH = 0.58f * 1059.f;
constexpr float PECK_CHICKEN_HEIGHT = 0.58f * 866.f;
constexpr float HIT_CHICKEN_WIDTH = 0.59f * 786.f;
constexpr float HIT_CHICKEN_HEIGHT = 0.59f * 846.f;
constexpr float DEATH_CHICKEN_WIDTH = 0.6f * 960.f;
constexpr float DEATH_CHICKEN_HEIGHT = 0.6f * 565.f;

bool walkLeft = false;
bool walkRight = false;

bool superFlameDone = false;

bool canSuperFlame(Motion chickenMotion, Motion playerMotion){
    Health health = registry.healths.get(chicken);
    if(!superFlameDone && health.current_health <= 5){
        superFlameDone = true;
        return true;
    }
    return false;
}

bool canWalk(Motion& chickenMotion, Motion& playerMotion) {
	if ((playerMotion.position.x <= renderSystem.getWindowWidth() / 3.f &&
		playerMotion.position.x >= renderSystem.getWindowWidth() / 5.f &&
		chickenMotion.position.x >= renderSystem.getWindowWidth() / 2.5f) &&
		playerMotion.position.x + 300 < chickenMotion.position.x) {
		// move left
		walkLeft = true;
		return true;
	}
	if (chickenMotion.position.x <= renderSystem.getWindowWidth() / 2.f
		&& playerMotion.position.x < renderSystem.getWindowWidth() / 10.f) {
		// move right
		walkRight = true;
		return true;
	}

	return false;
}

void walk(Motion& chickenMotion, Motion& playerMotion) {
	if (walkLeft) {
		chickenMotion.position.x -= 1.f;
	}
	else if (walkRight) {
		chickenMotion.position.x += 1.f;
	}

}

bool canPeck(Motion chickenMotion, Motion playerMotion, float time) {
    peck_cooldown_timer -= time;
    if(peck_cooldown_timer > 0){
        return false;
    }
    if (chickenMotion.position.x < playerMotion.position.x + 320.f &&
        chickenMotion.position.x > playerMotion.position.x) {
        peck_cooldown_timer = 0.3f;
        return true;
    }
    return false;
}
bool canFlame(Motion chickenMotion, Motion playerMotion, float time) {
    if (flame_cooldown <= 0) {
		return true;
	}
    flame_cooldown = flame_cooldown - time;
	return false;
}

bool animationDone = false;

Entity BossAISystem::init(Entity bossRoom) {
	current_state = STATE::IDLE;
	boss_room = bossRoom;
	// create and place chicken
	Motion chickenMotion;
	chickenMotion.position = glm::vec2(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f + 90.f);
	chickenMotion.velocity = glm::vec2(0, 0);
	chickenMotion.scale = { WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT };
	registry.motions.emplace(chicken, chickenMotion);

	Animation<ChickenState> chickenAnimations(CHICKEN_PECK);
	std::vector<Sprite> idleSprite;
	std::vector<Sprite> walkingSprites;
	std::vector<Sprite> peckingSprites;
	std::vector<Sprite> flameSprite;
    std::vector<Sprite> hitSprite;
	std::vector<Sprite> deathSprite;

	// Idle
	idleSprite.push_back(renderSystem.loadTexture("ChickenIdle.png"));

	// walking
	for (unsigned i = 1; i <= 6; i++) {
		walkingSprites.push_back(renderSystem.loadTexture("ChickenWalk" + std::to_string(i) + ".png"));
	}

	// Pecking
	for (unsigned i = 1; i <= 2; i++) {
		Sprite sprite(renderSystem.loadTexture("ChickenPeck" + std::to_string(i) + ".png"));
		peckingSprites.push_back(sprite);
	}

	for (unsigned i = 1; i <= 2; i++) {
		peckingSprites.push_back(renderSystem.loadTexture("ChickenPeck" + std::to_string(3 - i) + ".png"));
	}

	// Flame
	Sprite flame_sprite(renderSystem.loadTexture("ChickenFire.png"));
	flameSprite.push_back(flame_sprite);
	flameSprite.push_back(flame_sprite);
	flameSprite.push_back(flame_sprite);
	flameSprite.push_back(flame_sprite);

    // Hit
    hitSprite.push_back(renderSystem.loadTexture("ChickenHit.png"));

	// Death
	deathSprite.push_back(renderSystem.loadTexture("ChickenDead.png"));

	chickenAnimations.addState(ChickenState::CHICKEN_IDLE, std::move(idleSprite));
	chickenAnimations.addState(ChickenState::CHICKEN_WALK, std::move(walkingSprites));
	chickenAnimations.addState(ChickenState::CHICKEN_PECK, std::move(peckingSprites));
	chickenAnimations.addState(ChickenState::CHICKEN_FLAME, std::move(flameSprite));
    chickenAnimations.addState(ChickenState::CHICKEN_HIT, std::move(hitSprite));
	chickenAnimations.addState(ChickenState::CHICKEN_DEATH, std::move(deathSprite));

	registry.chickenAnimations.emplace(chicken, std::move(chickenAnimations));

	TransformComponent chickenTransform;
	chickenTransform.position = glm::vec3(renderSystem.getWindowWidth() / 2.0f, renderSystem.getWindowHeight() / 2.0f, 0.0f);
	chickenTransform.scale = glm::vec3(WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT, 1.0f);
	chickenTransform.rotation = 0.0f;
	registry.transforms.emplace(chicken, std::move(chickenTransform));

	Boss chickenBoss = Boss();
	chickenBoss.hitbox = { WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT };
	chickenBoss.attackbox = { WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT };
	chickenBoss.bodybox = { WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT };
	registry.bosses.emplace(chicken, chickenBoss);

	if (!isChickenDead) {
		registry.healths.emplace(chicken, std::move(Health{ 10, 10 }));
		registry.damages.emplace(chicken, std::move(Damage{ 1 }));
	}
	else {
		registry.healths.emplace(chicken, std::move(Health{ 10, 0 }));
		registry.gravity.emplace(chicken, std::move(Gravity()));
	}

	return chicken;
};

void BossAISystem::step(Entity player, float elapsed_time) {
	if (!registry.chickenAnimations.has(chicken)) {
		return;
	}
	auto& a = registry.chickenAnimations.get(chicken);
	Motion& chickenMotion = registry.motions.get(chicken);
	Motion& playerMotion = registry.motions.get(player);
    Boss& chickenBoss = registry.bosses.get(chicken);

	// check for death
	// check for death
	if (registry.healths.get(chicken).current_health <= 0) {
		current_state = STATE::DEATH;
		a.setState(CHICKEN_DEATH);

        chickenBoss.hitbox = {DEATH_CHICKEN_WIDTH, DEATH_CHICKEN_HEIGHT};
        chickenBoss.attackbox = {DEATH_CHICKEN_WIDTH, DEATH_CHICKEN_HEIGHT};
        chickenBoss.bodybox = {DEATH_CHICKEN_WIDTH, DEATH_CHICKEN_HEIGHT};
	}
	else if (animationDone) {
		animationDone = false;
		if (current_state == STATE::IDLE) {
            chickenBoss.hitbox = {WALKING_CHICKEN_WIDTH - 50.f, WALKING_CHICKEN_HEIGHT};
            chickenBoss.attackbox = {WALKING_CHICKEN_WIDTH - 200.f, WALKING_CHICKEN_HEIGHT};
            chickenBoss.bodybox = {WALKING_CHICKEN_WIDTH - 200.f, WALKING_CHICKEN_HEIGHT + 200.f};

			if (canPeck(chickenMotion, playerMotion, elapsed_time)) {
				current_state = STATE::PECK;
				a.setState(CHICKEN_PECK);
			}
			else if (canWalk(chickenMotion, playerMotion)) {
				current_state = STATE::WALK;
				a.setState(CHICKEN_WALK);
			}
			else if (canFlame(chickenMotion, playerMotion, elapsed_time)) {
				current_state = STATE::FLAME;
				a.setState(CHICKEN_FLAME);
				flame_cooldown = 1.5f;
                if(canSuperFlame(chickenMotion, playerMotion)){

                    float pos1 = chickenMotion.position.x - 100.f;
                    float pos2 = chickenMotion.position.x - 200.f;
                    float pos3 = chickenMotion.position.x - 300.f;
                    float pos4 = chickenMotion.position.x - 400.f;
                    float pos5 = chickenMotion.position.x - 500.f;
                    if(pos1 != playerMotion.position.x){
                        flame_attack(pos1);
                    }
                    if(pos2 != playerMotion.position.x){
                        flame_attack(pos2);
                    }
                    if(pos3 != playerMotion.position.x){
                        flame_attack(pos3);
                    }
                    if(pos4 != playerMotion.position.x){
                        flame_attack(pos4);
                    }
                    if(pos5 != playerMotion.position.x){
                        flame_attack(pos5);
                    }
                }else{
                    flame_attack(renderSystem.getWindowWidth() * uniform_dist(rng));
                    flame_attack(renderSystem.getWindowWidth() * uniform_dist(rng));
                    // flame_attack(renderSystem.getWindowWidth() * (2.f/3.f));
                    flame_attack(renderSystem.getWindowWidth() * uniform_dist(rng));
                }
			}

		}
		else if (current_state == STATE::PECK) {
            chickenBoss.hitbox = {PECK_CHICKEN_WIDTH, PECK_CHICKEN_HEIGHT * 0.6f};
            chickenBoss.attackbox = {PECK_CHICKEN_WIDTH - 100.f, PECK_CHICKEN_HEIGHT * 0.6f};
            chickenBoss.bodybox = {PECK_CHICKEN_WIDTH - 100.f, PECK_CHICKEN_HEIGHT + 200.f};

			if (canWalk(chickenMotion, playerMotion)) {
				current_state = STATE::WALK;
				a.setState(CHICKEN_WALK);
			}
			else if (canFlame(chickenMotion, playerMotion, elapsed_time)) {
				current_state = STATE::FLAME;
				a.setState(CHICKEN_FLAME);
				flame_cooldown = 1.5f;
				flame_attack(renderSystem.getWindowWidth() * uniform_dist(rng));
				flame_attack(renderSystem.getWindowWidth() * uniform_dist(rng));
				// flame_attack(renderSystem.getWindowWidth() * (2.f / 3.f));
				flame_attack(renderSystem.getWindowWidth() * uniform_dist(rng));
			}
			else {
				current_state = STATE::IDLE;
				a.setState(CHICKEN_IDLE);
			}
		}
		else if (current_state == STATE::WALK) {
			walkRight = false;
			walkLeft = false;

            chickenBoss.hitbox = {WALKING_CHICKEN_WIDTH - 50.f, WALKING_CHICKEN_HEIGHT};
            chickenBoss.attackbox = {WALKING_CHICKEN_WIDTH - 200.f, WALKING_CHICKEN_HEIGHT};
            chickenBoss.bodybox = {WALKING_CHICKEN_WIDTH - 200.f, WALKING_CHICKEN_HEIGHT + 200.f};

			if (canPeck(chickenMotion, playerMotion, elapsed_time)) {
				current_state = STATE::PECK;
				a.setState(CHICKEN_PECK);
			}
			else if (canWalk(chickenMotion, playerMotion)) {
				current_state = STATE::WALK;
				a.setState(CHICKEN_WALK);
			}
			else {
				current_state = STATE::IDLE;
				a.setState(CHICKEN_IDLE);
			}
		}
		else if (current_state == STATE::FLAME) {

            chickenBoss.hitbox = {WALKING_CHICKEN_WIDTH - 50.f, WALKING_CHICKEN_HEIGHT};
            chickenBoss.attackbox = {WALKING_CHICKEN_WIDTH - 200.f, WALKING_CHICKEN_HEIGHT};
            chickenBoss.bodybox = {WALKING_CHICKEN_WIDTH - 200.f, WALKING_CHICKEN_HEIGHT + 200.f};

			if (canWalk(chickenMotion, playerMotion)) {
				current_state = STATE::WALK;
				a.setState(CHICKEN_WALK);
			}
			else {
				current_state = STATE::IDLE;
				a.setState(CHICKEN_IDLE);
			}
		}
	}


	if (a.isAnimationComplete()) {
		animationDone = true;
	}
	else {
		if (current_state == STATE::WALK) {
			walk(chickenMotion, playerMotion);
		}
		a.next(elapsed_time);
	}

//	if (flame_cooldown > 0) {
//		flame_cooldown--;
//	}

	switch (current_state) {
	case STATE::IDLE:
		chickenMotion.scale = { IDLE_CHICKEN_WIDTH, IDLE_CHICKEN_HEIGHT };
		break;
	case STATE::WALK:
		chickenMotion.scale = { WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT };
		break;
	case STATE::PECK:
		chickenMotion.scale = { PECK_CHICKEN_WIDTH, PECK_CHICKEN_HEIGHT };
		break;
	case STATE::FLAME:
		chickenMotion.scale = { WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT };
		break;
	case STATE::HIT:
		chickenMotion.scale = { HIT_CHICKEN_WIDTH, HIT_CHICKEN_HEIGHT };
		break;
	case STATE::DEATH:
		chickenMotion.scale = { DEATH_CHICKEN_WIDTH, DEATH_CHICKEN_HEIGHT };
		break;
	}

	switch (chickenBoss.boxType) {
	case BoxType::ATTACK_BOX:
		chickenMotion.boundingBox = chickenBoss.attackbox;
	case BoxType::BODY_BOX:
		chickenMotion.boundingBox = chickenBoss.bodybox;
	case BoxType::HIT_BOX:
		chickenMotion.boundingBox = chickenBoss.hitbox;
	}
};

void BossAISystem::render() {
	if (registry.chickenAnimations.has(chicken) &&
		registry.transforms.has(chicken)) {
		auto& animation = registry.chickenAnimations.get(chicken);
		auto& transform = registry.transforms.get(chicken);
		renderSystem.drawEntity(animation.getCurrentFrame(), transform);
	}
}

void BossAISystem::chicken_get_damaged(Entity weapon, bool& isDead, bool& a_pressed, bool& d_pressed, Entity& player) {
	Health& chicken_health = registry.healths.get(chicken);
	Damage& weapon_damage = registry.damages.get(weapon);
	// if (chicken_health.current_health - weapon_damage.damage_dealt >= 0) {
    if (chicken_health.current_health > 0) {
        if (chicken_health.current_health - weapon_damage.damage_dealt > 0) {
			if (!registry.recentDamageTimers.has(chicken)) {
				registry.recentDamageTimers.emplace(chicken, RecentlyDamagedTimer());
			}
            registry.chickenAnimations.get(chicken).setState(CHICKEN_HIT);
            current_state = STATE::HIT;
        }

		chicken_health.current_health -= weapon_damage.damage_dealt;
		printf("Chicken now has %d hearts\n", chicken_health.current_health);
		if (chicken_health.current_health <= 0) {
			registry.damages.remove(chicken);
			isDead = true;
			a_pressed = false;
			d_pressed = false;
			registry.motions.get(player).velocity.x = 0;
			Mix_HaltMusic();
			registry.gravity.emplace(chicken, Gravity());
			renderSystem.getGameStateManager()->pauseState<PickupCutscene>();
		}
	}
}

// Add this function to manage the chicken’s transition back to WALK after HIT
void BossAISystem::update_damaged_chicken_sprites(float delta_time) {
    for (Entity entity : registry.recentDamageTimers.entities) {
        if (registry.chickenAnimations.has(entity)) {
            RecentlyDamagedTimer& damaged_timer = registry.recentDamageTimers.get(entity);
            damaged_timer.counter_ms -= delta_time;
            if (damaged_timer.counter_ms <= 0) {
                registry.chickenAnimations.get(entity).setState(CHICKEN_WALK);
                current_state = STATE::WALK;
                registry.motions.get(entity).scale = { WALKING_CHICKEN_WIDTH, WALKING_CHICKEN_HEIGHT };
                registry.recentDamageTimers.remove(entity);
            }
        }
    }
}

void BossAISystem::flame_attack(float x_pos) {
	Entity flame = Entity();

	Sprite flameSprite = renderSystem.loadTexture("ChickenFireball.png");
	registry.sprites.emplace(flame, renderSystem.loadTexture("ChickenFireball.png"));

	Motion motion;
	motion.position = { x_pos, 0.0f };
	motion.scale = { flameSprite.width / 4.f, flameSprite.height / 4.f };
	registry.motions.emplace(flame, std::move(motion));

	// Create and initialize a TransformComponent for the background
	TransformComponent flameTransform;
	flameTransform.position = glm::vec3(x_pos, 0.0, 0.0);
	flameTransform.scale = glm::vec3(flameSprite.width / 4.f, flameSprite.height / 4.f, 1.0);
	flameTransform.rotation = 3.14f + 3.14f / 2.f;
	registry.transforms.emplace(flame, std::move(flameTransform));

	registry.projectiles.emplace(flame, std::move(Projectile{ ProjectileType::SPIT }));
	registry.gravity.emplace(flame, std::move(Gravity()));
	registry.damages.emplace(flame, std::move(Damage{ 1 }));
	registry.hostiles.emplace(flame, std::move(Hostile()));

	registry.rooms.get(boss_room).insert(flame);
}


