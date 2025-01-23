#pragma once
#include <memory>
#include "region.hpp"
#include <functional>

class Region;

using RegionFactory = std::function<std::unique_ptr<Region>()>;

class RegionManager {
private:
	std::unique_ptr<Region> currentRegion;
public:
	RegionManager() = default;
	~RegionManager() = default;
	void init();
	Entity setRegion(const RegionFactory& region);
};
