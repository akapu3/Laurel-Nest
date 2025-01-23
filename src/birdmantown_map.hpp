#pragma once

#include "ecs.hpp"
#include "region.hpp"

class Birdmantown : public Region {
public:
    Birdmantown();
    ~Birdmantown();

    void init() override;

    Entity m_entrance_room;
    Entity m_room1;
    Entity m_room2;
    Entity m_room3;
    Entity m_room4;
    Entity m_room5;
    Entity m_npc_room1;
    Entity m_npc_room2;
    Entity m_ln_room1;
    Entity m_ln_room2;
    Entity m_ln_boss_room;

    Connection SetDoor(float width, float height, float xPos, float yPos, Entity connectNextRoom, vec2 connectNextSpawn);

private:

};
