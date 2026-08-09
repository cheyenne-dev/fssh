#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/process.hpp>
#include <sstream>

using namespace std;

void logo();

string getHomeDir() {
	const string homeEnv = getenv("HOME");
	return homeEnv;
}

const string HOME_PATH = getHomeDir();
const string CONFIG_PATH = HOME_PATH + "/.config/fssh/config.yaml";

//SETTING UP NEW CONNECTION

string printError(string context) {
	if (context == "connProfileFail") {
		return "";
	}

	return "[FSSH ERROR] Unexpected error. Sorry, I can't help you. You are on your own. ";
}

string newConn() {
	system("clear");

	string command = "touch " + CONFIG_PATH; //FIX THIS!!!!!!! COMMAND INJECTION!!!

	system(command.c_str());

	YAML::Node config = YAML::LoadFile(CONFIG_PATH);
	
	string newAddr;
	string newName;
	string newPort;
	string newUser;

	//Setting up
	
	logo();
	
	cout << "[FSSH EDIT] Adding new profile. Spaces are NOT allowed!\n"
	     << "[FSSH EDIT] Enter new name: ";
	cin >> newName;
	cout << "[FSSH EDIT] Enter new address: ";
	cin >> newAddr;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "[FSSH EDIT] Enter new port (default: 22): ";
	getline(cin, newPort);
	if (newPort.empty()) {
		newPort = "22";
	}
	cout << "[FSSH EDIT] Enter new user: ";
	cin >> newUser;

	config[newName]["ipAddr"] = newAddr;
	config[newName]["port"] = newPort;
	config[newName]["username"] = newUser;

	ofstream fout(CONFIG_PATH);
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

void editConn (string editName) {
	YAML::Node config = YAML::LoadFile(CONFIG_PATH);
	
	string newAddr;
	string newName;
	string newPort;
	string newUser;

	//Setting up
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Edit name (default: " << editName << "): ";
	getline(cin, newName);
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
	cout << "Edit address (default: " << config[editName]["ipAddr"].as<string>() << "): ";
	getline(cin, newAddr);
	if (!newAddr.empty()) {
		config[newName]["ipAddr"] = newAddr;
	}
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Edit port (default: " << config[editName]["port"].as<string>() << "): ";
	getline(cin, newPort);
	if (!newPort.empty()) {
		config[newName]["port"] = newPort;
	}
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Edit username (default: " << config[editName]["username"].as<string>() << "): ";
	getline(cin, newUser);
	if (!newUser.empty()) {
        	config[newName]["username"] = newUser;	
	}

	if (newName != editName) {
		config.remove(editName);
	}



	//config[newName]["ipAddr"] = newAddr;
	//config[newName]["port"] = newPort;
	//config[newName]["username"] = newUser;

	ofstream fout(CONFIG_PATH);
	fout << config;
	fout.close();
}

void delConn(string delName) {
	YAML::Node config = YAML::LoadFile(CONFIG_PATH);

	config.remove(delName);

	ofstream fout(CONFIG_PATH);
	fout << config;
	fout.close();
}

//READING CONFIG.YAML
vector<string> readConfig(bool fullReport = false) {
	vector<string> names;
	
	try {
			YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		if (config.IsMap()) {
			if (fullReport == false) {
				for (auto const& rootElement : config) {
					string deviceName = rootElement.first.as<string>();
					names.push_back(deviceName);
				}
			}
			else {
				for (auto const& rootElement : config) {
					string reportString;
					string deviceName = rootElement.first.as<string>();

					string ipAddr = config[deviceName]["ipAddr"].as<string>();
					string port = config[deviceName]["port"].as<string>();
					string username = config[deviceName]["username"].as<string>();
					
					reportString = deviceName + ": "
						+ " IP=" + ipAddr
						+ " SSH-PORT=" + port
						+ " USERNAME=" + username;
					
					names.push_back(reportString);
				}
			}
		}
	} catch (const YAML::Exception& e) {
		cerr << "Error: " << e.what() << endl;
	}

	return names;
}

//ESTABLISHING CONNECTION
string establishConn(string deviceName) {

	string ipAddr;
	string port;
	string username;
	
	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		ipAddr = config[deviceName]["ipAddr"].as<string>();
		port = config[deviceName]["port"].as<string>();
		username = config[deviceName]["username"].as<string>();

	} catch (const YAML::Exception& e) {
		cerr << "Error: " << e.what() << endl;
		return "\033[31m[FSSH CONN] ERROR: Can't read '" + deviceName + "' profile in '~/.config/fssh/config.yaml'.\033[0m\n";
	}

	stringstream mkDest;
	mkDest << username << "@" << ipAddr;

	string destination = mkDest.str();

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
		cout << "[FSSH KEYGEN] Key pair generated successfully!" << endl;
	}
	string keyLocation = HOME_PATH + "/.ssh/id_ed25519.pub";

	vector<string> names = readConfig();
	YAML::Node config = YAML::LoadFile(CONFIG_PATH);

	for (auto& name : names) {
		string destination = config[name]["username"].as<string>() + "@" + config[name]["ipAddr"].as<string>();
		string port = config[name]["port"].as<string>();

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
string LOGO_TYPE = "solid";

bool checkConfig() {

	//if (!filesystem::exists(CONFIG_PATH)) {
	//
	//}

	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);
		//LOGO_TYPE = 

	} catch (const YAML::Exception& e) {
		cerr << "Error: " << e.what() << endl;
		string command = "mkdir -p " + HOME_PATH + "/.config/fssh; " + "touch " + CONFIG_PATH;
		system(command.c_str());

		cout << "Config is not valid. Creating new file..." << endl;
		newConn();
	}

	return true;

}

