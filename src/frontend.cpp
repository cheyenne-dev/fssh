#include <yaml-cpp/yaml.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
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
		std::cout << "\033[33m[FSSH CONN] netcat not found. Please install it to see status of SSH port.\n" << std::endl;
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
		std::cout << "\033[38;5;" << getColor(line.color) << "m" << line.name << " \033[90m- \033[97m" << line.ip << "\033[90m:\033[96m" << line.port << " \033[90m| \033[96mTCP\033[90m: " << tcpResult[i] << "\033[0m" << std::endl;
		i++;
	}

	std::string profileName;

	std::cout << "\n[FSSH CONN] Enter profile name: ";
	std::cin >> profileName;

	if (profileName == "exit") {
		return "\033[33m[FSSH CONN] Interrupted by user.";
	}

	auto check = std::find_if(profileList.begin(), profileList.end(), [profileName](const report& r) {
		return r.name == profileName;
	});

	if (check != profileList.end()) {
		std::cout << "Connecting to " << profileName << "..." << std::endl;
		return establishConn(profileName);
	}
	else { 
		//return "\033[31m[FSSH CONN] ERROR: Can't read '" + profileName + "' profile in '~/.config/fssh/config.yaml'.\033[0m\n";
		errorInfo error = getError(error::connProfileFail, "[FSSH CONN] ", profileName);
		return error.message + "\n" + error.hint;
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
			return "\033[33m[FSSH EDIT] Interrupted by user.";
		}

		auto check = std::find_if(profileList.begin(), profileList.end(), [editName](const report& r) {
			return r.name == editName;
		});

		if (check != profileList.end()) {	
			std::cout << "\n[FSSH EDIT] Modifying profile '" << editName << "'." <<std::endl;
			break;
		}	
		else {
			error = "\033[31m[FSSH EDIT] ERROR: invalid name '" + editName + "'\n";
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
			return "\033[31m[FSSH EDIT] ERROR: New profile name already exists in config. Aborting.";
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
		return "\033[32m[FSSH EDIT] '" + editName + "' profile changed successfully.";
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
		return "\033[32m[FSSH EDIT] Added new profile '" + newName + "'.";
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
			return "\033[33m[FSSH EDIT] Interrupted by user.";
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
					return "\033[32m[FSSH EDIT] Profile '" + delName + "' deleted successfully.";
				}
				else {
					return result.error.message + "\n" + result.error.hint;
				}
			}
			else {
				return "\033[31m[FSSH EDIT] ERROR: Names are not the same. Aborted.";	
			}
		}
		else {
			error = "\033[31m[FSSH EDIT] ERROR: invalid name '" + delName + "'\n";
		}
	}
}

const std::vector<std::string> colors = {"red", "green", "yellow", "blue", "magenta", "cyan", "white", "br-red", "br-green", "br-yellow", "br-blue", "br-magenta", "br-cyan", "br-white"};

void colorList() {
	std::cout << "Which color do you prefer?\n\n"
		<< "Available colors:\n"
		<< "\033[0m - \033[38;5;1mred\n"
		<< "\033[0m - \033[38;5;9mbr-red\n"
		<< "\033[0m - \033[38;5;2mgreen\n"
		<< "\033[0m - \033[38;5;10mbr-green\n"
		<< "\033[0m - \033[38;5;3myellow\n"
		<< "\033[0m - \033[38;5;11mbr-yellow\n"
		<< "\033[0m - \033[38;5;4mblue\n"
		<< "\033[0m - \033[38;5;12mbr-blue\n"
		<< "\033[0m - \033[38;5;5mmagenta\n"
		<< "\033[0m - \033[38;5;13mbr-magenta\n"
		<< "\033[0m - \033[38;5;6mcyan\n"
		<< "\033[0m - \033[38;5;14mbr-cyan\n"
		<< "\033[0m - \033[38;5;7mwhite\n"
		<< "\033[0m - \033[38;5;15mbr-white\n"
		<< "\033[0m - \033[38;5;8mwhite\n\n"
		<< "\033[0m 0 Exit\n\n"
		<< "\033[0mYour choice : ";
}

