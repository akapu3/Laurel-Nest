#pragma once

#include "ecs.hpp"
#include "region.hpp"

class Cesspit : public Region {
public:
	Cesspit();
	~Cesspit();

	void init() override;

    Entity m_entrance_room;
    Entity m_room1;
    Entity m_room2;
    Entity m_room3;
    Entity m_room4;
    Entity m_boss_room;
    Entity m_exit_room;

    Connection SetDoor(float width, float height, float xPos, float yPos, Entity connectNextRoom, vec2 connectNextSpawn);

private:

};