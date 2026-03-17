#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <string>

#define FIFO_PATH "/tmp/stats.fifo"

// stats_once.cpp
int main() {
    int fd = open(FIFO_PATH, O_RDONLY);
    char buffer[4096];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    std::cout<<"jopa\n";
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::cout << buffer;
    }
    close(fd);
    return 0;
}
