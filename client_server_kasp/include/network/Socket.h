#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace net {

class Socket {
public:
	Socket();
	explicit Socket(int fileDescriptor);
	~Socket();

	Socket(const Socket &) = delete;
	Socket &operator=(const Socket &) = delete;

	Socket(Socket &&other) noexcept;
	Socket &operator=(Socket &&other) noexcept;

	bool bind(uint16_t port);
	bool listen(int backlog = 10);
	Socket accept();
	bool connect(const std::string &host, uint16_t port);

	ssize_t send(const void *data, size_t size);
	ssize_t recv(void *buffer, size_t size);

	bool isValid() const { return _fileDescriptor >= 0; }
	int getFileDescriptor() const { return _fileDescriptor; }
	void close();

private:
	int _fileDescriptor;
};

} // namespace net
