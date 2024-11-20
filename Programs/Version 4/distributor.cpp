#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include "client.h"
#include "testing.h"

void signalParent(int writePipeFd, int clientIdx)
{
    if (write(writePipeFd, &clientIdx, sizeof(clientIdx)) == -1)
    {
        perror("write failed");
    }
    close(writePipeFd); // Close write end
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Signaled parent to proceed", "debug.log");
}

void waitForParentSignal(int readPipeFd, int clientIdx)
{
    int signal;
    read(readPipeFd, &signal, sizeof(signal)); // Blocking read
    close(readPipeFd);                         // Close read end after receiving signal
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Received signal from parent", "debug.log");
}

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        std::cerr << "Usage: " << argv[0] << " <writePipeFd> <readPipeFd> <numClients> <clientIdx> <filesStartIdx> <filesEndIdx> <file1> <file2> ..." << std::endl;
        return 26;
    }

    int writePipeFd = std::stoi(argv[1]); // Parent-to-child pipe (write end)
    int readPipeFd = std::stoi(argv[2]);  // Child-to-parent pipe (read end)
    int numClients = std::stoi(argv[3]);
    int clientIdx = std::stoi(argv[4]);
    int filesStartIdx = std::stoi(argv[5]);
    int filesEndIdx = std::stoi(argv[6]);

    std::vector<std::string> files(filesEndIdx - filesStartIdx);
    for (int i = 7; i < argc; ++i)
    {
        files[i - 7] = argv[i];
    }

    Client client(clientIdx, filesStartIdx, filesEndIdx);

    client.verifyDataFilesDistribution(numClients, files);
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Verified data files distribution", "debug.log");

    signalParent(writePipeFd, clientIdx);
    waitForParentSignal(readPipeFd, clientIdx);

    // Start data processing, where the client reads the data files wirten previously
    // to temporary files and processes them. Each processor process will read the
    // data files it has received and sort the lines back into the correct order.
    // It will finally combine the lines back into a block of code.

    // Read the data files from the distributor step and update the client's file list
    client.readDistributorTempFiles(numClients);
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Read distributor temp files", "debug.log");

    //
    client.initializeProcessor();
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Finished processing data files", "debug.log");
    return 0;
}
