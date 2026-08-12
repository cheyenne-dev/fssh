#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <filesystem>
#include "backend.hpp"

std::string getHomeDir() {
	const std::string homeEnv = getenv("HOME");
	return homeEnv;
}

//ERROR RESOLVER
errorInfo getError(error errorCode, std::string prefix, std::string arg){
	switch (errorCode) {
		case error::connProfileFail:
			return {prefix + "Can't read '" + arg + "' profile in '~/.config/fssh/config.yaml'.",
				prefix + "If you are sure that profile you entered exists, try editing it."};
		case error::profileAddFail:
			return {prefix + "Can't add profile '" + arg + "' to '~/.config/fssh/config.yaml'.",
				""};
		case error::profileEditFail:
			return {prefix + "Can't edit profile '" + arg + "' in '~/.config/fssh/config.yaml'.",
			""};
		case error::profileDelFail:
			return {prefix + "Can't delete profile '" + arg + "' in '~/.config/fssh/config.yaml'.",
			""};
		default:
			return {prefix + "Unexpected error. Sorry, I can't help you. You are on your own.",
			""};
	}
}

//SETTING UP NEW CONNECTION
operationResult newConn(const std::string& newName, const std::string& newAddr, const std::string& newPort, const std::string& newUser) {
	try {
		std::ofstream(CONFIG_PATH, std::ios::app).close();

		YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		config[newName]["ipAddr"] = newAddr;
		config[newName]["port"] = newPort;
		config[newName]["username"] = newUser;

		std::ofstream fout(CONFIG_PATH);
		fout << config;
		fout.close();
	} catch (const YAML::Exception& e) {
		return {false, getError(error::profileAddFail, "[FSSH EDIT] ", newName)};
	}

	return {true, {}};
}

operationResult editConn (const std::string& editName, std::string newName, const std::string& newAddr, const std::string& newPort, const std::string& newUser) {
	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		if (newName.empty()) {
			newName = editName;
		}
		else {
			config[newName] = YAML::Clone(config[editName]);	
		}

		if (!newAddr.empty()) {
			config[newName]["ipAddr"] = newAddr;
		}

		if (!newPort.empty()) {
			config[newName]["port"] = newPort;
		}

		if (!newUser.empty()) {
        		config[newName]["username"] = newUser;	
		}

		if (newName != editName) {
			config.remove(editName);
		}

		std::ofstream fout(CONFIG_PATH);
		fout << config;
		fout.close();
	} catch (const YAML::Exception& e) {
		return {false, getError(error::profileEditFail, "[FSSH EDIT] ", editName)};
	}

	return {true, {}};
}

operationResult delConn(std::string delName) {
	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		config.remove(delName);

		std::ofstream fout(CONFIG_PATH);
		fout << config;
		fout.close();
	} catch (const YAML::Exception& e) {
		return {false, getError(error::profileDelFail, "[FSSH EDIT] ", delName)};
	}

	return {true, {}};
}

//TCP CHECKING
std::vector<std::string> checkHosts(const std::vector<report>& profileList, boost::filesystem::path ncPath) {
	std::vector<std::string> result;
	result.resize(profileList.size());

	std::vector<boost::process::process> processes;
	processes.reserve(profileList.size());

	boost::asio::io_context ctx;

	for (size_t i = 0; i < profileList.size(); i++) {
		processes.push_back(boost::process::process(ctx, ncPath ,{"-zv", "-w", "1", profileList[i].ip, profileList[i].port}, boost::process::process_stdio{nullptr, nullptr, nullptr}));
	}

	for (size_t i = 0; i < profileList.size(); i++) {
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
std::vector<report> readConfig() {
	std::vector<report> response;
	
	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);

		if (config.IsMap()) {
			for (auto const& rootElement : config) {
				std::string profileName = rootElement.first.as<std::string>();

				report reportLine;
				reportLine.name = profileName;
				reportLine.ip = config[profileName]["ipAddr"].as<std::string>();
				reportLine.port = config[profileName]["port"].as<std::string>();
				reportLine.user = config[profileName]["username"].as<std::string>();

				response.push_back(reportLine);
			}
		}
	} catch (const YAML::Exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return response;
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
		return "[FSSH CONN] ERROR: Can't read config";
	}

	std::string destination =  username + "@" + ipAddr;
	boost::asio::io_context ctx;
	boost::filesystem::path sshPath = boost::process::environment::find_executable("ssh");
	boost::process::process proc(ctx, sshPath, {
				destination,
				"-p",
				port,
			}
	);
	proc.wait();

	return "";

}

void makeKeyPair() {
	int keygenResult = system("ssh-keygen -t ed25519 -N \"\" -f ~/.ssh/id_ed25519");
	if (keygenResult == 0) {
		std::cout << "[FSSH KEYGEN] Key pair generated successfully!" << std::endl;
	}
	std::string keyLocation = HOME_PATH + "/.ssh/id_ed25519.pub";

	for (auto& line : readConfig()) {
		std::string destination = line.user + "@" + line.ip;

		boost::asio::io_context ctx;
		boost::filesystem::path ssh_copy_idPath = boost::process::environment::find_executable("ssh-copy-id");
		boost::process::process proc(ctx, ssh_copy_idPath, {
					"-i", 
					keyLocation,
					"-p",
					line.port,
					destination,
					});
		proc.wait();
	}
}

//CHECKING CONFIG
bool checkConfig() {
	
	try {
		YAML::Node config = YAML::LoadFile(CONFIG_PATH);
		return true; 

	} catch (const YAML::Exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::filesystem::create_directories(HOME_PATH + "/.config/fssh");
		std::ofstream(CONFIG_PATH, std::ios::app).close();
		return false;
	}
}
