// server.cpp
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

std::vector<int> clients;
std::mutex clients_mutex;

void handle_client(int client_fd) {
    char buffer[1024];

    while (true) {
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            std::cerr << "Client disconnected\n";
            close(client_fd);
            std::scoped_lock lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());
            break;
        }

        buffer[bytes] = '\0';
        std::string message = std::to_string(client_fd) + ' ';
        message += buffer;

        std::scoped_lock lock(clients_mutex);
        std::cout<<message.c_str()<<'\n';
        for (int other_fd : clients) {
            if (other_fd != client_fd) {
                send(other_fd, message.c_str(), message.size(), 0);
            }
        }
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{
        .sin_family = AF_INET,
        .sin_port = htons(5555),
        .sin_addr = { .s_addr = INADDR_ANY }
    };

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "Server listening on port 5555...\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        {
            std::scoped_lock lock(clients_mutex);
            clients.push_back(client_fd);
        }

        std::thread(handle_client, client_fd).detach();
    }

    close(server_fd);
    return 0;
}
