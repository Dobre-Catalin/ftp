#include "ServerController.h"
#include <iostream>

/*
 * constructor
 * Initializes the FTPClient object with the server address and port.
 * Takes parameters:
 * - serverAddress: the IP address of the server
 * - serverPort: the port number of the server
 * Returns void.
 */
ServerController::ServerController(const std::string& serverAddress, int serverPort)
    : client(serverAddress, serverPort) {}

// Destructor
ServerController::~ServerController() {
    // Cleanup can be done here if needed
}

/*
 * downloadFileValid function
 * checks if the path is not empty
 * checks if the path is safe from traversal attacks
 * Returns true if the path is valid, false otherwise.
 */
bool ServerController::downloadFileValid(const std::string &remotePath) {
    if (remotePath.empty()) {
        std::cerr << "Invalid path: path is empty" << std::endl;
        return false;
    }
    std::string unpermitted = "/\\:*?\"<>|";
    for (char c : unpermitted) {
        if (remotePath.find(c) != std::string::npos) {
            std::cerr << "Invalid character in remote path: " << c << std::endl;
            return false;
        }
    }
    return true;
}

/*
 * login function
 * Logs in to the server with the given username and password.
 * Takes parameters:
 * - username: the username to log in with
 * - password: the password to log in with
 * Returns void.
 * The function sends the USER and PASS commands to the server to log in.
 * It catches any exceptions thrown by the FTPClient object and prints an error message.
 */
void ServerController::login(const std::string& username, const std::string& password) {
    try {
        client.user(username);
        client.pass(password);
    } catch (const std::exception& ex) {
        std::cerr << "Login failed: " << ex.what() << std::endl;
    }
}

/*
 * listFiles function
 * Lists the files in the current directory on the server.
 * Returns void.
 */
void ServerController::listFiles() {
    try {
        client.listFiles();
    } catch (const std::exception& ex) {
        std::cerr << "Failed to list files: " << ex.what() << std::endl;
    }
}

/*
 * uploadFile function
 * Uploads a file to the server.
 * Takes parameters:
 * - localPath: the local path of the file to upload
 * - remotePath: the remote path where the file will be uploaded on the server
 * Returns void.
 * The function catches any exceptions thrown by the FTPClient object and prints an error message.
 */
void ServerController::uploadFile(const std::string& localPath, const std::string& remotePath) {
    try {
        //upload the file
        client.uploadFile(localPath, remotePath);
    } catch (const std::exception& ex) {
        std::cerr << "Failed to upload file: " << ex.what() << std::endl;
    }
}

/*
 * downloadFile function
 * Downloads a file from the server.
 * Takes parameters:
 * - remotePath: the remote path of the file to download
 * - localPath: the local path where the file will be saved
 * Returns void.
 * The function catches any exceptions thrown by the FTPClient object and prints an error message.
 */
void ServerController::downloadFile(const std::string& remotePath, const std::string& localPath) {

    if (downloadFileValid(remotePath) == false) {
        std::cerr<<"Invalid path"<<std::endl;
        return;
    }

    try {
        client.downloadFile(remotePath, localPath);
    } catch (const std::exception& ex) {
        std::cerr << "Failed to download file: " << ex.what() << std::endl;
    }
}

/*
 * logout function
 * Logs out the user from the server.
 * Returns void.
 * The function catches any exceptions thrown by the FTPClient object and prints an error message.
 */
void ServerController::logout() {
    try {
        client.logout();
    } catch (const std::exception& ex) {
        std::cerr << "Logout failed: " << ex.what() << std::endl;
    }
}
