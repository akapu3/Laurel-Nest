#include "region_manager.hpp"
#include <stdexcept>

void RegionManager::init() {

}

Entity RegionManager::setRegion(const RegionFactory& region) {
	if (region) {
		currentRegion = std::move(region());
		currentRegion->init();
		return currentRegion->start_room;
	}
	throw std::runtime_error("Failed to set region.");
}
