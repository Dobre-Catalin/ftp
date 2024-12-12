#include "FTPClient.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cerrno>
#include <filesystem>

const int BUFFER_SIZE = 8192;

/*
 * Constructor for the FTPClient class.
 * Initializes the control socket and connects to the server.
 * Throws a runtime_error if the connection fails.
 *
 * Takes parameters:
 * - address: the IP address of the server
 * - port: the port number of the server
 */
FTPClient::FTPClient(const std::string& address, int port) : serverAddress(address), serverPort(port) {
    // Create a new socket
    controlSocket = createSocket();

    // Initialize the server address structure
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(serverPort);

    // Connect to the server
    if (connect(controlSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Failed to connect: " + std::string(strerror(errno)));
    }

    // Print the server's welcome message
    std::cout << readResponse();
}

/*
 *createSocket function
 * Creates a new socket and returns the file descriptor.
 * Throws a runtime_error if the socket creation fails.
 * Returns the file descriptor of the created socket.
 */
int FTPClient::createSocket() {
    // Create a new socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    // Check for errors
    if (s < 0) {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }
    // Return the file descriptor of the created socket
    return s;
}

/*
 * sendCommand function
 * Sends a command to the server over the control socket.
 * Throws a runtime_error if the send operation fails.
 * Takes a string parameter cmd representing the command to send.
 * Returns void.
 */
void FTPClient::sendCommand(const std::string& cmd) const {
    // Append \r\n to the command and send it to the server
    std::string message = cmd + "\r\n";
    // Send the command to the server and check for errors
    if (send(controlSocket, message.c_str(), message.length(), 0) < 0) {
        // Throw an exception if the send operation fails
        throw std::runtime_error("Failed to send command: " + std::string(strerror(errno)));
    }
}

std::string FTPClient::readResponse() const {
    // Read the server's response from the control socket
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(controlSocket, buffer, BUFFER_SIZE - 1, 0);
    // Check for errors
    if (bytesReceived < 0) {
        throw std::runtime_error("Failed to read response: " + std::string(strerror(errno)));
    }
    // Null-terminate the buffer
    buffer[bytesReceived] = '\0';
    // Return the response as a string
    return std::string(buffer);
}

/*
 * checkResponseCode function
 * Checks if the response code from the server matches the expected code.
 * Takes parameters:
 * - response: the response from the server
 * - expectedCode: the expected response code
 * Returns true if the response code matches the expected code, false otherwise.
 * Prints an error message if the response code does not match the expected code.
 * Returns a boolean value.
 */
bool FTPClient::checkResponseCode(const std::string& response, const std::string& expectedCode) {
    // Check if the response code matches the expected code
    if (response.substr(0, 3) != expectedCode) {
        // Print an error message if the response code does not match the expected code
        std::cerr << "Expected response starting with " << expectedCode << ", but got: " << response << std::endl;
        return false;
    }
    return true;
}

/*
 * enterPassiveMode function
 * Enters passive mode and returns the data socket for data transfer.
 * Throws a runtime_error if entering passive mode fails.
 * Returns the file descriptor of the data socket.
 * The function sends the PASV command to the server and parses the response to obtain the IP address and port for the data connection.
 * It then creates a new socket and connects to the server using the obtained IP address and port.
 * The function returns the file descriptor of the data socket for data transfer.
 * The function throws a runtime_error if the connection to the data socket fails.
 * The function also throws a runtime_error if the response from the server does not match the expected format.
 * The function uses the readResponse and checkResponseCode functions to read and validate the server's response.
 * The function returns the file descriptor of the data socket for data transfer.
 */
int FTPClient::enterPassiveMode() {
    // Send the PASV command to the server
    sendCommand("PASV");
    // Read the server's response
    std::string response = readResponse();
    // Check if the response code is 227 (Entering Passive Mode)
    if (!checkResponseCode(response, "227")) {
        throw std::runtime_error("Failed to enter passive mode: " + response);
    }

    // Parse the IP address and port from the response
    size_t start = response.find('(');
    size_t end = response.find(')', start);
    std::string data = response.substr(start + 1, end - start - 1);

    // Split the IP address and port values
    std::vector<int> values;
    size_t pos = 0;
    while ((pos = data.find(',')) != std::string::npos) {
        values.push_back(std::stoi(data.substr(0, pos)));
        data.erase(0, pos + 1);
    }
    values.push_back(std::stoi(data));

    // Construct the IP address and port
    std::string ip = std::to_string(values[0]) + "." + std::to_string(values[1]) + "." +
                     std::to_string(values[2]) + "." + std::to_string(values[3]);
    int port = values[4] * 256 + values[5];

    // Initialize the data address structure
    sockaddr_in dataAddr = {};
    dataAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &dataAddr.sin_addr);
    dataAddr.sin_port = htons(port);

    // Create a new socket for the data connection
    int dataSocket = createSocket();
    if (connect(dataSocket, (sockaddr*)&dataAddr, sizeof(dataAddr)) < 0) {
        throw std::runtime_error("Failed to connect to data socket: " + std::string(strerror(errno)));
    }

    // Return the file descriptor of the data socket
    return dataSocket;
}

