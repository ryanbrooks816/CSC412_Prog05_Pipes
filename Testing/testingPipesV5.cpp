#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>

#define DEBUG_FILE(message, filename) std::cout << message << std::endl

// Client Class that writes to the pipe
class Client
{
public:
    void verifyDataFilesDistribution(int writePipeFd, const std::vector<std::string> &files)
    {
        std::string debugChFile = "debug_ch.log";
        DEBUG_FILE("Verifying data files for client", debugChFile);

        // Go through the specified subset of files and write to the pipe
        for (size_t i = 0; i < files.size(); i++)
        {
            const std::string &file = files[i];

            std::string message = "Verifying: " + file;
            DEBUG_FILE(message, debugChFile);

            // Write the message to the pipe
            int processIdx = 1; // Hardcoded process index for testing purposes
            std::string serverMessage = std::to_string(processIdx) + " " + file;
            size_t messageSize = serverMessage.size();

            DEBUG_FILE("Sending message to server: " + serverMessage, debugChFile);

            // Write the size of the message
            if (write(writePipeFd, &messageSize, sizeof(messageSize)) != sizeof(messageSize))
            {
                DEBUG_FILE("Failed to write message size to pipe", debugChFile);
                exit(158);
            }

            // Write the actual message
            if (write(writePipeFd, serverMessage.c_str(), messageSize) != static_cast<ssize_t>(messageSize))
            {
                DEBUG_FILE("Failed to write message to pipe", debugChFile);
                exit(159);
            }
        }
    }
};

// Server Class that reads from the pipe
class Server
{
public:
    void redistributeVerifiedDataFiles(int readPipeFd)
    {
        std::vector<std::vector<std::string>> incorrectlyDistributedFiles(3);

        while (true)
        {
            size_t messageSize;
            ssize_t bytesRead = read(readPipeFd, &messageSize, sizeof(messageSize));

            if (bytesRead == 0)
            {
                break; // End of pipe
            }
            else if (bytesRead != sizeof(messageSize))
            {
                DEBUG_FILE("Failed to read message size from pipe", "debug.log");
                exit(160);
            }

            std::cout << "Message size: " << messageSize << std::endl;

            std::string result(messageSize, '\0');
            bytesRead = read(readPipeFd, result.data(), messageSize);

            std::cout << "Bytes read: " << bytesRead << std::endl;

            if (bytesRead != static_cast<ssize_t>(messageSize))
            {
                DEBUG_FILE("Failed to read message from pipe", "debug.log");
                exit(161);
            }

            std::cout << "Read from pipe: " << result << std::endl;

            size_t spacePos = result.find(' ');
            if (spacePos != std::string::npos)
            {
                int clientIdx = std::stoi(result.substr(0, spacePos));
                std::string filePath = result.substr(spacePos + 1);
                incorrectlyDistributedFiles[clientIdx].push_back(filePath);
            }
            else
            {
                DEBUG_FILE("Invalid message format received from pipe", "debug.log");
                exit(162);
            }
        }

        std::cout << "Finished reading from pipes" << std::endl;
    }
};

int main()
{
    int pipeFds[2];
    if (pipe(pipeFds) == -1)
    {
        perror("Pipe creation failed");
        return 1;
    }

    // Create a client and server instance
    Client client;
    Server server;

    // Create a vector of hardcoded files
    std::vector<std::string> files = {
        "./Data/Data-Sets/Data-Sets/Large-Data-Sets/DataSet-14--64-procs/f1.txt",
        "./Data/Data-Sets/Data-Sets/Large-Data-Sets/DataSet-14--64-procs/f2.txt",
        "./Data/Data-Sets/Data-Sets/Large-Data-Sets/DataSet-14--64-procs/f3.txt",
        "./Data/Data-Sets/Data-Sets/Large-Data-Sets/DataSet-14--64-procs/f4.txt",
        "./Data/Data-Sets/Data-Sets/Large-Data-Sets/DataSet-14--64-procs/f5.txt"};

    // Fork the process to simulate client-server interaction
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork failed");
        return 1;
    }
    else if (pid == 0)
    {
        // Child process (Client)
        close(pipeFds[0]); // Close unused read end
        client.verifyDataFilesDistribution(pipeFds[1], files);
        close(pipeFds[1]); // Close write end after sending data
    }
    else
    {
        // Parent process (Server)
        close(pipeFds[1]); // Close unused write end
        server.redistributeVerifiedDataFiles(pipeFds[0]);
        close(pipeFds[0]); // Close read end after reading data
    }

    return 0;
}
