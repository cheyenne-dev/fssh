#include <yaml-cpp/yaml.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <limits>
#include <filesystem>

#include "backend.hpp"
#include "frontend.hpp"
#include "launcher.hpp"

//MENU: ESTABLISH CONNECTION
std::string chooseProfile() {
	checkConfig();

	system("clear");
	logo();
	
	std::vector<report> profileList = readConfig();

	boost::filesystem::path ncPath = boost::process::environment::find_executable("nc");
	std::vector<std::string> tcpResult(profileList.size());

	if (ncPath.empty()) {
		std::cout << "[FSSH CONN] netcat not found. Please install it to see status of SSH port.\n" << std::endl;
		for (size_t i = 0; i < profileList.size(); i++) {
			tcpResult[i] = "[N/A]";
		}
	}
	else {
		std::cout << "[FSSH CONN] Checking SSH ports status. Please wait...\n" << std::endl;
		tcpResult = checkHosts(profileList, ncPath);
	}

	int i = 0;
	for (auto& line : profileList) {
		std::cout << line.name << " - " << line.ip << ":" << line.port << " | TCP: " << tcpResult[i] << std::endl;
		i++;
	}

	std::string profileName;

	std::cout << "\n[FSSH CONN] Enter profile name: ";
	std::cin >> profileName;

	auto check = std::find_if(profileList.begin(), profileList.end(), [profileName](const report& r) {
		return r.name == profileName;
	});

	if (check != profileList.end()) {
		std::cout << "Connecting to " << profileName << "..." << std::endl;
		return establishConn(profileName);
	}
	else {
		return "\033[31m[FSSH CONN] ERROR: Can't read '" + profileName + "' profile in '~/.config/fssh/config.yaml'.\033[0m\n";
	}
}

//MENU: EDIT EXISTING PROFILE
std::string editProfile() {
	std::vector<report> profileList = readConfig();
	std::string editName;
	std::string error;

	while (true) {
		system("clear");
		logo();
	
		std::cout << error << "\n[FSSH EDIT] Existing profiles:" << std::endl;
	
		int i = 1;
		for (auto& line : profileList) {
			std::cout << i << " - " << line.name << ": IP=" << line.ip << " PORT=" << line.port << " USER=" << line.user << std::endl;
			i++;
		}
	
		std::cout << "[FSSH EDIT] Which profile you want to edit: ";
		std::cin >> editName;

		if (editName == "exit") {
			return "[FSSH EDIT] Interrupted by user.";
		}

		auto check = std::find_if(profileList.begin(), profileList.end(), [editName](const report& r) {
			return r.name == editName;
		});

		if (check != profileList.end()) {	
			std::cout << "\n[FSSH EDIT] Modifying profile '" << editName << "'." <<std::endl;
			break;
		}	
		else {
			error = "[FSSH EDIT] ERROR: invalid name '" + editName + "'\n";
		}
	}

	report oldInfo;

	for (auto& line : profileList) {
		if (line.name == editName) {
			oldInfo.name = line.name;
			oldInfo.ip = line.ip;
			oldInfo.port = line.port;
			oldInfo.user = line.user;
		}
	}

	std::string newName;
	std::string newAddr;
	std::string newPort;
	std::string newUser;


	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << "Edit name (default: '" + oldInfo.name + "'): ";
	std::getline(std::cin, newName);
	//CHECK FOR REWRITE
	if (newName != editName && !newName.empty()) {
		auto check = std::find_if(profileList.begin(), profileList.end(), [newName](const report& r) {
			return r.name == newName;
		});
		if (check != profileList.end()){
			return "[FSSH EDIT] ERROR: New profile name already exists in config. Aborting.";
		}
	}
	std::cout << "Edit address (default: '" + oldInfo.ip + "'): ";
	std::getline(std::cin, newAddr);
	std::cout << "Edit port (default: '" + oldInfo.port + "'): ";
	std::getline(std::cin, newPort);
	std::cout << "Edit username (default: '" + oldInfo.user + "'): ";
	std::getline(std::cin, newUser);
	
	operationResult result = editConn(editName, newName, newAddr, newPort, newUser);
	if (result.success) {
		return "[FSSH EDIT] '" + editName + "' profile changed successfully.";
	}
	else {
		return result.error.message + "\n" + result.error.hint;
	}
}

