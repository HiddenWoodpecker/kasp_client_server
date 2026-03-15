#pragma once

#include "../network/Socket.h"
#include "../server/Config.h"
#include <atomic>
#include <csignal>
#include <vector>

namespace server {

class Server {
public:
	Server();
	~Server();

	bool initialize(const Config &config);
	void run();
	void stop();

private:
	net::Socket _listenSocket;
	Config _config;
	std::vector<pid_t> _childProcesses;
	std::atomic<bool> _running;

	void setupSignalHandlers();
	void handleSignal(int signal);
	void acceptConnection();
	void spawnWorker(net::Socket clientSocket);
	void cleanup();

	// ссылка на инстанс для обработки сигналов
	static Server *_instance;
	static void signalHandler(int signal);
};

} // namespace server
