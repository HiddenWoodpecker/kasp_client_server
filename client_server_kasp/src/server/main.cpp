#include "../../include/server/Config.h"
#include "../../include/server/Server.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
	std::cout << "Vhod"<<std::endl;
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <config.json> <port>" << std::endl;
		return 1;
	}
	server::Config config;
	config.setConfigPath(argv[1]);
	config.setPort(static_cast<uint16_t>(std::stoi(argv[2])));
	// TODO: fifo
	config.setFifoPath("/tmp/antivirus_fifo");
	if (!config.load(config.getConfigPath())) {
		std::cerr << "failed to load config" << std::endl;
		return 1;
	}

	server::Server server;

	if (!server.initialize(config)) {
		std::cerr << "failed to initialize server" << std::endl;
		return 1;
	}

	server.run();

	return 0;
}
