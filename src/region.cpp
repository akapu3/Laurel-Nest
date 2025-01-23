#include "region.hpp"

void Region::setRoomStrategy(std::unique_ptr<RoomStrategy> roomStrat) {
	currentRoom = std::move(roomStrat);
}
