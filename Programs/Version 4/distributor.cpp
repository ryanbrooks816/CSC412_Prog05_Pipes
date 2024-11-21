#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include "client.h"
#include "testing.h"

/**
 * Signals the parent process to proceed by writing the client index to the specified
 * pipe's file descriptor.
 *
 * @param writePipeFd The file descriptor of the write end of the pipe.
 * @param clientIdx The index of the client to be written to the pipe.
 */
void signalParent(int writePipeFd, int clientIdx)
{
    if (write(writePipeFd, &clientIdx, sizeof(clientIdx)) == -1)
    {
        perror("write failed");
    }
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Signaled parent to proceed", "debug.log");
}

/**
 * Waits for a signal from the parent process by reading from the specified pipe's file
 * descriptor. The function blocks until a signal is received.
 *
 * @param readPipeFd The file descriptor of the read end of the pipe.
 * @param clientIdx The index of the client.
 */
void waitForParentSignal(int readPipeFd, int clientIdx)
{
    int signal;
    read(readPipeFd, &signal, sizeof(signal)); // Blocking read
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Received signal from parent", "debug.log");
}

int main(int argc, char *argv[])
{
    // Just check for safety purposes; we can have many more arguments due to the file paths
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

    // Handle the main data distribution to verify the distribution of data files 
    // among clients by reading the process index from the file and writing the correct
    // client index and file index to a temporary file. A chuld process of the current
    // distributor process will eventually read the files.

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

    // Initialize the processor process to sort and combine the data files contents
    // Writes the results to a temporary file
    client.initializeProcessor();
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Finished processing data files", "debug.log");

    // Data processing has finished for this client, so we can read the temporary file 
    // created by the child process and create the combined code block
    std::string combinedResult = client.readDataProcessingTempFile();

    // Send combined result back to the parent (the server)
    size_t resultSize = combinedResult.size();
    write(writePipeFd, &resultSize, sizeof(resultSize));    // Send the size of the result
    write(writePipeFd, combinedResult.c_str(), resultSize); // Send the actual result
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Sent combined result to parent", "debug.log");

    close(readPipeFd);  // Close read end
    close(writePipeFd); // Close write end

    return 0;
}