//MENU: ADD NEW PROFILE
std::string newProfile () {
	std::string newName; std::string newAddr; std::string newPort; std::string newUser;

	system("clear");
	logo();

	std::cout << "[FSSH EDIT] Adding new profile. Spaces are NOT allowed!\n"
	     << "[FSSH EDIT] Enter new name: ";
	std::cin >> newName;
	std::cout << "[FSSH EDIT] Enter new address: ";
	std::cin >> newAddr;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << "[FSSH EDIT] Enter new port (default: 22): ";
	std::getline(std::cin, newPort);
	if (newPort.empty()) {
		newPort = "22";
	}
	std::cout << "[FSSH EDIT] Enter new user: ";
	std::cin >> newUser;

	operationResult result = newConn(newName, newAddr, newPort, newUser);

	if (result.success) {
		return "[FSSH EDIT] Added new profile '" + newName + "'.";
	}
	else {
		return result.error.message + "\n" + result.error.hint;
	}

}

//MENU: DELETE PROFILE
std::string deleteProfile() {
	std::string error;

	while (true) {
		system("clear");
		logo();

		std::vector<report> profileList = readConfig();
			
		std::cout << error << "\n\nExisting profiles:" << std::endl;
	
		int i = 1;
		for (auto& line : profileList) {
			std::cout << i << " - " << line.name << ": IP=" << line.ip << " PORT=" << line.port << " USER=" << line.user << std::endl;
			i++;
		}
	
		std::cout << "[FSSH EDIT] Which profile do you want to delete: ";
		std::string delName;
		std::cin >> delName;

		if (delName == "exit") {
			return "[FSSH EDIT] Interrupted by user.";
		}

			auto check = std::find_if(profileList.begin(), profileList.end(), [delName](const report& r) {
				return r.name == delName;
			});

		if (check != profileList.end()) {	
			std::cout << "\n[FSSH EDIT] Waiting confirmation for " << delName << "\nTo confirm deletion, type the profile name again"<<std::endl;
			std::string confirmation;
			std::cin >> confirmation;
			if (confirmation == delName) {
				operationResult result = delConn(delName);
				if (result.success) {
					return "[FSSH EDIT] Profile '" + delName + "' deleted successfully.";
				}
				else {
					return result.error.message + "\n" + result.error.hint;
				}
			}
			else {
				return "[FSSH EDIT] ERROR: Names are not the same. Aborted.";	
			}
		}
		else {
			error = "[FSSH EDIT] ERROR: invalid name '" + delName + "'\n";
		}
	}
}

//MAIN MENU
void mainMenu(std::string message) {	
	bool firstRun = true; 

	while (true) {
		system("clear");
        	logo();

        	if (!message.empty()) {
        		std::cout << "" << message << "\n\n";
        	}

        	std::cout << "Choose option:\n\n"
             	     << "┌ 1 - Establish connection;\n"
		     << "│ 2 - Edit existing profile;\n"
            	     << "└ 3 - Add new profile;\n\n"
             	     << "┌ 4 - Generate key pair (for all profiles);\n"
		     << "└ 5 - empty entry;\n\n"
            	     << "┌ 7 - Delete existing profile;\n"
        	     << "│ 8 - Rebuild config from scratch;\n"
		     << "└ 9 - Change launcher preferences;\n\n"
		     << "  0 - Exit.\n\n" 
             	     << "Your choice (default: 1): ";

        	std::string answer;
        
        	if (!firstRun) {
            		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        	}
        	firstRun = false;

        	std::getline(std::cin, answer);
		
		if (answer.empty() || answer == "1") {
           		message = chooseProfile();
		}
		else if (answer == "2") {
            		message = editProfile();
        	}
		else if (answer == "3") {
            		message = newProfile(); 
        	}
		else if (answer == "4") {
            		makeKeyPair();
            		message = "";
        	}
		else if (answer == "7") {
            		message = deleteProfile();
        	}
		else if (answer == "8") {
			std::filesystem::remove(CONFIG_PATH);
           		message = newProfile();
        	}
		else if (answer == "9") {
			std::filesystem::remove(CONFIG_PATH);
           		message = newProfile();
        	}

		else if (answer == "0" || answer == "exit") {
        	    	break; 
        	}
		else {
			message = "[FSSH MENU] Invalid option '" + answer + "'. Use one of the following.";
		}
    	}
}

