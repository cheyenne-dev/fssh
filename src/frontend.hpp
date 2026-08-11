#ifndef FRONTEND_HPP
#define FRONTEND_HPP

#include <string>

//LOGO RENDER
void logoLines();
void logoSOLID();
void logo();

//OPERATIONS WITH PROFILES
std::string chooseProfile();
std::string editProfile();
std::string newProfile ();
std::string deleteProfile();

//MAIN MENU
void mainMenu(std::string message = "[FSSH MENU] Errors or notifications will appear here.");

#endif
