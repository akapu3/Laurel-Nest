#include "serialize.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int readIntFromFile(const std::string& filePath, int lineNumber, int defaultValue) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		return defaultValue;
	}

	std::string line;
	int currentLine = 0;

	while (std::getline(file, line)) {
		if (currentLine == lineNumber) {
			std::istringstream iss(line);
			int value;
			if (iss >> value) {
				return value;
			}
			return defaultValue;
		}
		currentLine++;
	}
	return defaultValue;
}

bool readBoolFromFile(const std::string& filePath, int lineNumber, bool defaultValue) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return defaultValue;
    }

    std::string line;
    int currentLine = 0;

    while (std::getline(file, line)) {
        if (currentLine == lineNumber) {
            std::istringstream iss(line);
            bool value;
            if (iss >> std::boolalpha >> value) {
                return value;
            }
            return defaultValue;
        }
        currentLine++;
    }
    return defaultValue;
}

const std::string BoolToString(bool b)
{
	return b ? "true" : "false";
}

void readFromSaveFile(const std::string& filePath, SaveFile& sf) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		return;
	}

	std::string line;
	int currentLine = 0;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		int value;
		bool value_b;
		switch (static_cast<SAVEFILE_LINES>(currentLine)) {
		case SAVEFILE_LINES::IS_INIT:
			if (iss >> std::boolalpha >> value_b) {
				sf.is_init = value_b;
			}
			break;
		case SAVEFILE_LINES::PLAYER_MAX_HEALTH:
			if (iss >> value) {
				sf.player_max_health = value;
			}
			break;
		case SAVEFILE_LINES::PLAYER_CURRENT_HEALTH:
			if (iss >> value) {
				sf.player_current_health = value;
			}
			break;
		case SAVEFILE_LINES::HEALTH_FLASK_USES:
			if (iss >> value) {
				sf.health_flask_uses = value;
			}
			break;
		case SAVEFILE_LINES::SWORD_DAMAGE:
#ifdef GOD_MODE
			sf.sword_damage = 999;
#else
			if (iss >> value) {
				sf.sword_damage = value;
			}
#endif
			break;
		case SAVEFILE_LINES::HEART_POWER_UP_0:
			if (iss >> std::boolalpha >> value_b) {
				sf.heart_power_up_0 = value_b;
			}
			break;
		case SAVEFILE_LINES::HEART_POWER_UP_1:
			if (iss >> std::boolalpha >> value_b) {
				sf.heart_power_up_1 = value_b;
			}
			break;
		case SAVEFILE_LINES::SWORD_POWER_UP_0:
			if (iss >> std::boolalpha >> value_b) {
				sf.sword_power_up_0 = value_b;
			}
			break;
		case SAVEFILE_LINES::IS_CHICKEN_DEAD:
			if (iss >> std::boolalpha >> value_b) {
				sf.is_chicken_dead = value_b;
			}
			break;
		case SAVEFILE_LINES::START_FROM_CHECKPOINT:
			if (iss >> std::boolalpha >> value_b) {
				sf.start_from_checkpoint = value_b;
			}
			break;
		case SAVEFILE_LINES::SAVED_THIS_INSTANCE:
			if (iss >> std::boolalpha >> value_b) {
				sf.saved_this_instance = value_b;
			}
			break;

		}
		currentLine++;
	}
}

void clearSaveData() {
	std::fstream fs;
	fs.open(SAVE_FILE_PATH, std::ios::out);
	if (!fs.is_open()) {
		std::error_code ec(errno, std::generic_category());
		std::cerr << "Error: Failed to open file at " << SAVE_FILE_PATH << ". Reason: "
			<< ec.message() << std::endl;
	}
	fs.close();
}