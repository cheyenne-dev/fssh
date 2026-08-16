#ifndef FRONTEND_HPP
#define FRONTEND_HPP

#include <string>

//OPERATIONS WITH PROFILES
std::string chooseProfile();
std::string editProfile();
std::string newProfile ();
std::string deleteProfile();

void colorList();

//LAUNCHER PREFERENCES EDITOR
std::string editMenu_mainPage(std::string message = "\033[90m[FSSH MENU] Errors or notifications will appear here.\033[0m");
std::string editMenu_logoColor(std::string message = "\033[90m[FSSH MENU] Errors or notifications will appear here.\033[0m");
std::string editMenu_logoStyle(std::string message = "\033[90m[FSSH MENU] Errors or notifications will appear here.\033[0m");
std::string editMenu_profileColor(std::string message = "\033[90m[FSSH MENU] Errors or notifications will appear here.\033[0m");


//MAIN MENU
void mainMenu(std::string message = "\033[90m[FSSH MENU] Errors or notifications will appear here.\033[0m");

#endif
