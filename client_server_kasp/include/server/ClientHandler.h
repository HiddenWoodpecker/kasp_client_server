#pragma once

#include "../network/Socket.h"
#include "../server/Config.h"
#include <string>
#include <vector>

#define MAX_FILESIZE 100 * 1024 * 1024

namespace server {

class ClientHandler {
public:
	ClientHandler(net::Socket socket, const Config &config);

	void run();

private:
	net::Socket _socket;
	Config _config;
	uint32_t _fileSize;

	uint32_t receiveFileSize();
	std::string receiveFileContent();
	bool scanFile(const std::string &content);
	void sendResult(bool isInfected,
									const std::vector<std::string> &foundPatterns);
};

} // namespace server
