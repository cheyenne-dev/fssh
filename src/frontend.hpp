#ifndef FRONTEND_HPP
#define FRONTEND_HPP

#include <string>

//OPERATIONS WITH PROFILES
std::string chooseProfile();
std::string editProfile();
std::string newProfile ();
std::string deleteProfile();

//LAUNCHER PREFERENCES EDITOR
std::string editMenu_mainPage(std::string message = "[FSSH MENU] Errors or notifications will appear here.");
std::string editMenu_logoColor(std::string message = "[FSSH MENU] Errors or notifications will appear here.");
std::string editMenu_logoStyle(std::string message = "[FSSH MENU] Errors or notifications will appear here.");


//MAIN MENU
void mainMenu(std::string message = "[FSSH MENU] Errors or notifications will appear here.");

#endif
