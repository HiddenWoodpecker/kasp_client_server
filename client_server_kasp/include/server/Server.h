#pragma once
#include <thread>

#include "../network/Socket.h"
#include "../server/Config.h"
#include <atomic>
#include <mutex>
#include <csignal>
#include <vector>
#include <string.h>
#include <map>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/stats.fifo"

namespace server {

struct SharedStatistics {
	// Все данные внутри структуры - для shared memory
	volatile uint64_t totalFiles;
	volatile uint64_t infectedFiles;
	volatile uint64_t totalThreats;
	
	static const int MAX_PATTERNS = 256;
	volatile uint64_t patternCounts[MAX_PATTERNS];
	char patternNames[MAX_PATTERNS][64];
	
	SharedStatistics() {
		totalFiles = 0;
		infectedFiles = 0;
		totalThreats = 0;
		for (int i = 0; i < MAX_PATTERNS; i++) {
			patternCounts[i] = 0;
			patternNames[i][0] = '\0';
		}
	}
	
	void initPatterns(const std::vector<std::string>& patterns) {
		for (size_t i = 0; i < patterns.size() && i < MAX_PATTERNS; i++) {
			strncpy(patternNames[i], patterns[i].c_str(), sizeof(patternNames[i]) - 1);
		}
	}
	
	void addFile(bool infected, int threats) {
		__sync_fetch_and_add(&totalFiles, 1);
		__sync_fetch_and_add(&totalThreats, threats);
		if (infected) {
			__sync_fetch_and_add(&infectedFiles, 1);
		}
	}
	
	void addPattern(int index, int count = 1) {
		if (index >= 0 && index < MAX_PATTERNS) {
			__sync_fetch_and_add(&patternCounts[index], count);
		}
	}
	
	int findPattern(const std::string& name) const {
		for (int i = 0; i < MAX_PATTERNS; i++) {
			if (patternNames[i][0] != '\0' && 
				strcmp(patternNames[i], name.c_str()) == 0) {
				return i;
			}
		}
		return -1;
	}
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
	void* _sharedMem;
	SharedStatistics* _stats;  // Указатель на shared memory

	// Поток для вывода статистики
	std::thread _statsThread;
		void setupSignalHandlers();
	void handleSignal(int signal);
	void acceptConnection();
	void spawnWorker(net::Socket clientSocket);


	void cleanup();
	void cleanupWorkers();
	void cleanupSharedMemory();
	void cleanupFifo();
	void cleanupSocket();


	void setupFifo();
	void writeFifo();

	void setupSharedMemory();		   // Создание shared memory
	void statisticsLoop();			  // Поток для вывода статистики
	void printStatistics();  

	// ссылка на инстанс для обработки сигналов
	static Server *_instance;
	static void signalHandler(int signal);

	static struct Statistics stats;

	int _statsFifoFd;
};

} // namespace server
