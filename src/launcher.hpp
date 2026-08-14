#ifndef LAUNCHER_HPP
#define LAUNCHER_HPP

#include <string>

#include "constants.hpp"
#include "backend.hpp"

inline const std::string LAUNCHER_CONFIG_PATH = HOME_PATH + "/.config/fssh/launcher.yaml";

enum class editType {
	logoBgColor,
	logoFontColor,
	logoStyle
};

struct globalColors {
	std::string l1;
	std::string l2;
	std::string l3;
};

struct editArgs {
	std::string color;
	std::string type;
	std::string style;
	std::string str;
};

std::string getColor(std::string code);

operationResult checkLauncherConfig();
std::string editLauncherConfig(editType type, editArgs args);
globalColors loadLauncherConfig();

//inline globalColors COLORS = loadLauncherConfig();

//LOGO RENDER
void logoLINES();
void logoSOLID();
void logo();

#endif
