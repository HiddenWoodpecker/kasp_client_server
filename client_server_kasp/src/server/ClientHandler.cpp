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
			return;
		}
		nlohmann::json threatInfo = receiveAndScanFileContent();
		sendResult(threatInfo);	

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
	if (size > MAX_FILESIZE) {
		std::cerr << "[ClientHandler] File size too large: " << size << " bytes" << std::endl;
		return 0;
	}

	return size;
}

nlohmann::json ClientHandler::receiveAndScanFileContent() {

	const size_t chunkSize = 4096; 
	std::string buffer(chunkSize, '\0');
	size_t totalReceived = 0;
	nlohmann::json threatInfo; 
	threatInfo["status"] = "OK"; 
	threatInfo["threats_found"] = false;
	threatInfo["patterns"] = nlohmann::json::array(); 

	std::cout << "[ClientHandler::receiveAndScanFileContent] Starting to receive and scan " << _fileSize << " bytes..." << std::endl;

	while (totalReceived < _fileSize) {
		size_t toRead = std::min(chunkSize, static_cast<size_t>(_fileSize - totalReceived));
		ssize_t received = _socket.recv(&buffer[0], toRead);

		if (received <= 0) {
			std::cerr << "[ClientHandler::receiveAndScanFileContent] Receive failed at total: " << totalReceived << "/" << _fileSize << std::endl;
			break;
		}
		std::string chunk = buffer.substr(0, received);

		auto foundInChunk = scanChunk(chunk);

				if (!foundInChunk.empty()) {
			threatInfo["threats_found"] = true;
			threatInfo["status"] = "INFECTED";
			for (const auto& pattern : foundInChunk) {
				threatInfo["patterns"].push_back(pattern);
			}
		}

		totalReceived += received;
	}


	std::cout << "[ClientHandler::receiveAndScanFileContent] Finished receiving and scanning." << std::endl;
	return threatInfo;
}

std::vector<std::string> ClientHandler::scanChunk(const std::string &chunk) {
	std::vector<std::string> foundPatterns;
	for (const auto& pattern : _config.getPatterns()) {
	    if (chunk.find(pattern) != std::string::npos) {
	        std::cout << "[ClientHandler::scanChunk] Found pattern '" << pattern << "' in chunk." << std::endl;
	        foundPatterns.push_back(pattern);
	    }
	}
	return foundPatterns;
}

void ClientHandler::sendResult(const nlohmann::json& threatInfo) {
	std::string jsonString = threatInfo.dump();

	protocol::MessageHeader header(static_cast<uint32_t>(jsonString.size()));
	_socket.sendT(htonl(header.size));
	_socket.send(jsonString.c_str(), htonl(jsonString.size()));

	std::cout << "[ClientHandler] Sent result status: " << threatInfo["status"] << std::endl;
	std::cout << "[ClientHandler] Sent result: " << threatInfo["patterns"] << std::endl;
}

} // namespace server
