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
	std::cout << "[ClientHandler] New client connection" << std::endl;

	try {

		std::string content = receiveFile();
		if (content.empty()) {
			std::cerr << "[ClientHandler] Empty file received" << std::endl;
			return;
		}

		std::cout << "[ClientHandler] Received " << content.size() << " bytes" << std::endl;

		auto scanResults = scanFile(content);
		bool isInfected = !scanResults.empty();
		sendResult(isInfected, scanResults);

		std::cout << "[ClientHandler] Client handled successfully" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "[ClientHandler] Error: " << e.what() << std::endl;
	}
}


std::string ClientHandler::receiveFile() {
	uint32_t size;
	_socket.recvT(size);
	size = ntohl(size);
	if (size == 0 || size > MAX_FILESIZE) { 
		return "";
	}
	std::string buffer(size, '\0');
	size_t totalReceived = 0;
	while (totalReceived < size) {
		ssize_t received = _socket.recv(&buffer[totalReceived], size - totalReceived);
		if (received <= 0) {
			break;
		}
		totalReceived += received;
	}

	return buffer;
}

//такая реализация позволяет передавать огромные файлы, но возникает проблема, когда паттерн разибивается чанками
//UPD: решено было изменить на сканирование полного файла, ограничив размер файла, так как recv из сокета все равно будет читать чанками + упрощает реализацию
std::map<std::string, int> ClientHandler::scanFile(const std::string& content) {
	std::cout << "[ClientHandler] scanning file for patterns " << std::endl;
	std::map<std::string, int> results; 
	for (const auto& pattern : _config.getPatterns()) {
		int count = 0;
		size_t pos = 0;
		while ((pos = content.find(pattern, pos)) != std::string::npos) {
			++count;
			pos += pattern.length(); 
		}
		if (count > 0) {
			results[pattern] = count;
			std::cout << "[ClientHandler] Found pattern '" << pattern << "' " << count << std::endl;
		}
		usleep(1000000);
	}

	return results;
}


void ClientHandler::sendResult(bool isInfected, const std::map<std::string, int>& foundPatterns) {
	nlohmann::json result;
	result["status"] = isInfected ? "INFECTED" : "OK";

	nlohmann::json patternsJson = nlohmann::json::array();
	for (const auto& [pattern, count] : foundPatterns) {
		nlohmann::json p;
		p["name"] = pattern;
		p["count"] = count;
		patternsJson.push_back(p);
	}
	result["patterns"] = patternsJson;

	std::string jsonString = result.dump();
	// std::cout<<"JSON string "<< jsonString <<std::endl;

	uint32_t size = htonl(static_cast<uint32_t>(jsonString.size()));
	_socket.sendT(size);
	_socket.send(jsonString.c_str(), size);

	std::cout<<"[Server] ";
	if (isInfected) {
		for (const auto& [pattern, count] : foundPatterns) {
			std::cout << "  - Pattern '" << pattern << "' found " << count<< std::endl;
		}
	}
}

} // namespace server
