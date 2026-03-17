#include "../../include/client/Client.h"
#include "../../include/network/Protocol.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace client {

Client::Client() : _connected(false) {}

Client::~Client() { disconnect(); }

bool Client::connect(const std::string &host, uint16_t port) {
	std::cout << "client connecting to " << host << ":" << port << std::endl;

	if (!_socket.connect(host, port)) {
		std::cerr << "client connection failed" << std::endl;
		return false;
	}

	_connected = true;
	std::cout << "client connected successfully" << std::endl;
	return true;
}

ScanResponse Client::sendFile(const std::string &filePath) {
	if (!_connected) {
		return ScanResponse{false, "not connected", {}};
	}
	std::string content = readFile(filePath);
	if (content.empty()) {
		return ScanResponse{false, "failed to read file", {}};
	}
	if (!sendFileContent(content)) {
		return ScanResponse{false, "failed to send file", {}};
	}

	return receiveResponse();
}

std::string Client::readFile(const std::string &filePath) {
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cerr << "client cannot open file" << std::endl;
		return "";
	}
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string buffer(size, '\0');
	file.read(&buffer[0], size);
	return buffer;
}

bool Client::sendFileContent(const std::string &content) {
	uint32_t size = htonl(static_cast<uint32_t>(content.size()));
	if (_socket.send(&size, sizeof(size)) <= 0) {
		return false;
	}
	ssize_t sent = _socket.send(content.c_str(), content.size());
	return sent == static_cast<ssize_t>(content.size());
}

ScanResponse Client::receiveResponse() {
	uint32_t size;
	if (_socket.recvT(size) <= 0) {
		return ScanResponse{false, "failed to receive response", {}};
	}
	size = ntohl(size);
	std::cout << "[Client] got response size = " << size << std::endl;
	if (size == 0 || size > 1024 * 1024) {
		return ScanResponse{false, "invalid response size", {}};
	}

	std::string jsonString(size, '\0');
	if (_socket.recv(&jsonString[0], size) <= 0) {
		return ScanResponse{false, "failed to receive response data", {}};
	}

	try {
	nlohmann::json response = nlohmann::json::parse(jsonString);

        ScanResponse result;
        result.success = true;
        result.status = response.value("status", "UNKNOWN");

        if (response.contains("patterns") && response["patterns"].is_array()) {
            for (const auto& item : response["patterns"]) {
                if (item.is_object() && item.contains("name")) {
                    std::string name = item.value("name", "");
                    int count = item.value("count", 1); 
                    if (!name.empty()) {
			name.append(" count :" + std::to_string(count)); 
                        result.patterns.push_back(name);
                    }
                }
            }
        }

        return result;
    } catch (const std::exception& e) {
        std::cerr <<e.what() << std::endl;
        return ScanResponse{false, "Failed to parse response", {}}; 
    }}

void Client::disconnect() {
	if (_connected) {
		_socket.close();
		_connected = false;
		std::cout << "client disconnected" << std::endl;
	}
}

} // namespace client
