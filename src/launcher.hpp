#ifndef LAUNCHER_HPP
#define LAUNCHER_HPP

#include <string>

#include "constants.hpp"

inline const std::string LAUNCHER_CONFIG_PATH = HOME_PATH + "/.config/fssh/launcher.yaml";

struct globalColors {
	std::string l1;
	std::string l2;
	std::string l3;
};

std::string getColor(std::string code);

globalColors loadLauncherConfig();

inline const globalColors COLORS = loadLauncherConfig();

//LOGO RENDER
void logoLINES();
void logoSOLID();
void logo();

#endif
