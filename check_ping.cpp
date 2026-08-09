#include <sstream>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <future>
#include <chrono>
#include <thread>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

int main() {

	system("clear");

	ifstream config("list.conf");

	vector<string> lines = {"\033[0mChecking connection to LAN devices via ICMP, TCP and HTTPS:\n\n"};
	vector<string> ipAddresses;

	if (config.is_open()) {
		string line;

		while (getline(config, line)) {
			stringstream bufferString(line);
			string name;
			string ipAddress;

			if (bufferString >> name >> ipAddress) {			
				if (name.find("#") != string::npos) {
					continue;
				}

				string color = "\033[0m";

				stringstream fLine;
				fLine << color << name << " ";
				lines.push_back(fLine.str()); lines.push_back("\033[90m[N/A]\n");
				
				ipAddresses.push_back(ipAddress);

			}
			else {
				cout << "Bad format.";
			}
			
		}
		
		size_t maxLenght;
		int checkIndex = 1;
		while (checkIndex < lines.size()) {
			maxLenght = max(maxLenght, lines[checkIndex].size());
			checkIndex += 2;
		}

		cout << maxLenght;

		checkIndex = 1;

		while (checkIndex < lines.size()) {
			string spaceNeeded = string((maxLenght - lines[checkIndex].size()), ' ');
			lines[checkIndex] += spaceNeeded;
			checkIndex += 2;
		}

	}
	else {
		cout << "close";
	}


	int ipIndex = 0;

	//End statistics
	int devicesOnline = 0;
	int portsAvailable = 0;
	int serversUp = 0;	

	vector<string> animation = {
			"\033[0m[Checking /]\n", "\033[0m[Checking -]\n", "\033[0m[Checking \\]\n", "\033[0m[Checking |]\n"
	};

	int frame = 0;

	while (ipIndex < ipAddresses.size()) {
		stringstream pingMake;
		pingMake << "ping -c 1 -w 3 " << ipAddresses[ipIndex] << " > /dev/null 2>&1";
		string pingCommandString = pingMake.str();

		stringstream ncMake;
		ncMake << "timeout 3 nc -zv " << ipAddresses[ipIndex] << " 22 8022 > /dev/null 2>&1";
		string ncCommandString = ncMake.str();

		stringstream curlMake;
		curlMake << "timeout 3 curl -Ik https://" << ipAddresses[ipIndex] << " > /dev/null 2>&1";
		string curlCommandString = curlMake.str();

				
		auto output_ping = async(launch::async, [pingCommandString]() {return system(pingCommandString.c_str());});
		auto output_nc = async(launch::async, [ncCommandString]() {return system(ncCommandString.c_str());});
		
		auto output_curl = async(launch::async, [curlCommandString]() {return system(curlCommandString.c_str());});

				int index = 2 + ipIndex * 2;

		
		while (output_ping.wait_for(chrono::milliseconds(0)) != future_status::ready
			&& output_nc.wait_for(chrono::milliseconds(0)) != future_status::ready
			&& output_curl.wait_for(chrono::milliseconds(0)) != future_status::ready) {
			
			lines[index] = animation[frame];
			frame++;

			if (frame == 4) {
				frame = 0;
			}

			cout << "\033[H";
			for (auto& line : lines) {
				cout << line << flush;
			}
			cout << flush;

			this_thread::sleep_for(chrono::milliseconds(300));

		}

		int pingStatus = output_ping.get();
		int ping_exit_code = WEXITSTATUS(pingStatus);

		int ncStatus = output_nc.get();
		int nc_exit_code = WEXITSTATUS(ncStatus);

		int curlStatus = output_curl.get();
		int curl_exit_code = WEXITSTATUS(curlStatus);

		stringstream makeIndex;
		
		//Ping
		if (ping_exit_code == 0) {

			makeIndex << "\033[96mICMP\033[90m: \033[92m [OK] \033[90m│ ";
			devicesOnline++;
		}
		else {
			makeIndex << "\033[96mICMP\033[90m: \033[91m[FAIL]\033[90m│ ";
		}
		//Netcat
		if (nc_exit_code == 0) {
			makeIndex << "\033[96mTCP\033[90m(22/8022): \033[92m [OK] \033[90m│ ";
			portsAvailable++;
		}
		else {
			makeIndex << "\033[96mTCP\033[90m(22/8022): \033[91m[FAIL]\033[90m│ ";
		}

		//Curl
		if (curl_exit_code == 0) {
			makeIndex << "\033[96mHTTPS\033[90m: \033[92m [OK] \033[0m\n";
			serversUp++;
		}
		else {
			makeIndex << "\033[96mHTTPS\033[90m: \033[91m[FAIL]\033[0m\n";
		}


		lines[index] = makeIndex.str();

		cout << "\033[H";
		for (auto& line : lines) {
			cout << line << flush;
		}
		cout << flush;
		
		ipIndex++;
		
	}
	cout << "            \033[33mOnline\033[90m: \033[37m" << devicesOnline 
		<< "  \033[90m │ \033[33mOpen ports\033[90m: \033[37m" << portsAvailable 
		<< "      \033[90m │ \033[33mServers up\033[90m: \033[37m" << serversUp << "\033[0m\n";
}  
