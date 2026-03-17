#include <gtest/gtest.h>
#include "network/Protocol.h"
#include <string>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

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
        protocol::MessageHeader header;
	uint32_t size = 0;
        recv(serverFd, &size, sizeof(header.size), 0);
        uint32_t receivedSize = ntohl(header.size);
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
