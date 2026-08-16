#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

std::string getHomeDir();
inline const std::string HOME_PATH = getHomeDir();
inline const std::string LAUNCHER_CONFIG_PATH = HOME_PATH + "/.config/fssh/launcher.yaml";

inline const std::string VERSION = "v0.3.2-lacombe";

#endif
