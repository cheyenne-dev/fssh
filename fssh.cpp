#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <sstream>
#include <limits>

void logo();

std::string getHomeDir() {
	const std::string homeEnv = getenv("HOME");
	return homeEnv;
}

const std::string HOME_PATH = getHomeDir();
const std::string CONFIG_PATH = HOME_PATH + "/.config/fssh/config.yaml";

//SETTING UP NEW CONNECTION

std::string printError(std::string context) {
	if (context == "connProfileFail") {
		return "";
	}

	return "[FSSH ERROR] Unexpected error. Sorry, I can't help you. You are on your own. ";
}

std::string newConn() {
	system("clear");

	std::string command = "touch " + CONFIG_PATH; //FIX THIS!!!!!!! COMMAND INJECTION!!!

	system(command.c_str());

	YAML::Node config = YAML::LoadFile(CONFIG_PATH);
	
	std::string newAddr;
	std::string newName;
	std::string newPort;
	std::string newUser;

	//Setting up
	
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

	config[newName]["ipAddr"] = newAddr;
	config[newName]["port"] = newPort;
	config[newName]["username"] = newUser;

	std::ofstream fout(CONFIG_PATH);
	fout << config;
	fout.close();

	//string answer;
	//cout << "Do you want to add another device? (y/n) ";
	//cin >> answer;
	//if (answer == "y") {
	//	newConn();
	//}

	return newName;
}

void editConn (std::string editName) {
	YAML::Node config = YAML::LoadFile(CONFIG_PATH);
	
	std::string newAddr;
	std::string newName;
	std::string newPort;
	std::string newUser;

	//Setting up
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << "Edit name (default: " << editName << "): ";
	std::getline(std::cin, newName);
	if (newName.empty()) {
		newName = editName;
	}
	else {
		config[newName] = YAML::Clone(config[editName]);

		//config[newName]["ipAddr"] = config[editName]["ipAddr"].as<string>();
		//config[newName]["port"] = config[editName]["port"].as<string>();
		//config[newName]["username"] = config[editName]["username"].as<string>();
		
	}
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');
	std::cout << "Edit address (default: " << config[editName]["ipAddr"].as<std::string>() << "): ";
	std::getline(std::cin, newAddr);
	if (!newAddr.empty()) {
		config[newName]["ipAddr"] = newAddr;
	}
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');
	std::cout << "Edit port (default: " << config[editName]["port"].as<std::string>() << "): ";
	std::getline(std::cin, newPort);
	if (!newPort.empty()) {
		config[newName]["port"] = newPort;
	}
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');
	std::cout << "Edit username (default: " << config[editName]["username"].as<std::string>() << "): ";
	std::getline(std::cin, newUser);
	if (!newUser.empty()) {
        	config[newName]["username"] = newUser;	
	}

	if (newName != editName) {
		config.remove(editName);
	}

	std::ofstream fout(CONFIG_PATH);
	fout << config;
	fout.close();
}

void delConn(std::string delName) {
	YAML::Node config = YAML::LoadFile(CONFIG_PATH);

	config.remove(delName);

	std::ofstream fout(CONFIG_PATH);
	fout << config;
	fout.close();
}

std::vector<std::string> checkHosts(const std::vector<std::string>& ip, const std::vector<std::string>& ports) {
	boost::filesystem::path ncPath = boost::process::environment::find_executable("nc");

	std::vector<std::string> result;
	result.resize(ip.size());

	if (ncPath.empty()) {
	//if (true) {	SKIPS TCP TEST 
		std::cout << "[FSSH CONN] netcat not found. Please install it to see status of SSH port.\n" << std::endl;
		for (size_t i = 0; i < ip.size(); i++) {
			result[i] = "[N/A]";
		}
		return result;
	}
	else {
		std::cout << "[FSSH CONN] Checking SSH ports status. Please wait...\n" << std::endl;
	}

	std::vector<boost::process::process> processes;
	processes.reserve(ip.size());

	boost::asio::io_context ctx;

	for (size_t i = 0; i < ip.size(); i++) {
		processes.push_back(boost::process::process(ctx, ncPath ,{"-zv", "-w", "1", ip[i], ports[i]}, boost::process::process_stdio{nullptr, nullptr, nullptr}));
	}

	for (size_t i = 0; i < ip.size(); i++) {
		processes[i].wait();

		if (processes[i].exit_code() == 0) {
			result[i] = "[ONLINE]";
		}
		else {
			result[i] = "[UNREACHABLE]";
		}
	}
	return result;
}



