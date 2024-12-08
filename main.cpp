#include <iostream>
#include "FTPClient.h"
#include <string>
#include <vector>

std::vector<std::string> getTokens(const std::string& str) {
    std::vector<std::string> tokens;
    size_t start = 0;

    while (start < str.length()) {
        while (start < str.length() && std::isspace(str[start])) {
            ++start; // Skip whitespace characters
        }

        size_t end = start;
        while (end < str.length() && !std::isspace(str[end])) {
            ++end; // Find the next space or end of the string
        }

        if (start < end) {
            tokens.push_back(str.substr(start, end - start));
        }
        start = end;
    }

    return tokens;
}


int main() {
    std::string serverAddress;
    int serverPort;
    std::cout << "Enter the server address: ";
    std::cin >> serverAddress;
    std::cout << "Enter the server port: ";
    std::cin >> serverPort;

    try {
        std::string username, password;
        FTPClient client(serverAddress, serverPort);

        std::cout << "Enter username: ";
        std::cin >> username;
        client.user(username);

        std::cout << "Enter password: ";
        std::cin >> password;
        client.pass(password);

        std::cin.ignore(); // Clear newline left in the input buffer

        while (true) {
            std::string command;
            std::cout << "> ";
            std::getline(std::cin, command);

            if (command.empty()) {
                continue;
            }

            std::vector<std::string> tokens = getTokens(command);

            if (tokens[0] == "list") {
                client.listFiles();
            } else if (tokens[0] == "exit") {
                client.logout();
                break;
            } else if (tokens[0] == "stor" && tokens.size() == 3) {
                client.uploadFile(tokens[1], tokens[2]);
            } else if (tokens[0] == "retr" && tokens.size() == 3) {
                client.downloadFile(tokens[1], tokens[2]);
            } else {
                std::cout << "Invalid command or incorrect arguments." << std::endl;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
