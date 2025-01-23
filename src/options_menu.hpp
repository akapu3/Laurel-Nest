#pragma once
#include "pause_state.hpp"

class OptionsMenu : public MenuState {
public:
	~OptionsMenu();
	void init() override;
	void cleanup() override;
	void update(float deltaTime) override;
	void on_mouse_click(int button, int action, const glm::vec2& position, int mods) override;
	void on_key(int key, int scancode, int action, int mods) override;
	void render() override;

private:
	Entity helpEntity;
	Entity clearSaveEntity;
	Entity tutorialEntity;
	Entity esc_key;
	Entity control_keys;
	Entity mouse_click;
	Entity h_key;
	Entity e_key;
	Entity q_key;
	bool showTutorial = false;
};