//READING CONFIG.YAML
std::vector<std::string> readConfig(std::string reportType) {
	std::vector<std::string> report;
	
	try {
			YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		if (config.IsMap()) {
			if (reportType == "ONLY_ROOT") {
				for (auto const& rootElement : config) {
					std::string deviceName = rootElement.first.as<std::string>();
					report.push_back(deviceName);
				}
			}
			else if (reportType == "FULL") {
				for (auto const& rootElement : config) {
					std::string reportString;
					std::string deviceName = rootElement.first.as<std::string>();

					std::string ipAddr = config[deviceName]["ipAddr"].as<std::string>();
					std::string port = config[deviceName]["port"].as<std::string>();
					std::string username = config[deviceName]["username"].as<std::string>();
					
					reportString = deviceName + ": "
						+ " IP=" + ipAddr
						+ " SSH-PORT=" + port
						+ " USERNAME=" + username;
					
					report.push_back(reportString);
				}
			}
			else if (reportType == "INTERACTIVE") {
				std::vector<std::string> ip;
				std::vector<std::string> ports;

				for (auto const& rootElement : config) {
					std::string deviceName = rootElement.first.as<std::string>();
					
					std::string ipAddr = config[deviceName]["ipAddr"].as<std::string>();
					std::string port = config[deviceName]["port"].as<std::string>();

					ip.push_back(config[deviceName]["ipAddr"].as<std::string>());
					ports.push_back(config[deviceName]["port"].as<std::string>());

					std::string reportString = deviceName + " - " + ipAddr + ":" + port + " | TCP: ";

					report.push_back(reportString);
				}

				std::vector<std::string> tcpResult = checkHosts(ip, ports);

				for (size_t i = 0; i < report.size(); i++) {
					report[i] += tcpResult[i];
				}

			}
		}
	} catch (const YAML::Exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return report;
}

//ESTABLISHING CONNECTION
std::string establishConn(std::string deviceName) {

	std::string ipAddr;
	std::string port;
	std::string username;
	
	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		ipAddr = config[deviceName]["ipAddr"].as<std::string>();
		port = config[deviceName]["port"].as<std::string>();
		username = config[deviceName]["username"].as<std::string>();

	} catch (const YAML::Exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return "\033[31m[FSSH CONN] ERROR: Can't read '" + deviceName + "' profile in '~/.config/fssh/config.yaml'.\033[0m\n";
	}

	std::stringstream mkDest;
	mkDest << username << "@" << ipAddr;

	std::string destination = mkDest.str();

	boost::asio::io_context ctx;
	boost::filesystem::path sshPath = boost::process::environment::find_executable("ssh");
	boost::process::process proc(ctx, sshPath, {
				destination,
				"-p",
				port,
			}
	);
	proc.wait();

	return 0;

}

void makeKeyPair() {
	int keygenResult = system("ssh-keygen -t ed25519 -N \"\" -f ~/.ssh/id_ed25519");
	if (keygenResult == 0) {
		std::cout << "[FSSH KEYGEN] Key pair generated successfully!" << std::endl;
	}
	std::string keyLocation = HOME_PATH + "/.ssh/id_ed25519.pub";

	std::vector<std::string> names = readConfig("FULL");
	YAML::Node config = YAML::LoadFile(CONFIG_PATH);

	for (auto& name : names) {
		std::string destination = config[name]["username"].as<std::string>() + "@" + config[name]["ipAddr"].as<std::string>();
		std::string port = config[name]["port"].as<std::string>();

		boost::asio::io_context ctx;
		boost::filesystem::path ssh_copy_idPath = boost::process::environment::find_executable("ssh-copy-id");
		boost::process::process proc(ctx, ssh_copy_idPath, {
					"-i", 
					keyLocation,
					"-p",
					port,
					destination,
					});
		proc.wait();
	}
}

enum class ConfigStatus {
    Valid,
    FileNotFound,
    FileEmpty,
    InvalidYaml,
    NoStructure 
};

//CHECKING CONFIG
std::string LOGO_TYPE = "solid";

bool checkConfig() {

	//if (!filesystem::exists(CONFIG_PATH)) {
	//
	//}

	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);
		//LOGO_TYPE = 

	} catch (const YAML::Exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::string command = "mkdir -p " + HOME_PATH + "/.config/fssh; " + "touch " + CONFIG_PATH;
		system(command.c_str());

		std::cout << "Config is not valid. Creating new file..." << std::endl;
		newConn();
	}

	return true;
}

