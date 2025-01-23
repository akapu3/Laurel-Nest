#pragma once

// standard libs
#include <string>
#include <tuple>
#include <vector>
#include <type_traits>

// glfw (OpenGL)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) { return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name; };
inline std::string textures_path(const std::string& name) { return data_path() + "/textures/" + std::string(name); };
inline std::string audio_path(const std::string& name) { return data_path() + "/audio/" + std::string(name); };
inline std::string mesh_path(const std::string& name) { return data_path() + "/meshes/" + std::string(name); };

constexpr int window_width_px = 1920;
constexpr int window_height_px = 1080;
constexpr float TPS = 500.f;
constexpr float max_delta_time = 1 / 100.f;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	Transform& scale(vec2 scale);
	Transform& rotate(float radians);
	Transform& translate(vec2 offset);
};

bool gl_has_errors();

// no C++ 17 smh
	// Basically allows this to be called on anything that's a number (float, int, unsigned int, etc.)
	template <typename T1, typename T2, typename T3>
	inline std::enable_if_t<std::is_arithmetic<T1>::value &&
	                      std::is_arithmetic<T2>::value &&
	                      std::is_arithmetic<T3>::value, std::common_type_t<T1, T2, T3>>
clamp(T1 n, T2 min, T3 max) {
		using T = std::common_type_t<T1, T2, T3>;
		T tn = static_cast<T>(n);
		T tmin = static_cast<T>(min);
		T tmax = static_cast<T>(max);

		if (tn < tmin) {
			return tmin;
		}
		if (tn > tmax) {
			return tmax;
		}
		return tn;
	}

// Returns 1, -1, or 0 depending on the sign of a number
template <typename Number>
inline int signof(Number n) {
	if (n > 0) {
		return 1;
	}
	else if (n < 0) {
		return -1;
	}
	return 0;
}

