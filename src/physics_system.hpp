#pragma once

#include "common.hpp"
#include "ecs.hpp"
#include "components.hpp"
#include "ecs_registry.hpp"
#include "threadpool.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
    ThreadPool threadPool;
	Entity player;
	Entity currentRoom;

public:
	void step(float elapsed_ms);
	void setRoom(Entity newRoom);
	void setPlayer(const Entity& newPlayer);
	bool checkForCollision(Entity e1, Entity e2, vec2& direction, vec2& overlap);

    PhysicsSystem() : threadPool(std::thread::hardware_concurrency())
	{
	}
};
