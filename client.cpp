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
        ssize_t bytes = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            std::cout << "\nDisconnected from server.\n";
            break;
        }
        buffer[bytes] = '\0';
        std::cout << "\n" << buffer << "\n> " << std::flush;
    }
}

int main() {
    // Prompt for name before connecting
    std::cout << "Please enter a name: ";
    std::string client_name;
    std::getline(std::cin, client_name);

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5555);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        return 1;
    }

    if (connect(sock_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    std::cout << "Connected to server. Type /quit to exit.\n";

    // Send client name to server
    if (send(sock_fd, client_name.c_str(), client_name.size(), 0) < 0) {
        perror("send");
        close(sock_fd);
        return 1;
    }

    // Start receiving messages in background thread
    std::thread(receive_loop, sock_fd).detach();

    std::string msg;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, msg)) break; // Handle EOF
        if (msg == "/quit") break;

        if (send(sock_fd, msg.c_str(), msg.size(), 0) < 0) {
            perror("send");
            break;
        }
    }

    close(sock_fd);
    return 0;
}
