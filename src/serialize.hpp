#pragma once
#define SAVE_FILE_PATH "saveFile.txt"
#include <string>
//#define GOD_MODE

enum class SAVEFILE_LINES {
	IS_INIT,
	PLAYER_MAX_HEALTH,
	PLAYER_CURRENT_HEALTH,
	HEALTH_FLASK_USES,
	SWORD_DAMAGE,
	HEART_POWER_UP_0,
	HEART_POWER_UP_1,
	SWORD_POWER_UP_0,
	IS_CHICKEN_DEAD,
	START_FROM_CHECKPOINT,
	SAVED_THIS_INSTANCE,
	SAVEFILE_LINES
};

//enum class SAVEFILE_LINES {
//	PLAYER_MAX_HEALTH = 0,
//	PLAYER_CURRENT_HEALTH = PLAYER_MAX_HEALTH + 1,
//	HEALTH_FLASK_USES = PLAYER_CURRENT_HEALTH +1,
//	HEART_POWER_UP_0 = HEALTH_FLASK_USES +1,
//	HEART_POWER_UP_1 = HEART_POWER_UP_0 +1,
//	SWORD_POWER_UP_0 = HEART_POWER_UP_1 +1,
//	IS_CHICKEN_DEAD = SWORD_POWER_UP_0 +1,
//	START_FROM_CHECKPOINT = IS_CHICKEN_DEAD + 1,
//	SAVED_THIS_INSTANCE = START_FROM_CHECKPOINT + 1,
//	SAVEFILE_LINES = SAVED_THIS_INSTANCE +1
//};

struct SaveFile {
	bool is_init;
	int player_max_health = 3;
	int player_current_health = 3;
	int health_flask_uses = 3;
	int sword_damage = 1;
	bool heart_power_up_0 = false;
	bool heart_power_up_1 = false;
	bool sword_power_up_0 = false;
	bool is_chicken_dead = false;
	bool start_from_checkpoint = false;
	bool saved_this_instance = false;
};

// opens file, returns int stored at line from file, or default value if can't
int readIntFromFile(const std::string& filePath, int lineNumber, int defaultValue);

// opens file, returns bool stored at line from file, or default value if can't
bool readBoolFromFile(const std::string& filePath, int lineNumber, bool defaultValue);
const std::string BoolToString(bool b);

void readFromSaveFile(const std::string& filePath, SaveFile& sf);


constexpr int line_count = static_cast<int>(SAVEFILE_LINES::SAVEFILE_LINES);

void clearSaveData();
