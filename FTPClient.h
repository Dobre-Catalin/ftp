#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <stdexcept>

class FTPClient {
private:
    int controlSocket;
    std::string serverAddress;
    int serverPort;

    int createSocket();
    void sendCommand(const std::string& cmd) const;
    std::string readResponse() const;
    int enterPassiveMode();

public:
    FTPClient(const std::string& address, int port);
    ~FTPClient();

    void user(const std::string& username);
    void pass(const std::string& password);
    void logout();
    void uploadFile(const std::string& localPath, const std::string& remotePath);
    void downloadFile(const std::string& remotePath, const std::string& localPath);
    void listFiles();

    bool checkResponseCode(const std::string &response, const std::string &expectedCode);
};