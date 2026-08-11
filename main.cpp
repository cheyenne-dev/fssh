#include "src/backend.hpp"
#include "src/frontend.hpp"
#include <algorithm>
#include <iostream>

//COMMAND HANDLER
int commandHandler(std::string profile) {
	std::vector<report> profileList = readConfig();
		
	auto check = std::find_if(profileList.begin(), profileList.end(), [profile](const report& r){
		return r.name == profile;
	});
	if (check != profileList.end()) {
		establishConn(profile);
	}
	else {
		std::cerr << "[FSSH] Profile '" + profile + "' doesn't exist." << std::endl;
	}
	return 0;
}

//MAIN THREAD
int main(int argc, char* argv[]) {
	if (argc == 1) {
		checkConfig();
		mainMenu();
		return 0;
	}
	else {
		if (argc == 2) {
			return commandHandler(argv[1]);
		}
		else {
			std::cerr << "[FSSH] Too many arguments." << std::endl;
			return 1;
		}
	}
}
