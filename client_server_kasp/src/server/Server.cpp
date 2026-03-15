#include "../../include/server/Server.h"
#include "../../include/server/ClientHandler.h"
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace server {

Server *Server::_instance = nullptr;

Server::Server() : _running(false) { _instance = this; }

Server::~Server() {
	cleanup();
	_instance = nullptr;
}

bool Server::initialize(const Config &config) {
	_config = config;

	std::cout << "[Server] Initializing on port " << _config.getPort()<< std::endl;

	if (!_listenSocket.bind(_config.getPort())) {
		std::cerr << "[Server] Failed to bind" << std::endl;
		return false;
	}

	if (!_listenSocket.listen(10)) {
		std::cerr << "[Server] Failed to listen" << std::endl;
		return false;
	}

	setupSignalHandlers();
	_running = true;

	std::cout << "[Server] Ready to accept connections" << std::endl;
	return true;
}

void Server::setupSignalHandlers() {
	struct sigaction sa{};
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGINT, &sa, nullptr);
	sigaction(SIGTERM, &sa, nullptr);
	signal(SIGCHLD, SIG_IGN);
}

void Server::signalHandler(int signal) {
	if (_instance) {
		_instance->handleSignal(signal);
	}
}

void Server::handleSignal(int signal) {
	std::cout << "\n[Server] Received signal " << signal << ", shutting down"<< std::endl;
	_running = false;
}

void Server::run() {
	while (_running) {
		acceptConnection();
	}

	cleanup();
}

void Server::acceptConnection() {
	std::cout << "[Server] waiting for connection..." << std::endl;

	net::Socket clientSocket = _listenSocket.accept();

	if (!clientSocket.isValid()) {
		if (errno == EINTR) {
			return;
		}
		std::cerr << "[Server] accept failed" << std::endl;
		return;
	}

	spawnWorker(std::move(clientSocket));
}

void Server::spawnWorker(net::Socket clientSocket) {
	pid_t pid = fork();

	if (pid < 0) {
		std::cerr << "[Server] fork failed" << std::endl;
		return;
	}

	if (pid == 0) {
		_listenSocket.close();

		ClientHandler handler(std::move(clientSocket), _config);
		handler.run();

		exit(0);
	} else {
		std::cout << "[Server] spawned worker process " << pid << std::endl;
		_childProcesses.push_back(pid);
		clientSocket.close();
	}
}

void Server::stop() {
	_running = false;
	_listenSocket.close();
}

void Server::cleanup() {
	std::cout << "[Server] Cleaning up..." << std::endl;

	for (pid_t pid : _childProcesses) {
		kill(pid, SIGTERM);
	}

	for (pid_t pid : _childProcesses) {
		waitpid(pid, nullptr, 0);
	}

	_childProcesses.clear();
	_listenSocket.close();

	std::cout << "[Server] Cleanup complete" << std::endl;
}

} // namespace server
