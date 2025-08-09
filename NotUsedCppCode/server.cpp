#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

constexpr int PORT = 5555;
constexpr int BUFFER_SIZE = 1024;

std::unordered_map<int, std::string> clients; // client_fd → name
std::mutex clients_mutex;

void remove_client(int client_fd) {
    {
        std::scoped_lock lock(clients_mutex);
        std::cout << clients[client_fd] << " has disconnected.\n";
        clients.erase(client_fd);
    }
    close(client_fd);
}

void broadcast_message(const std::string &message, int sender_fd) {
    std::scoped_lock lock(clients_mutex);
    for (const auto &[fd, name] : clients) {
        if (fd != sender_fd) {
            if (send(fd, message.c_str(), message.size(), 0) == -1) {
                perror("send");
            }
        }
    }
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];

    // Get client's name first
    ssize_t name_len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (name_len <= 0) {
        close(client_fd);
        return;
    }
    buffer[name_len] = '\0';
    {
        std::scoped_lock lock(clients_mutex);
        clients[client_fd] = buffer;
    }

    std::cout << buffer << " has joined the chat.\n";
    broadcast_message(buffer + std::string(" has joined the chat.\n"), client_fd);

    // Chat loop
    while (true) {
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            remove_client(client_fd);
            break;
        }
        buffer[bytes] = '\0';

        std::string message = clients[client_fd] + ": " + buffer;
        broadcast_message(message, client_fd);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        std::thread(handle_client, client_fd).detach();
    }

    close(server_fd);
    return 0;
}