//   FRONT-END FUNCTIONS
//MENU: ONLY LOGO
void logoLINES() {
	std::cout << "\033[34m░░▒▒▓▓\033[44m\033[37m   ______ _____ _____ _    _   \033[0m\033[34m▓▓▒▒░░ \n" 
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  |  ____/ ____/ ____| |  | |  \033[0m\033[34m▓▓▒▒░░ \n" 
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  | |__ | (___| (___ | |__| |  \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  |  __| \\___ \\\\___ \\|  __  |  \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  | |    ____) |___) | |  | |  \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  |_|   |_____/_____/|_|  |_|  \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m                               \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\n"
	     << "\033[34m        \033[34m░▒▓\033[44m\033[37m Fluent Secure Shell \033[0m\033[34m▓▒░     \033[0m\n"
	     << "\033[90m              v0.2a (cezanne)               \033[0m\n"
	     << "                                     \n";
}

void logoSOLID() {
	std::cout << "\033[34m░░▒▒▓▓\033[44m\033[37m                               \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  ██████ ██████ ██████ ██  ██  \033[0m\033[34m▓▓▒▒░░ \n" 
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  ██     ██     ██     ██  ██  \033[0m\033[34m▓▓▒▒░░ \n" 
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  ██████ ██████ ██████ ██████  \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  ██         ██     ██ ██  ██  \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m  ██     ██████ ██████ ██  ██  \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m                               \033[0m\033[34m▓▓▒▒░░ \n"
	     << "\n"
	     //<< "\033[34m        \033[34m░▒▓\033[44m\033[37m Fluent Secure Shell \033[0m\033[34m▓▒░     \033[0m\n"
	     << "\033[37m            Fluent Secure Shell            \n"
	     << "\033[90m              v0.2a (cezanne)               \033[0m\n"
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

//MENU: CHOOSE CONNECTION PROFILE
std::string chooseConn() {
	checkConfig();

	system("clear");
	logo();
	
	std::vector<std::string> availableNames = readConfig("ONLY_ROOTS");
	std::vector<std::string> output = readConfig("INTERACTIVE");

	for (auto& line : output) {
		std::cout << line << std::endl;
	}

	std::string deviceName;

	std::cout << "\n[FSSH CONN] Enter profile name: ";
	std::cin >> deviceName;

	auto check = std::find(availableNames.begin(), availableNames.end(), deviceName);

	if (check != availableNames.end()) {
		std::cout << "Connecting to " << deviceName << "..." << std::endl;
	}
	return establishConn(deviceName);
}

//MENU: EDIT CONFIG
void editConfig(std::string error = "") {
	system("clear");
	logo();
	
	std::vector<std::string> fullReport = readConfig("FULL");
	std::vector<std::string> availableNames = readConfig("ONLY_ROOTS");
	
	std::cout << "Existing profiles:" << std::endl;

	int i = 1;
	for (auto& line : fullReport) {
		std::cout << i << " - "<< line << std::endl;
		i++;
	}
	
	std::cout << error <<"[FSSH EDIT] Which profile you want to edit: ";
	std::string editName;
	std::cin >> editName;

	if (editName == "exit") {
		return;
	}

	auto check = std::find(availableNames.begin(), availableNames.end(), editName);

	if (check != availableNames.end()) {	
		std::cout << "\n[FSSH EDIT] Modifying " << editName << ". Be careful! Changes are applied IMMEDIATELY!"<<std::endl;
		editConn(editName);
	}
	else {
		error = "[FSSH EDIT] ERROR: invalid name " + editName + "\n";
		editConfig(error);
	}


}

//MENU: DELETE PROFILE
void deleteProfile(std::string error = "") {
	system("clear");
	logo();

	std::vector<std::string> fullReport = readConfig("FULL");
	std::vector<std::string> availableNames = readConfig("ONLY_ROOTS");
	
	std::cout << "Existing profiles:" << std::endl;

	int i = 1;
	for (auto& line : fullReport) {
		std::cout << i << " - "<< line << std::endl;
		i++;
	}
	
	std::cout << error <<"[FSSH EDIT] Which profile do you want to delete: ";
	std::string delName;
	std::cin >> delName;

	if (delName == "exit") {
		return;
	}

	auto check = std::find(availableNames.begin(), availableNames.end(), delName);

	if (check != availableNames.end()) {	
		std::cout << "\n[FSSH EDIT] Waiting confirmation for " << delName << "\nTo confirm deletion, type the profile name again"<<std::endl;
		std::string confirmation;
		std::cin >> confirmation;
		if (confirmation == delName) {
			delConn(delName);
		}
		else {
			error = "[FSSH EDIT] \n";
			deleteProfile(error);	
		}
	}
	else {
		error = "[FSSH EDIT] ERROR: Names are not the same. Aborted. " + delName + "\n";
		deleteProfile(error);
	}


	delConn(delName);
}

//MAIN MENU
void mainMenu(std::string message = "[FSSH MENU] Errors or notifications will appear here.") {	
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
            	     << "┌ 8 - Delete existing profile;\n"
        	     << "└ 9 - Rebuild config from scratch;\n\n"
		     << "  0 - Exit.\n\n" 
             	     << "Your choice (default: 1): ";

        	std::string answer;
        
        	if (!firstRun) {
            	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        	}
        	firstRun = false;

        	std::getline(std::cin, answer);
		
		if (answer.empty() || answer == "1") {
           		message = chooseConn() + "[FSSH CONN] If you are sure that profile you entered exists, try editing it.";
        	}
		else if (answer == "2") {
            		editConfig();
            		message = "";
        	}
		else if (answer == "3") {
            		message = "[FSSH EDIT] Added new profile: " + newConn(); 
        	}
		else if (answer == "4") {
            		makeKeyPair();
            		message = "";
        	}
		else if (answer == "8") {
            		deleteProfile();
            		message = "";
        	}
		else if (answer == "9") {
            		std::string command = "rm " + CONFIG_PATH;
            		system(command.c_str());
           		newConn();
            		message = "[FSSH CONFIG] Config rebuilt successfully.";
        	}
		else if (answer == "0" || answer == "exit") {
        	    	break; 
        	}
		else {
			message = "[FSSH MENU] Invalid option '" + answer + "'. Use one of the following.";
		}
    	}
}

//MAIN THREAD
int main() {
	checkConfig();

	mainMenu();

	return 0;
}