/*
 * Destructor for the FTPClient class.
 * Closes the control socket.
 */
FTPClient::~FTPClient() {
    // Close the control socket
    close(controlSocket);
}

/*
 * user function
 * Sends the USER command to the server to authenticate the user.
 * Takes a string parameter username representing the username to send.
 * Returns void.
 */
void FTPClient::user(const std::string& username) {
    // sends USER command used for logging in.
    sendCommand("USER " + username);
    std::cout << readResponse();
}

/*
 * pass function
 * Sends the PASS command to the server to authenticate the user.
 * Takes a string parameter password representing the password to send.
 * Returns void.
 */
void FTPClient::pass(const std::string& password) {
    // Analog to the user function, sends the password to the server.
    sendCommand("PASS " + password);
    std::cout << readResponse();
}

/*
 * logout function
 * Sends the QUIT command to the server to log out the user.
 * Returns void.
 */
void FTPClient::logout() {
    // Sends the QUIT command to the server to log out the user.
    sendCommand("QUIT");
    std::cout << readResponse();
}

/*
 * uploadFile function
 * Uploads a file to the server.
 * Takes parameters:
 * - localPath: the local path of the file to upload
 * - remotePath: the remote path where the file will be uploaded on the server
 * Throws a runtime_error if the file does not exist or if the upload fails.
 * Returns void.
 * The function first checks if the 'drive' directory exists in the current directory.
 * It then constructs the full local path by appending the localPath to the 'drive' directory.
 * The function checks if the file exists and is not a directory.
 * It then opens the file in binary mode for reading.
 * The function enters passive mode and obtains the data socket for data transfer.
 * It sends the STOR command to the server with the remote path.
 * The function reads the server's response and checks if the response code is 150 (File status okay).
 * The function then reads the file in chunks and sends the data to the server using the data socket.
 * The function closes the file and the data socket after the upload is complete.
 */
