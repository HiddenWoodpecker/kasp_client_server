#include "../../include/client/Client.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << " <file_path> <host> <port>" << std::endl;
		return 1;
	}
	
	std::string filePath = argv[1];
	std::string host = argv[2];
	uint16_t port = static_cast<uint16_t>(std::stoi(argv[3]));
	
	client::Client client;
	
	if (!client.connect(host, port)) {
		return 1;
	}
	
	client::ScanResponse response = client.sendFile(filePath);
	
	std::cout << "\nScan Result\n";
	std::cout << "Status: " << response.status << std::endl;
	
	if (!response.patterns.empty()) {
		std::cout << "threats found: " << response.patterns.size() << std::endl;
		for (const auto& pattern : response.patterns) {
			std::cout << "	- " << pattern << std::endl;
		}
	} else {
		std::cout << "no threats found" << std::endl;
	}
	
	client.disconnect();
	
	return response.success ? 0 : 1;
}
