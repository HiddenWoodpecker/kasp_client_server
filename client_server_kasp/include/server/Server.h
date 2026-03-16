#pragma once

#include "../network/Socket.h"
#include "../server/Config.h"
#include <atomic>
#include <mutex>
#include <csignal>
#include <vector>
#include <string>
#include <map>

namespace server {

struct Statistics {
	uint32_t infectedFiles;
	std::map<std::string, int> foundPatterns;

	Statistics() : infectedFiles(0), foundPatterns(){};
};

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
	static struct Statistics stats;
};

} // namespace server
