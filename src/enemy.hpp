#pragma once
#include "world_system.hpp"
#include "components.hpp"
#include "ecs_registry.hpp"

class Enemy  {

public:
	Entity entity;
	Enemy();

	void init(float x, float y) {
		init_sprite();
		init_components(x, y);
	};

	void set_direction(bool dir);
	void set_health(const int num);

private:
	virtual void init_sprite() = 0;
	virtual void init_components(float x, float y) = 0;
};

class GoombaLand : public Enemy {
public:
	GoombaLand();
private:
	void init_sprite() override;
	void init_components(float x, float y) override;
};

class GoombaCeiling : public Enemy {
public:
	float bottom_edge = 40.f;
	float with_platform = 37.f + bottom_edge;
	GoombaCeiling();
	void set_spit_timer(float time);
private:
	void init_sprite() override;
	void init_components(float x, float y) override;
};

class GoombaFlying : public Enemy {
public:
	GoombaFlying();
	void set_initial_attack(FlyingGoombaState state);
private:
	void init_sprite() override;
	void init_components(float x, float y) override;
};

class GoombaSwarm : public Enemy {
public:
	GoombaSwarm();
private:
	void init_sprite() override;
	void init_components(float x, float y) override;
};