#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

std::string getHomeDir();
inline const std::string HOME_PATH = getHomeDir();

inline const std::string VERSION = "v0.3.1-lacombe";

#endif
