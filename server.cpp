// server.cpp
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// std::vector<int> clients;
std::unordered_map<int, std::string> clients;
std::mutex clients_mutex;

void handle_client(int client_fd) {
    char buffer[1024];

    while (true) {
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            {
                std::scoped_lock lock(clients_mutex);
                std::cerr << clients[client_fd] << " disconnected\n";
                clients.erase(client_fd);
            }
            close(client_fd);
            break;
        }

        buffer[bytes] = '\0';
        std::string message = clients[client_fd] + ": ";
        message += buffer;

        std::vector<int> targets;   
        {
            std::scoped_lock lock(clients_mutex);
            std::cout << message << '\n';
            for (auto& [other_fd, name] : clients) {
                if (other_fd != client_fd) {
                    targets.push_back(other_fd);
                }
            }
        }
        for (int fd : targets) {
            send(fd, message.c_str(), message.size(), 0);
        }
    }
}

bool get_client_name(int client_fd) {
    char buffer[1024];
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        std::cerr << "No name, client " << client_fd << " disconnected\n";
        close(client_fd);
        return false;
    }
    buffer[bytes] = '\0';

    std::string name(buffer);
    name.erase(name.find_last_not_of(" \n\r\t") + 1); // trim

    {
        std::scoped_lock lock(clients_mutex);
        clients[client_fd] = name;
    }
    return true;
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

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

    if (get_client_name(client_fd)) { // make it return bool
        std::thread(handle_client, client_fd).detach();
    }
    }

    close(server_fd);
    return 0;
}
