#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>

#include "launcher.hpp"

std::string LOGO_TYPE = "solid";

static std::string_view l1 = COLORS.l1; 
static std::string_view l2 = COLORS.l2; 
static std::string_view l3 = COLORS.l3; 

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
	return "7";
}

globalColors loadLauncherConfig() {
	std::ofstream(LAUNCHER_CONFIG_PATH, std::ios::app).close();
	try {
		YAML::Node launcherConfig = YAML::LoadFile(LAUNCHER_CONFIG_PATH);

		std::string color_bg = launcherConfig["colors"]["color-logo-bg"].as<std::string>();
		std::string color_font = launcherConfig["colors"]["color-logo-font"].as<std::string>();

		std::string c1 = "\033[3" + getColor(color_bg) + "m";
		std::string c2 = "\033[4" + getColor(color_bg) + "m\033[3" + getColor(color_font) + "m";
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
	std::cout << "\033[34m░░▒▒▓▓\033[44m\033[37m   ______ _____ _____ _    _   \033[0m\033[34m▓▓▒▒░░ \n" 
	     	  << "\033[34m░░▒▒▓▓\033[44m\033[37m  |  ____/ ____/ ____| |  | |  \033[0m\033[34m▓▓▒▒░░ \n" 
	     	  << "\033[34m░░▒▒▓▓\033[44m\033[37m  | |__ | (___| (___ | |__| |  \033[0m\033[34m▓▓▒▒░░ \n"
	     	  << "\033[34m░░▒▒▓▓\033[44m\033[37m  |  __| \\___ \\\\___ \\|  __  |  \033[0m\033[34m▓▓▒▒░░ \n"
	     	  << "\033[34m░░▒▒▓▓\033[44m\033[37m  | |    ____) |___) | |  | |  \033[0m\033[34m▓▓▒▒░░ \n"
	     	  << "\033[34m░░▒▒▓▓\033[44m\033[37m  |_|   |_____/_____/|_|  |_|  \033[0m\033[34m▓▓▒▒░░ \n"
	     	  << "\033[34m░░▒▒▓▓\033[44m\033[37m                               \033[0m\033[34m▓▓▒▒░░ \n"
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

