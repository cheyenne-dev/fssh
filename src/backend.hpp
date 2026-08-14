#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <string>
#include <vector>
#include <boost/process.hpp>

#include "constants.hpp"

enum class error {
	connProfileFail,
	profileAddFail,
	profileEditFail,
	profileDelFail,
	
	launcherConfigFail
};

struct errorInfo {
    std::string message;
    std::string hint;
};

struct operationResult {
	bool success;
	errorInfo error;
};

struct report {
	std::string name;
	std::string ip;
	std::string port;
	std::string user;

};

//GETTING ~/ DIRECTORY
inline const std::string CONFIG_PATH = HOME_PATH + "/.config/fssh/config.yaml";

//ERROR HANDLER
errorInfo getError(error errorCode, std::string prefix = "[FSSH ERROR] ", std::string arg = "N/A");

//OPERATIONS WITH CONNECTIONS
operationResult newConn(const std::string& newName, const std::string& newAddr, const std::string& newPort, const std::string& newUser);
operationResult editConn (const std::string& editName, std::string newName, const std::string& newAddr, const std::string& newPort, const std::string& newUser);
operationResult delConn(std::string delName);

//TCP CHECKING
std::vector<std::string> checkHosts(const std::vector<report>& profileList, boost::filesystem::path ncPath);

//CONFIG READ
std::vector<report> readConfig();

//ESTABLISHING CONNECTION
std::string establishConn(std::string deviceName);

//MAKE SSH KEYS
void makeKeyPair();

//CONFIG CHECKING
bool checkConfig();

#endif
