#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include "FTPClient.h"
#include <string>
#include <stdexcept>

class ServerController {
public:
    ServerController(const std::string& serverAddress, int serverPort);
    ~ServerController();

    void login(const std::string& username, const std::string& password);
    void listFiles();
    void uploadFile(const std::string& localPath, const std::string& remotePath);
    void downloadFile(const std::string& remotePath, const std::string& localPath);
    void logout();

private:
    FTPClient client;
};

#endif