//LAUNCHER PREFERENCES EDITOR
std::string editMenu_mainPage(std::string message) { //WORK IN PROGRESS
	while (true) {
		system("clear");
        	logo();

        	if (!message.empty()) {
        		std::cout << "" << message << "\033[0m\n\n";
        	}

        	std::cout << "Choose option:\n\n"
             	     	  << "┌ 1 - Change logo colors\n"
			  << "└ 2 - Change logo style\n\n"
			  << "  3 - Change profile color in list\n\n"
		     	  << "  0 - Exit.\n\n" 
             	     	  << "Your choice: ";
		
		std::string answer;
		std::cin >> answer;
		
		//LOGO EDIT
		if (answer == "1") {
			message = editMenu_logoColor();
		}
		else if (answer == "2") {
			message = editMenu_logoStyle();
		}
		else if (answer == "3") {
			message = editMenu_profileColor();
		}
		else if (answer == "0" || answer == "exit") {
			return "\033[33m[FSSH MENU] Interrupted by user.";
		}
		else {
			message = "\033[31m[FSSH MENU] ERROR: Incorrect option '" + answer + "'.";
		}
	}
}
std::string editMenu_logoColor(std::string message) {
	while (true) {
		system("clear");
        	logo();

		if (!message.empty()) {
        		std::cout << "" << message << "\033[0m\n\n";
        	}

		std::cout << "Choose what to change:\n\n"
             	  	<< "┌ 1 - Background color\n"
		  	<< "└ 2 - Letters color\n\n"
		  	<< "  0 - Exit.\n\n" 
             	  	<< "Your choice: ";

			std::string answer;
			std::cin >> answer;

			if (answer == "1" || answer == "2") {
				while (true) {
					system("clear");
        				logo();

					if (!message.empty()) {
        					std::cout << "" << message << "\033[0m\n\n";
        				}
					
					colorList();
						
					std::string answerOld = answer;
					std::string answer;
					std::cin >> answer;

					if (answer == "0" || answer == "exit") {
						return "\033[33m[FSSH MENU] Interrupted by user.";
					}


					auto check = std::find_if(colors.begin(), colors.end(), [&answer](const std::string color){
						return boost::iequals(color, answer);
					});

					if (check != colors.end()) {
						if (answerOld == "1") {
							editArgs args; args.color = boost::algorithm::to_lower_copy(answer);
							message = editLauncherConfig(editType::logoBgColor, args);
							break;
						}
						else {
							editArgs args; args.color = boost::algorithm::to_lower_copy(answer);
							message = editLauncherConfig(editType::logoFontColor, args);
							break;
						}
					}
					else {
						message = "\033[31m[FSSH MENU] ERROR: Color '" + answer + "' doesn't exist.";
					}
				}
			}	
			else if (answer == "0" || answer == "exit") {
				return "\033[33m[FSSH MENU] Interrupted by user.";
			}
			else {
				message = "\033[31m[FSSH MENU] ERROR: Incorrect option '" + answer + "'.";
			}
		}
}
std::string editMenu_logoStyle(std::string message) {
	while (true) {
		system("clear");
        	logo();

		std::vector<std::string> styles = {"solid", "lines"};

		if (!message.empty()) {
        		std::cout << "" << message << "\033[0m\n\n";
        	}
		std::cout << "Which style do you prefer?\n\n"
			<< "Available styles:\n"
			<< " - Solid\n"
			<< " - Lines\n\n"
			<< " 0 Exit\n\n"
			<< "Your choice : ";

		std::string answer;
		std::cin >> answer;

		if (answer == "0" || answer == "exit") {
			return "\033[33m[FSSH MENU] Interrupted by user.";
		}

		auto check = std::find_if(styles.begin(), styles.end(), [&answer](const std::string color){
			return boost::iequals(color, answer);
		});

		if (check != styles.end()) {
			editArgs args; args.style = boost::algorithm::to_lower_copy(answer);
			return editLauncherConfig(editType::logoStyle, args);
		}
		else {
			return "\033[31m[FSSH EDIT] ERROR: Style you entered doesn't exist.";
		}
	}
}
std::string editMenu_profileColor (std::string message) {
	while (true) {
		system("clear");
        	logo();

		std::vector<report> profileList = readConfig();

		if (!message.empty()) {
        		std::cout << "" << message << "\033[0m\n\n";
        	}
		std::cout << "Which profile color you want to modify?\n\n";\

		for (auto& line : profileList) {
			std::cout << "\033[0m - \033[38;5;" << getColor(line.color) << "m" << line.name << "\n";	
		}

		std::cout << "\n 0 Exit\n\n"
			<< "Your choise: ";

		std::string answer;
		std::cin >> answer;

		if (answer == "0" || answer == "exit") {
			return "\033[33m[FSSH MENU] Interrupted by user.";
		}

		auto check = std::find_if(profileList.begin(), profileList.end(), [answer](const report& r) {
			return r.name == answer;
		});

		if (check != profileList.end()) {
			while (true) {
				system("clear");
        			logo();

				if (!message.empty()) {
        				std::cout << "" << message << "\n\n";
        			}

				colorList();

				std::string answerOld = answer;
				std::string answer;
				std::cin >> answer;

				if (answer == "0" || answer == "exit") {
					return "\033[33m[FSSH MENU] Interrupted by user.";
				}


				auto check = std::find_if(colors.begin(), colors.end(), [&answer](const std::string color){
					return boost::iequals(color, answer);
				});

				if (check != colors.end()) {
					editArgs args; 
					args.color = boost::algorithm::to_lower_copy(answer);
					args.name = boost::algorithm::to_lower_copy(answerOld);

					message = editLauncherConfig(editType::profileColor, args);
					break;
				}
				else {
					message = "\033[31m[FSSH MENU] ERROR: Color '" + answer + "' doesn't exist.";
				}
			}
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
        		std::cout << "" << message << "\033[0m\n\n";
        	}

        	std::cout << "Choose option:\n\n"
             	     << "┌ 1 - Establish connection;\n"
		     << "│ 2 - Edit existing profile;\n"
            	     << "└ 3 - Add new profile;\n\n"
             	     << "┌ 4 - Generate key pair (for all profiles);\n"
		     << "└ 5 - empty entry;\n\n"
            	     << "┌ 7 - Delete existing profile;\n"
        	     << "└ 8 - Rebuild config from scratch;\n\n"
		     << "  9 - Change launcher preferences;\n\n"
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
			message = editMenu_mainPage();
		}

		else if (answer == "0" || answer == "exit") {
        	    	break; 
        	}
		else {
			message = "\033[31m[FSSH MENU] Invalid option '" + answer + "'. Use one of the following.";
		}
    	}
}

