#include "options_menu.hpp"
#include <fstream>
#include "serialize.hpp"
#include "splash_screen_state.hpp"
#include <iostream>

OptionsMenu::~OptionsMenu() {
	OptionsMenu::cleanup();
}

void OptionsMenu::init() {
	MenuItem helpComponent(renderSystem.loadTexture("menu/help_active.png"), renderSystem.loadTexture("menu/help_inactive.png"),
		renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f - 100.f);
	registry.menuItems.emplace(helpEntity, helpComponent);

	MenuItem clearSaveComponent(renderSystem.loadTexture("menu/clear_data_active.png"), renderSystem.loadTexture("menu/clear_data_inactive.png"),
		renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f + 150.f);
	registry.menuItems.emplace(clearSaveEntity, clearSaveComponent);

	Sprite escSprite(renderSystem.loadTexture("tutorial/esc_key.PNG"));
	registry.sprites.emplace(esc_key, escSprite);
	registry.transforms.emplace(esc_key, TransformComponent{
		vec3(renderSystem.getWindowWidth() * 0.08f, renderSystem.getWindowHeight() * 0.94f, 0.f),
		vec3(escSprite.width * 0.3f, escSprite.height * 0.3f, 1.f), 0.f
		});

	Sprite tutorialSprite(renderSystem.loadTexture("tutorial/box.PNG"));
	registry.sprites.emplace(tutorialEntity, tutorialSprite);
	registry.transforms.emplace(tutorialEntity, TransformComponent{
		vec3(renderSystem.getWindowWidth() / 2.f, renderSystem.getWindowHeight() / 2.f, 0.f),
		vec3(tutorialSprite.width * 0.35f, tutorialSprite.height * 0.43f, 1.f), 0.f
		});

	Sprite controlSprite(renderSystem.loadTexture("tutorial/control_keys.PNG"));
	registry.sprites.emplace(control_keys, controlSprite);
	registry.transforms.emplace(control_keys, TransformComponent{
		vec3(renderSystem.getWindowWidth() * 0.2f, renderSystem.getWindowHeight() * 0.18f, 0.f),
		vec3(controlSprite.width * 0.3f, controlSprite.height * 0.3f, 1.f), 0.f
		});

	Sprite mouseSprite(renderSystem.loadTexture("tutorial/mouse_click.PNG"));
	registry.sprites.emplace(mouse_click, mouseSprite);
	registry.transforms.emplace(mouse_click, TransformComponent{
		vec3(renderSystem.getWindowWidth() * 0.2f, renderSystem.getWindowHeight() * 0.36f, 0.f),
		vec3(mouseSprite.width * 0.3f, mouseSprite.height * 0.3f, 1.f), 0.f
		});

	Sprite hSprite(renderSystem.loadTexture("tutorial/H_key.PNG"));
	registry.sprites.emplace(h_key, hSprite);
	registry.transforms.emplace(h_key, TransformComponent{
		vec3(renderSystem.getWindowWidth() * 0.2f, renderSystem.getWindowHeight() * 0.51f, 0.f),
		vec3(hSprite.width * 0.4f, hSprite.height * 0.4f, 1.f), 0.f
		});

	Sprite eSprite(renderSystem.loadTexture("tutorial/E_key.PNG"));
	registry.sprites.emplace(e_key, eSprite);
	registry.transforms.emplace(e_key, TransformComponent{
		vec3(renderSystem.getWindowWidth() * 0.15f, renderSystem.getWindowHeight() * 0.735f, 0.f),
		vec3(eSprite.width * 0.4f, eSprite.height * 0.4f, 1.f), 0.f
		});

	Sprite qSprite(renderSystem.loadTexture("tutorial/Q_key.PNG"));
	registry.sprites.emplace(q_key, qSprite);
	registry.transforms.emplace(q_key, TransformComponent{
		vec3(renderSystem.getWindowWidth() * 0.66f, renderSystem.getWindowHeight() * 0.735f, 0.f),
		vec3(qSprite.width * 0.4f, qSprite.height * 0.4f, 1.f), 0.f
		});
	
}

void OptionsMenu::cleanup() {
	registry.remove_all_components_of(helpEntity);
	registry.remove_all_components_of(clearSaveEntity);
	registry.remove_all_components_of(tutorialEntity);
	registry.remove_all_components_of(esc_key);
	registry.remove_all_components_of(control_keys);
	registry.remove_all_components_of(mouse_click);
	registry.remove_all_components_of(h_key);
	registry.remove_all_components_of(e_key);
	registry.remove_all_components_of(q_key);
}

void OptionsMenu::update(float) {

}

void OptionsMenu::on_mouse_click(int, int, const vec2&, int) {
	if (registry.menuItems.get(helpEntity).isPointWithin(mouse_pos)) {
		showTutorial = true;
	} else if (registry.menuItems.get(clearSaveEntity).isPointWithin(mouse_pos)) {
		clearSaveData();
		renderSystem.getGameStateManager()->resetPausedStates<SplashScreenState>();
	}
}

void OptionsMenu::on_key(int key, int scancode, int action, int mods) {
	MenuState::on_key(key, scancode, action, mods);
	if (action == GLFW_RELEASE && key == GLFW_KEY_N) {
		showTutorial = false;
	}
}

void OptionsMenu::render() {
	MenuState::render();
	renderMenuItem(registry.menuItems.get(helpEntity), mouse_pos);
	renderMenuItem(registry.menuItems.get(clearSaveEntity), mouse_pos);
	if (showTutorial) {
		renderSystem.drawEntity(registry.sprites.get(tutorialEntity), registry.transforms.get(tutorialEntity));

		renderSystem.drawEntity(registry.sprites.get(control_keys), registry.transforms.get(control_keys));
		renderSystem.renderText("Keys To Control Player Movement", window_width_px * 0.3f, window_height_px * 0.8f, 0.8f, vec3(1), mat4(1));
		
		renderSystem.drawEntity(registry.sprites.get(mouse_click), registry.transforms.get(mouse_click));
		renderSystem.renderText("Left Click Mouse To Attack", window_width_px * 0.25f, window_height_px * 0.62f, 0.8f, vec3(1), mat4(1));
		
		renderSystem.drawEntity(registry.sprites.get(h_key), registry.transforms.get(h_key));
		renderSystem.renderText("Hold For 2 Seconds To Restore Health Up To 3 Times", window_width_px * 0.25f, window_height_px * 0.48f, 0.8f, vec3(1), mat4(1));
		
		renderSystem.renderText("After Defeating The Flame Chicken", window_width_px * 0.3f, window_height_px * 0.37f, 1.0f, vec3(1), mat4(1));
		
		renderSystem.drawEntity(registry.sprites.get(e_key), registry.transforms.get(e_key));
		renderSystem.renderText("Key To Equip Beak of Fire", window_width_px * 0.18f, window_height_px * 0.25f, 0.8f, vec3(1), mat4(1));
		
		renderSystem.drawEntity(registry.sprites.get(q_key), registry.transforms.get(q_key));
		renderSystem.renderText("Key To Unequip", window_width_px * 0.7f, window_height_px * 0.25f, 0.8f, vec3(1), mat4(1));

	}
	renderSystem.drawEntity(registry.sprites.get(esc_key), registry.transforms.get(esc_key));
	renderSystem.renderText("To Return To Menu", window_width_px * 0.1f, window_height_px * 0.05f, 0.5f, vec3(1), mat4(1));
}
