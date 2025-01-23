#pragma once
#include <memory>

template<typename R>
std::unique_ptr<Region> makeRegion() {
	return std::make_unique<R>();
}
