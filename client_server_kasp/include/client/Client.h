#pragma once

#include "../network/Socket.h"
#include <string>
#include <vector>

namespace client {

struct ScanResponse {
	bool success;
	std::string status;
	std::vector<std::string> patterns;

	ScanResponse() : success(false) {}
	ScanResponse(bool s, std::string st, std::vector<std::string> p)
			: success(s), status(std::move(st)), patterns(std::move(p)) {}
	//избегаем копирования с помощью move секмантики
};

class Client {
public:
	Client();
	~Client();

	bool connect(const std::string &host, uint16_t port);
	ScanResponse sendFile(const std::string &filePath);
	void disconnect();

private:
	net::Socket _socket;
	bool _connected;

	std::string readFile(const std::string &filePath);
	bool sendFileContent(const std::string &content);
	ScanResponse receiveResponse();
};

} // namespace client