//   FRONT-END FUNCTIONS
//MENU: ONLY LOGO
void logoLINES() {
	cout //<< "\033[0m                                     \n"
	     //<< "\033[34m        \033[34m░▒▓\033[44m\033[37m  cheyenne-dev/fssh  \033[0m\033[34m▓▒░     \033[0m\n"
	     //<< "\n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m   ______ _____ _____ _    _   \033[0m\033[34m▓▓▒▒░░ \n" 
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
	cout //<< "\033[0m                                     \n"
	     //<< "\033[34m        \033[34m░▒▓\033[44m\033[37m  cheyenne-dev/fssh  \033[0m\033[34m▓▒░     \033[0m\n"
	     //<< "\n"
	     << "\033[34m░░▒▒▓▓\033[44m\033[37m                               \033[0m\033[34m▓▓▒▒░░ \n"
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
string chooseConn() {
	checkConfig();
	
	vector<string> availableNames = readConfig();

	for (auto& line : availableNames) {
		cout << line << endl;
	}

	string deviceName;

	cout << "Enter device name: ";
	cin >> deviceName;

	auto check = find(availableNames.begin(), availableNames.end(), deviceName);

	if (check != availableNames.end()) {
		cout << "Name found " << deviceName << endl;
	}
	else {
		cout << "FAIL: " << deviceName << endl;
	}

	return establishConn(deviceName);
}

//MENU: EDIT CONFIG
void editConfig(string error = "") {
	system("clear");
	logo();
	
	vector<string> fullReport = readConfig(true);
	vector<string> availableNames = readConfig();
	
	cout << "Existing profiles:" << endl;

	int i = 1;
	for (auto& line : fullReport) {
		cout << i << " - "<< line << endl;
		i++;
	}
	
	cout << error <<"[FSSH EDIT] Which profile you want to edit: ";
	string editName;
	cin >> editName;

	if (editName == "exit") {
		return;
	}

	auto check = find(availableNames.begin(), availableNames.end(), editName);

	if (check != availableNames.end()) {	
		cout << "\n[FSSH EDIT] Modifying " << editName << ". Be careful! Changes are applied IMMEDIATELY!"<<endl;
		editConn(editName);
	}
	else {
		error = "[FSSH EDIT] ERROR: invalid name " + editName + "\n";
		editConfig(error);
	}


}

//MENU: DELETE PROFILE
void deleteProfile(string error = "") {
	system("clear");
	logo();

	vector<string> fullReport = readConfig(true);
	vector<string> availableNames = readConfig();
	
	cout << "Existing profiles:" << endl;

	int i = 1;
	for (auto& line : fullReport) {
		cout << i << " - "<< line << endl;
		i++;
	}
	
	cout << error <<"[FSSH EDIT] Which profile do you want to delete: ";
	string delName;
	cin >> delName;

	if (delName == "exit") {
		return;
	}

	auto check = find(availableNames.begin(), availableNames.end(), delName);

	if (check != availableNames.end()) {	
		cout << "\n[FSSH EDIT] Waiting confirmation for " << delName << "\nTo confirm deletion, type the profile name again"<<endl;
		string confirmation;
		cin >> confirmation;
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
void mainMenu(string message = "[FSSH MENU] Errors or notifications will appear here.") {	
	bool firstRun = true; 

	while (true) {
		system("clear");
        	logo();

        	if (!message.empty()) {
        		cout << "\n" << message << "\n\n";
        	}

        	cout << "Choose option:\n\n"
             	     << "┌ 1 - Establish connection;\n"
		     << "│ 2 - Edit existing profile;\n"
            	     << "└ 3 - Add new profile;\n\n"
             	     << "┌ 4 - Generate key pair (for all profiles);\n"
		     << "└ 5 - empty entry;\n\n"
            	     << "┌ 8 - Delete existing profile;\n"
        	     << "└ 9 - Rebuild config from scratch;\n\n"
		     << "  0 - Exit.\n\n" 
             	     << "Your choice (default: 1): ";

        	string answer;
        
        	if (!firstRun) {
            	cin.ignore(numeric_limits<streamsize>::max(), '\n');
        	}
        	firstRun = false;

        	getline(cin, answer);
		
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
            		string command = "rm " + CONFIG_PATH;
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

