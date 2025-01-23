#pragma once
#include "game_state.hpp"
#include "components.hpp"
#include "splash_screen_state.hpp"
#include <array>
#include <boost/optional.hpp>
#include <sstream>

#include "serialize.hpp"

constexpr int LAST_OPENING_ANIMATION_FRAME = 18;
constexpr int LAST_PICKUP_ANIMATION_FRAME = 6;
constexpr int LAST_ENDING_ANIMATION_FRAME = 5;

template<int Frames, int FramesPerSecond>
struct Cutscene : public GameState {
protected:
	Cutscene(float seconds_passed_, int frameCount_): seconds_passed(seconds_passed_), frameCount(frameCount_) {};
	static constexpr int totalFrames = Frames;
	static constexpr float SECONDS_PER_FRAME = 1.f / FramesPerSecond;
	float seconds_passed;
	int frameCount;
	std::array<boost::optional<Sprite>, totalFrames> frames;
};

class OpeningCutscene : public Cutscene<LAST_OPENING_ANIMATION_FRAME, 2> {
public:
	OpeningCutscene();
	~OpeningCutscene() override;

	void init() override {}
	void cleanup() override {}
	void on_key(int key, int scancode, int action, int mods) override;
	void on_mouse_click(int button, int action, const glm::vec2& position, int mods) override;
	void on_mouse_move(const vec2& position) override;
	void update(float deltaTime) override;
	void render() override;

private:
	bool hasLoaded;
	bool isShowingTutorial;

	Entity tutorialEntity;
	Entity control_keys;
	Entity mouse_click;
	Entity h_key;
	Entity e_key;
	Entity q_key;
};

class PickupCutscene : public Cutscene<LAST_PICKUP_ANIMATION_FRAME, 2> {
public:
	PickupCutscene();
	~PickupCutscene() override = default;

	void init() override {}
	void cleanup() override {}
	void on_key(int key, int scancode, int action, int mods) override {};
	void on_mouse_click(int button, int action, const glm::vec2& position, int mods) override {};
	void on_mouse_move(const vec2& position) override {};
	void update(float deltaTime) override;
	void render() override;

private:
	float transitionFrame;
	bool finishedCutscene;
};

template<int Which>
class EndingCutscene : public Cutscene<LAST_ENDING_ANIMATION_FRAME, 1> {
public:
	EndingCutscene();
	~EndingCutscene() override = default;

	void init() override {}
	void cleanup() override {}
	void on_key(int key, int scancode, int action, int mods) override {};
	void on_mouse_click(int button, int action, const glm::vec2& position, int mods) override {};
	void on_mouse_move(const vec2& position) override {};
	void update(float deltaTime) override;
	void render() override;

private:
	float transitionFrame;
	bool finishedCutscene;
};

template<int Which>
EndingCutscene<Which>::EndingCutscene() : Cutscene(0.f, 0), transitionFrame(-0.5f), finishedCutscene(false) {
	clearSaveData();
	std::array<std::future<Image>, totalFrames> images;
	std::atomic<int> count;
	for (int i = 0; i < totalFrames; i++) {
		std::stringstream filePath;
		filePath << "ending_" << Which << "/" << i << ".png";
		images[i] = loadImageData(filePath.str(), count);
	}

	for (int i = 0; i < totalFrames; i++) {
		frames[i] = bindTexture(images[i].get());
	}
}

template<int Which>
void EndingCutscene<Which>::update(float deltaTime) {
	if (!finishedCutscene) {
		seconds_passed += deltaTime;
		if (seconds_passed > SECONDS_PER_FRAME) {
			seconds_passed = 0;
			if (++frameCount >= totalFrames) {
				finishedCutscene = true;
				renderSystem.captureScreen();
			}
		}
	}
	else {
		while (transitionFrame < 1.f) {
			transitionFrame += deltaTime * 2.f;
			renderSystem.doGlassBreakTransition(clamp(static_cast<int>(transitionFrame * 100), 0, 100), 100);
		}
		renderSystem.getGameStateManager()->resetPausedStates<SplashScreenState>();
	}
}

template<int Which>
void EndingCutscene<Which>::render() {
	if (!finishedCutscene) {
		TransformComponent transform{ vec3(window_width_px / 2.f, window_height_px / 2.f, 0.f), vec3(window_width_px, window_height_px, 1.f), 0.f };
		renderSystem.drawEntity(frames[frameCount].get(), transform);
	}
}
