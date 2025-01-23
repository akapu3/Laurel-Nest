#pragma once
#include "ecs.hpp"
#include "render_system.hpp"

class RoomStrategy {
public:
	virtual Entity execute() = 0;
	virtual ~RoomStrategy() = default;

    Entity SetBG(Sprite bgSprite);

    Entity SetCeiling(Sprite ceilingSprite, float xPos);

    Entity SetGround(Sprite groundSprite, float width, float height, float xPos, float yPos);

	Entity SetWall(Sprite groundSprite, float left, float width, float height, float xPos, float yPos);

    Entity SetPlatform(Sprite platformSprite, float width, float height, float xPos, float yPos);

	Entity SetSpikeObstacle(float width, float height, float xPos, float yPos);

	Entity SetMovingPlatform(Sprite platformSprite, bool vertical, float width, float height, float xPos, float yPos, vec2 startPos, vec2 endPos, bool moving);

	Entity SetBGElem(Sprite elemSprite, float width, float height, float xPos, float yPos, float rot);
	
	Entity SetCheckpoint(float xPos, float yPos);

	Entity SetPelican(float xPos, float yPos);
	Entity SetBirdmanElder(float xPos, float yPos);
	Entity SetOgreKat(float xPos, float yPos);

    // for testing
	Entity SetDoorEx(float width, float height, float xPos, float yPos);
    //
};

// cesspit
class CPEntranceRoomStrategy : public RoomStrategy {
	Entity execute() override;
};

class CPRoom1Strategy : public RoomStrategy {
	Entity execute() override;
};

class CPRoom2Strategy : public RoomStrategy {
	Entity execute() override;
};

class CPRoom3Strategy : public RoomStrategy {
	Entity execute() override;
};

class CPRoom4Strategy : public RoomStrategy {
	Entity execute() override;
};

class CPBossRoomStrategy : public RoomStrategy {
	Entity execute() override;
};

class CPExitRoomStrategy : public RoomStrategy {
	Entity execute() override;
};

// birdman town
class BMTEntranceRoomStrategy : public RoomStrategy {
	Entity execute() override;
};

class BMTRoom1Strategy : public RoomStrategy {
	Entity execute() override;
};

class BMTRoom2Strategy : public RoomStrategy {
	Entity execute() override;
};

class BMTRoom3Strategy : public RoomStrategy {
	Entity execute() override;
};

class BMTRoom4Strategy : public RoomStrategy {
	Entity execute() override;
};

class BMTRoom5Strategy : public RoomStrategy {
	Entity execute() override;
};

// npc rooms
class NPCRoom1Strategy : public RoomStrategy {
	Entity execute() override;
};

class NPCRoom2Strategy : public RoomStrategy {
	Entity execute() override;
};

// laurel nest
class LNRoom1Strategy : public RoomStrategy {
	Entity execute() override;
};

class LNRoom2Strategy : public RoomStrategy {
	Entity execute() override;
};

class LNBossRoomStrategy : public RoomStrategy {
	Entity execute() override;
};

