#pragma once
#include "world_system.hpp"
#include "room_strategy.hpp"

class Region {
public:
	virtual ~Region() = default;
	virtual void init() = 0;
	Entity start_room;
protected:
	std::unique_ptr<RoomStrategy> currentRoom;
	void setRoomStrategy(std::unique_ptr<RoomStrategy> roomStrat);
};
