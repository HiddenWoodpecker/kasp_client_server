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




	// template ф-и на всякий случай
	template <typename T> ssize_t sendT(const T &value) {
		return send(&value, sizeof(T));
	}
	template <typename T> ssize_t recvT(T &value) {
		return recv(&value, sizeof(T));
	}

	bool isValid() const { return _fileDescriptor >= 0; }
	int getFileDescriptor() const { return _fileDescriptor; }
	void close();

private:
	int _fileDescriptor;
};

} // namespace net
