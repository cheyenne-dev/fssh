#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "launcher.hpp"
#include "backend.hpp"

std::string LOGO_TYPE = "solid";

//static std::string_view l1 = COLORS.l1; 
//static std::string_view l2 = COLORS.l2; 
//static std::string_view l3 = COLORS.l3; 

globalColors COLORS = loadLauncherConfig();

std::string l1 = COLORS.l1; 
std::string l2 = COLORS.l2; 
std::string l3 = COLORS.l3; 

std::string getColor(std::string code) {
	if (code == "red") {
		return "1";
	}
	else if (code == "green") {
		return "2";
	}
	else if (code == "yellow") {
		return "3";
	}
	else if (code == "blue") {
		return "4";
	}
	else if (code == "magenta") {
		return "5";
	}
	else if (code == "cyan") {
		return "6";
	}
	else if (code == "white") {
		return "7";
	}
	return "0";
}

operationResult checkLauncherConfig() {
	try {
		YAML::Node config = YAML::LoadFile(LAUNCHER_CONFIG_PATH);
		return {true, {}}; 

	} catch (const YAML::Exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::filesystem::create_directories(HOME_PATH + "/.config/fssh");
		std::ofstream(LAUNCHER_CONFIG_PATH, std::ios::app).close();
		return {false, getError(error::launcherConfigFail, "[FSSH MENU] ")};
	}
}

std::string editLauncherConfig(editType type, editArgs args) {
	YAML::Node launcherConfig = YAML::LoadFile(LAUNCHER_CONFIG_PATH);
	std::string message;

	switch (type) {
		case editType::logoBgColor:
			launcherConfig["logo"]["color-logo-bg"] = args.color;
			message = "[FSSH MENU] Logo background color changed to '" + args.color + "'.";
			break;
		case editType::logoFontColor:
			launcherConfig["logo"]["color-logo-font"] = args.color;
			message = "[FSSH MENU] Logo font color changed to '" + args.color + "'.";
			break;
		case editType::logoStyle:
			launcherConfig["logo"]["style"] = args.style;
			message = "[FSSH MENU] Logo style changed to '" + args.style + "'.";
			break;
	}

	std::ofstream fout(LAUNCHER_CONFIG_PATH);
	fout << launcherConfig;
	fout.close();

	COLORS = loadLauncherConfig();

	l1 = COLORS.l1; 
	l2 = COLORS.l2; 
	l3 = COLORS.l3;

	return message;
}	

globalColors loadLauncherConfig() {
	operationResult checkResult = checkLauncherConfig();
	if (!checkResult.success) {
		return {"\033[34m",
			"\033[44m\033[37m",
			"\033[0m\033[34m"};
	}

	try {
		//std::ofstream(LAUNCHER_CONFIG_PATH, std::ios::app).close();
		YAML::Node launcherConfig = YAML::LoadFile(LAUNCHER_CONFIG_PATH);

		std::string colorBg = launcherConfig["logo"]["color-logo-bg"].as<std::string>("blue");
		std::string colorFont = launcherConfig["logo"]["color-logo-font"].as<std::string>("white");

		LOGO_TYPE = launcherConfig["logo"]["style"].as<std::string>("solid");

		std::string c1 = "\033[3" + getColor(colorBg) + "m";
		std::string c2 = "\033[4" + getColor(colorBg) + "m\033[3" + getColor(colorFont) + "m";
		std::string c3 = "\033[0m" + c1;

		return {c1,
			c2,
			c3};
		
	} catch (const YAML::Exception& e) {
		return {"\033[34m",
			"\033[44m\033[37m",
			"\033[0m\033[34m"};
	}
}

void logoLINES() {
	std::cout << l1 << "░░▒▒▓▓" << l2 << "   ______ _____ _____ _    _   " << l3 << "▓▓▒▒░░ \n" 
	     	  << l1 << "░░▒▒▓▓" << l2 << "  |  ____/ ____/ ____| |  | |  " << l3 << "▓▓▒▒░░ \n" 
	     	  << l1 << "░░▒▒▓▓" << l2 << "  | |__ | (___| (___ | |__| |  " << l3 << "▓▓▒▒░░ \n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "  |  __| \\___ \\\\___ \\|  __  |  " << l3 << "▓▓▒▒░░ \n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "  | |    ____) |___) | |  | |  " << l3 << "▓▓▒▒░░ \n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "  |_|   |_____/_____/|_|  |_|  " << l3 << "▓▓▒▒░░ \n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "                               " << l3 << "▓▓▒▒░░ \n"
	     	  << "\n"
	     	  << "\033[37m            Fluent Secure Shell            \n"
	       	  << "\033[90m              " << VERSION << "               \033[0m\n"
	     	  << "                                     \n";
}

void logoSOLID() {
	std::cout << l1 << "░░▒▒▓▓" << l2 << "                               " << l3 << "▓▓▒▒░░\n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "  ██████ ██████ ██████ ██  ██  " << l3 << "▓▓▒▒░░\n" 
	     	  << l1 << "░░▒▒▓▓" << l2 << "  ██     ██     ██     ██  ██  " << l3 << "▓▓▒▒░░\n" 
	     	  << l1 << "░░▒▒▓▓" << l2 << "  ██████ ██████ ██████ ██████  " << l3 << "▓▓▒▒░░\n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "  ██         ██     ██ ██  ██  " << l3 << "▓▓▒▒░░\n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "  ██     ██████ ██████ ██  ██  " << l3 << "▓▓▒▒░░\n"
	     	  << l1 << "░░▒▒▓▓" << l2 << "                               " << l3 << "▓▓▒▒░░\n"
	     	  << "\n"
	     	  << "\033[37m            Fluent Secure Shell            \n"
		  << "\033[90m              " << VERSION << "               \033[0m\n"
	     	  << "                                     \n";
}

void logo() {
	if (LOGO_TYPE == "solid") {
		logoSOLID();
	}
	else {
		logoLINES();
	}
}

