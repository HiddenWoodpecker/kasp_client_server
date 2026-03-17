#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include "../include/server/Server.h"
#include <thread>

// совпадение отправленной и полученной длины
TEST(LengthTest, FileSizeMatch) {
	int sv[2];
	ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);
	int serverFd = sv[0];
	int clientFd = sv[1];
	std::string content = "Hello World!";
	uint32_t originalSize = content.length();
	uint32_t networkSize = htonl(originalSize);
	std::thread serverThread([serverFd, originalSize]() {
	uint32_t size = 0;
		recv(serverFd, &size, sizeof(size), 0);
		uint32_t receivedSize = ntohl(size);
		EXPECT_EQ(receivedSize, originalSize); 
		close(serverFd);
	});
	send(clientFd, &networkSize, sizeof(networkSize), 0);
	send(clientFd, content.c_str(), content.length(), 0);
	close(clientFd);
	serverThread.join();
}

// проверка длины для пустого файла
TEST(LengthTest, ZeroLength) {
	int sv[2];
	ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);
	int serverFd = sv[0];
	int clientFd = sv[1];
	uint32_t networkZero = htonl(0);
	std::thread serverThread([serverFd]() {
		uint32_t size = 0;
		recv(serverFd, &size, sizeof(size), 0);
		uint32_t receivedSize = ntohl(size);
		EXPECT_EQ(receivedSize, 0); 
		close(serverFd);
	});
	send(clientFd, &networkZero, sizeof(networkZero), 0);

	close(clientFd);
	serverThread.join();
}

//проверка правильная ли статистика в shared memory
class StatisticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        
        mem = mmap(nullptr, sizeof(server::SharedStatistics),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS,
                   -1, 0);
        ASSERT_NE(mem, MAP_FAILED);
        
        stats = new (mem) server::SharedStatistics();
        
        std::vector<std::string> patterns = {"VIRUS", "KEYLOGGER", "CRYPTOMINER"};
        stats->initPatterns(patterns);
    }

    void TearDown() override {
        munmap(mem, sizeof(server::SharedStatistics));
    }

    void* mem;
    server::SharedStatistics* stats;
};

TEST_F(StatisticsTest, InitialValues) {
    EXPECT_EQ(stats->totalFiles, 0);
    EXPECT_EQ(stats->infectedFiles, 0);
    EXPECT_EQ(stats->totalThreats, 0);
}

TEST_F(StatisticsTest, AddFile) {
    stats->addFile(false, 0);
    EXPECT_EQ(stats->totalFiles, 1);
    EXPECT_EQ(stats->infectedFiles, 0);
    EXPECT_EQ(stats->totalThreats, 0);

    stats->addFile(true, 5);
    EXPECT_EQ(stats->totalFiles, 2);
    EXPECT_EQ(stats->infectedFiles, 1);
    EXPECT_EQ(stats->totalThreats, 5);
}

TEST_F(StatisticsTest, FindPattern) {
    EXPECT_EQ(stats->findPattern("VIRUS"), 0);
    EXPECT_EQ(stats->findPattern("KEYLOGGER"), 1);
    EXPECT_EQ(stats->findPattern("CRYPTOMINER"), 2);
    EXPECT_EQ(stats->findPattern("UNKNOWN"), -1);
}

TEST_F(StatisticsTest, AddPattern) {
    stats->addPattern(0, 3);  // VIRUS +3
    stats->addPattern(1, 2);  // KEYLOGGER +2
    stats->addPattern(0, 1);  // VIRUS +1

    EXPECT_EQ(stats->patternCounts[0], 4);
    EXPECT_EQ(stats->patternCounts[1], 2);
    EXPECT_EQ(stats->patternCounts[2], 0);
}

TEST_F(StatisticsTest, ConcurrentUpdates) {
    const int NUM_THREADS = 100;
    const int UPDATES_PER_THREAD = 100;
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([this]() {
            for (int j = 0; j < UPDATES_PER_THREAD; j++) {
                stats->addFile(true, 1);
                stats->addPattern(0, 1);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    EXPECT_EQ(stats->totalFiles, NUM_THREADS * UPDATES_PER_THREAD);
    EXPECT_EQ(stats->infectedFiles, NUM_THREADS * UPDATES_PER_THREAD);
    EXPECT_EQ(stats->patternCounts[0], NUM_THREADS * UPDATES_PER_THREAD);
}


