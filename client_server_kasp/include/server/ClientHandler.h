#pragma once

#include "../network/Socket.h"
#include "../server/Config.h"
#include "../server/Server.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <map>
#define MAX_FILESIZE 100 * 1024 * 1024
//100 mb
namespace server {

class ClientHandler {
public:
	ClientHandler(net::Socket socket, const Config &config, SharedStatistics* stats);

	void run();

private:
	net::Socket _socket;
	Config _config;
	uint32_t _fileSize;
	SharedStatistics* _stats;

	std::string receiveFile();
	std::map<std::string, int> scanFile(const std::string& content);
	void sendResult(const bool isInfected, const std::map<std::string, int>&);
	void updateStatistics(const std::map<std::string, int>& results);
};
} // namespace server
