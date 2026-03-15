#pragma once

#include "../network/Socket.h"
#include "../server/Config.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
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
	nlohmann::json receiveAndScanFileContent();
	std::vector<std::string> scanChunk(const std::string &content);
	void sendResult(const nlohmann::json& threatInfo);
};

} // namespace server
