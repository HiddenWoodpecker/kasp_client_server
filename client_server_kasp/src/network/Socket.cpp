#include "../../include/network/Socket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

namespace net {

Socket::Socket() : _fileDescriptor(-1) {}

Socket::Socket(int fileDescriptor) : _fileDescriptor(fileDescriptor) {}

Socket::~Socket() { close(); }

Socket::Socket(Socket &&other) noexcept
		: _fileDescriptor(other._fileDescriptor) {
	other._fileDescriptor = -1;
}

Socket &Socket::operator=(Socket &&other) noexcept {
	if (this != &other) {
		close();
		_fileDescriptor = other._fileDescriptor;
		other._fileDescriptor = -1;
	}
	return *this;
}

bool Socket::bind(uint16_t port) {
	_fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (_fileDescriptor < 0) {
		perror("socket");
		return false;
	}

	int opt = 1;
	setsockopt(_fileDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	struct sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (::bind(_fileDescriptor, (struct sockaddr *)&address, sizeof(address)) <0) {
		perror("bind");
		close();
		return false;
	}
	return true;
}

bool Socket::listen(int backlog) {
	if (::listen(_fileDescriptor, backlog) < 0) {
		perror("listen");
		return false;
	}
	return true;
}

Socket Socket::accept() {
	int clientFd = ::accept(_fileDescriptor, nullptr, nullptr);
	return Socket(clientFd);
}

bool Socket::connect(const std::string &host, uint16_t port) {
	_fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (_fileDescriptor < 0) {
		perror("socket");
		return false;
	}

	struct sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);

	if (inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr) <= 0) {
		std::cerr << "Invalid address" << std::endl;
		close();
		return false;
	}

	if (::connect(_fileDescriptor, (struct sockaddr *)&serverAddress,
								sizeof(serverAddress)) < 0) {
		perror("connect");
		close();
		return false;
	}
	return true;
}

ssize_t Socket::send(const void *data, size_t size) {
	if (!isValid()) {
		return -1;
	}
	return ::send(_fileDescriptor, data, size, 0);
}

ssize_t Socket::recv(void *buffer, size_t size) {
	if (!isValid()) {
		return -1;
	}
	return ::recv(_fileDescriptor, buffer, size, 0);
}

void Socket::close() {
	if (_fileDescriptor >= 0) {
		::close(_fileDescriptor);
		_fileDescriptor = -1;
	}
}

} // namespace net
