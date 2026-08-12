#include "src/backend.hpp"
#include "src/frontend.hpp"
#include <algorithm>
#include <iostream>

//COMMAND HANDLER
std::string commandHandler(std::string profile) {
	std::vector<report> profileList = readConfig();
		
	auto check = std::find_if(profileList.begin(), profileList.end(), [profile](const report& r){
		return r.name == profile;
	});
	if (check != profileList.end()) {
		return establishConn(profile);
	}
	else {
		return "[FSSH] Profile '" + profile + "' doesn't exist.";
	}
}

//MAIN THREAD
int main(int argc, char* argv[]) {
	if (argc == 1) {
		if (!checkConfig()) {
			newProfile();	
		}
		mainMenu();
		return 0;
	}
	else {
		if (argc == 2) {
			std::cout << commandHandler(argv[1]);
			return 0;
		}
		else {
			std::cerr << "[FSSH] Too many arguments." << std::endl;
			return 1;
		}
	}
}