void FTPClient::uploadFile(const std::string& localPath, const std::string& remotePath) {


    const std::string driveFolder = "drive";  // Define the 'drive' directory name

    if (!std::filesystem::exists(driveFolder)) {
        throw std::runtime_error("Directory 'drive' does not exist.");
    }

    std::string fullLocalPath = driveFolder + "/" + localPath;

    if (!std::filesystem::exists(fullLocalPath) || std::filesystem::is_directory(fullLocalPath)) {
        throw std::runtime_error("File not found or invalid path: " + fullLocalPath);
    }

    std::ifstream file(fullLocalPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + fullLocalPath);
    }

    int dataSocket = enterPassiveMode();
    sendCommand("STOR " + remotePath);
    std::string response = readResponse();

    if (!checkResponseCode(response, "150") && !checkResponseCode(response, "125")) {
        throw std::runtime_error("Failed to initiate file upload: " + response);
    }

    std::cout << "Starting file upload: " << fullLocalPath << " to " << remotePath << std::endl;

    char buffer[BUFFER_SIZE];
    while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
        int bytesToSend = static_cast<int>(file.gcount());
        int bytesSent = 0;

        // Continue sending until all bytes are transmitted
        while (bytesSent < bytesToSend) {
            int sent = send(dataSocket, buffer + bytesSent, bytesToSend - bytesSent, 0);
            if (sent < 0) {
                throw std::runtime_error("Failed to send file data: " + std::string(strerror(errno)));
            }
            bytesSent += sent;
        }
    }

    file.close();
    close(dataSocket);

    response = readResponse();
    if (!checkResponseCode(response, "226") && !checkResponseCode(response, "250")) {
        throw std::runtime_error("File upload failed: " + response);
    }

    std::cout << "File uploaded successfully: " << remotePath << std::endl;
}


/*
 * downloadFile function
 * Downloads a file from the server.
 * Takes parameters:
 * - remotePath: the remote path of the file to download
 * - localPath: the local path where the file will be saved
 * Throws a runtime_error if the download fails.
 * Returns void.
 * The function enters passive mode and obtains the data socket for data transfer.
 * It sends the RETR command to the server with the remote path.
 * The function reads the server's response and checks if the response code is 226 (Closing data connection).
 * The function then reads the data from the data socket and writes it to a file in binary mode.
 * The function closes the file and the data socket after the download is complete.
 */
void FTPClient::downloadFile(const std::string& remotePath, const std::string& localPath) {
    // Check if the 'drive' directory exists
    const std::string driveFolder = "drive";

    // Create the 'drive' directory if it does not exist
    if (!std::filesystem::exists(driveFolder)) {
        std::filesystem::create_directory(driveFolder);
    }

    // Construct the full local path
    std::string fullLocalPath = driveFolder + "/" + localPath;

    int dataSocket = enterPassiveMode();

    // Send the RETR command to the server
    sendCommand("RETR " + remotePath);
    std::cout << readResponse();

    // Open the file for writing in binary mode
    std::ofstream file(fullLocalPath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create file: " + fullLocalPath);
    }

    // Read the data from the data socket and write it to the file
    char buffer[BUFFER_SIZE];
    int bytesRead;
    // Read the data from the data socket and write it to the file
    while ((bytesRead = recv(dataSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        file.write(buffer, bytesRead);
    }

    // Close the file and the data socket
    file.close();
    close(dataSocket);

    // Read the final response from the server
    std::string response = readResponse();
    if (!checkResponseCode(response, "226")) {
        throw std::runtime_error("Failed to download file: " + response);
    }

    std::cout << "File downloaded successfully: " << remotePath << std::endl;
}

/*
 * listFiles function
 * Lists the files in the current directory on the server.
 * Returns void.
 * The function enters passive mode and obtains the data socket for data transfer.
 * It sends the LIST command to the server to list the files in the current directory.
 * The function reads the server's response and prints it to the console.
 * The function then reads the data from the data socket and prints it to the console.
 * The function closes the data socket after reading all the data.
 * The function reads the final response from the server and prints it to the console.
 */
void FTPClient::listFiles() {
    // Enter passive mode and obtain the data socket
    int dataSocket = enterPassiveMode();

    // Send the LIST command to the server
    sendCommand("LIST");

    // Print the server's response
    std::cout << readResponse();

    char buffer[BUFFER_SIZE];
    int bytesRead;

    // Receive and print the data from the data socket
    while ((bytesRead = recv(dataSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        std::cout.write(buffer, bytesRead);
    }

    // Close the data connection
    close(dataSocket);

    // Print the final response from the server
    std::cout << readResponse();
}
