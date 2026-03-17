#include "../../include/server/Server.h"
#include "../../include/server/ClientHandler.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
namespace server {

Server *Server::_instance = nullptr;

Server::Server() : _running(false), _sharedMem(nullptr) { _instance = this; }

void Server::setupSharedMemory(){
	_sharedMem = mmap(nullptr, sizeof(SharedStatistics),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1, 0);
	_stats = new (_sharedMem)SharedStatistics();
}

Server::~Server() {
	stop();
	_instance = nullptr;
}
void Server::setupFifo() {
	unlink(FIFO_PATH);
	if (mkfifo(FIFO_PATH, 0666) < 0) {
		_statsFifoFd = -1;
		return;
	}
	_statsFifoFd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
	if (_statsFifoFd < 0) {
		if (errno == ENXIO) {
			std::cerr << "[Server fifo] no reader yet" << std::endl;
		}
	} else {
		std::cout << "[Server fifo] FIFO opened successfully" << std::endl;
	}
}

void Server::writeFifo() {
	_statsFifoFd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
	if (_statsFifoFd >= 0){
		std::string stats;
		stats += "Files scanned: " + std::to_string(_stats->totalFiles) + "\n";
		stats += "Infected files: " + std::to_string(_stats->infectedFiles);
		uint64_t total = _stats->totalFiles;
		if (total > 0) {
			stats += " (" + std::to_string(_stats->infectedFiles * 100 / total) + "%)";
		}
		stats += "\nTotal threats: " + std::to_string(_stats->totalThreats) + "\n"; 
		stats += "\nPatterns found:\n";
		for (int i = 0; i < SharedStatistics::MAX_PATTERNS; i++) {
			if (_stats->patternNames[i][0] == '\0') continue;
			uint64_t count = _stats->patternCounts[i];
			if (count > 0) {
				stats += "  " + std::string(_stats->patternNames[i]) + ": " + 
						 std::to_string(count) + "\n";
			}
		}
		stats += "\n";
		write(_statsFifoFd, stats.c_str(), stats.size());
	}
}


bool Server::initialize(const Config &config) {
	_config = config;
	_listenSocket.bind(_config.getPort());
	_listenSocket.listen(10);
	setupSharedMemory();
	setupFifo();
	setupSignalHandlers();
	_stats->initPatterns(_config.getPatterns());
	_statsThread = std::thread(&Server::statisticsLoop, this);
	_running = true;
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
	std::cout<<"[Server] inside handle signal";
	switch (signal) {
		case SIGCHLD:
			while (true) {
				int status;
				pid_t pid = waitpid(-1, &status, WNOHANG);
				if (pid <= 0) break;
				auto it = std::find(_childProcesses.begin(), _childProcesses.end(), pid);
				if (it != _childProcesses.end()) {
					_childProcesses.erase(it);
				}
				if (WIFEXITED(status)) {
					std::cout << "[Server] Worker " << pid << " exited normally" << std::endl;
				} else if (WIFSIGNALED(status)) {
					std::cout << "[Server] Worker " << pid << " killed by signal " 
							  << WTERMSIG(status) << std::endl;
				}
			}
			break;
			
		case SIGINT:
		case SIGTERM:
			std::cout << "\n[Server] Shutting down..." << std::endl;
			stop();
			break;
	}
}

void Server::run() {
	while (_running) {
		acceptConnection();
	}
	stop();
}

void Server::acceptConnection() {
	net::Socket clientSocket = _listenSocket.accept();
	if (!clientSocket.isValid()) {
		if (errno == EINTR) {
			return;
		}
		return;
	}

	spawnWorker(std::move(clientSocket));
}

void Server::spawnWorker(net::Socket clientSocket) {
	pid_t pid = fork();

	if (pid < 0) {
		return;
	}

	if (pid == 0) {
		_listenSocket.close();
		ClientHandler handler(std::move(clientSocket), _config, _stats);
		handler.run();

		exit(0);
	} else {
		std::cout << "[Server] spawned worker process " << pid << std::endl;
		_childProcesses.push_back(pid);
		clientSocket.close();
	}
}
void Server::statisticsLoop() {
	while (_running) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		writeFifo();
	}
}

void Server::stop() {
	if (!_running){return;}
	_running = false;
	if (_statsThread.joinable()){_statsThread.join();}
	cleanup();
}

void Server::cleanup() {
	cleanupWorkers();		
	cleanupSharedMemory();  
	cleanupFifo();		 
	cleanupSocket();   
}


void Server::cleanupWorkers() {
	if (_childProcesses.empty()) {
		return;
	}
	
	std::cout << "[Server]waiting for " << _childProcesses.size() << " workers to finish" << std::endl; 
	for (pid_t pid : _childProcesses) {
		int status;
		std::cout << "[Server] waiting for worker " << pid << std::endl; 
		waitpid(pid, &status, 0);	
	} 
	_childProcesses.clear();
}
void Server::cleanupSharedMemory() {
	if (_sharedMem && _sharedMem != MAP_FAILED) {
		munmap(_sharedMem, sizeof(SharedStatistics));
		_sharedMem = nullptr;
		_stats = nullptr;
	}
}

void Server::cleanupFifo() {
	if (_statsFifoFd >= 0) {
		close(_statsFifoFd);
		_statsFifoFd = -1;
	} 
	unlink(FIFO_PATH);
	
}

void Server::cleanupSocket() {
	if (_listenSocket.isValid()) {
		_listenSocket.close();
	}
}

} // namespace server
