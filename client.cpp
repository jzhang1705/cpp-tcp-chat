// client.cpp
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void receive_loop(int socket_fd) {
    char buffer[1024];
    while (true) {
        ssize_t bytes = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        std::cout << "\n" << buffer << "\n> " << std::flush;
    }
}

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{
        .sin_family = AF_INET,
        .sin_port = htons(5555),
    };
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    std::cout << "Connected to server. Type /quit to exit.\n";
    std::thread(receive_loop, sock_fd).detach();

    std::string msg;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, msg);
        if (msg == "/quit") break;
        send(sock_fd, msg.c_str(), msg.size(), 0);
    }

    close(sock_fd);
    return 0;
}
