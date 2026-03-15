#include "../../include/server/ClientHandler.h"
#include "../../include/network/Protocol.h"
#include <iostream>
#include <netinet/in.h>
#include <nlohmann/json.hpp>

// TODO: сделать логгер нормальный

namespace server {
ClientHandler::ClientHandler(net::Socket socket, const Config &config)
	: _socket(std::move(socket)), _config(config), _fileSize(0) {}

void ClientHandler::run() {
	std::cout << "[ClientHandler] new client connection" << std::endl;

	try {
		_fileSize = receiveFileSize();
		if (_fileSize == 0) {
			std::cerr << "[ClientHandler] invalid file size received" << std::endl;
			return;
		}
		std::cout << "[ClientHandler] Expected file size: " << _fileSize << " bytes" << std::endl;
		std::string content = receiveFileContent();
		if (content.size() != _fileSize) {
			std::cerr << "[ClientHandler] incomplete file received: " << content.size() << "/" << _fileSize << std::endl;
			return;
		}

		std::cout << "[ClientHandler] received " << content.size() << " bytes" << std::endl;
		bool isInfected = scanFile(content);
		sendResult(isInfected, {});

		std::cout << "[ClientHandler] client handled successfully" << std::endl;
	} 
	catch (const std::exception &e) {
		std::cerr << "[ClientHandler] error: " << e.what() << std::endl;
	}
}

uint32_t ClientHandler::receiveFileSize() {
	protocol::MessageHeader header;
	auto recvRes = _socket.recvT(header.size);
	if (recvRes <= 0) {
		std::cerr << "[ClientHandler] failed to receive file size header, result: " << recvRes << std::endl;
		return 0;
	}

	uint32_t size = ntohl(header.size);
	std::cout << "[ClientHandler] received raw size (ntohl applied): " << size << std::endl;
	if (size > MAX_FILESIZE) {
		std::cerr << "[ClientHandler] File size too large: " << size << " bytes" << std::endl;
		return 0;
	}

	return size;
}

std::string ClientHandler::receiveFileContent() {

	if (_fileSize == 0) {
		return "";
	}

	std::string buffer(_fileSize, '\0');
	size_t totalReceived = 0;

	std::cout << "[ClientHandler::receiveFileContent] starting to receive "<< _fileSize << " bytes" << std::endl;

	while (totalReceived < _fileSize) {
		ssize_t received = _socket.recv(&buffer[totalReceived], _fileSize - totalReceived);
		std::cout<< "[ClientHandler::receiveFileContent] attempted to receive, got: " << received << " bytes (total: " << totalReceived << ")" << std::endl;
		if (received <= 0) {
			std::cerr << "[ClientHandler::receiveFileContent] receive failed at total: " << totalReceived << " = " << _fileSize << std::endl;
			break;
		}
		totalReceived += received;
	}

	if (totalReceived < _fileSize) {
		std::cerr<< "[ClientHandler::receiveFileContent] incomplete data received: "<< totalReceived << "/" << _fileSize << std::endl;
		return buffer.substr(0, totalReceived);
	}

	std::cout << "[ClientHandler::receiveFileContent] successfully received full content." << std::endl;
	return buffer;
}

bool ClientHandler::scanFile(const std::string &content) {
	// TODO: ЗАГЛУШКА
	std::cout << "[ClientHandler] scanning file ..." << std::endl;
	return false;
}

void ClientHandler::sendResult(bool isInfected,
															 const std::vector<std::string> &foundPatterns) {
	nlohmann::json result;
	result["status"] = isInfected ? "INFECTED" : "OK";
	result["patterns"] = foundPatterns;

	std::string jsonString = result.dump();
	std::cout << "\n[SERVER] sending result " << jsonString << std::endl;
	protocol::MessageHeader header(static_cast<uint32_t>(jsonString.size()));
	std::cout << "\n[SERVER] sending size " << jsonString.size() << std::endl;
	_socket.sendT(ntohl(header.size));
	_socket.send(jsonString.c_str(), jsonString.size());
	std::cout << "[ClientHandler] Sent result: " << result["status"] << std::endl;
}

} // namespace server
