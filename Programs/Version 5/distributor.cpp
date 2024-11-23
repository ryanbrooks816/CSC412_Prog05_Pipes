#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include "client.h"
#include "testing.h"

/**
 * @brief Reads incoming file paths from the server over the current pipe and adds them 
 * to the client's file list.
 *
 * This function continuously reads messages from the specified read pipe file descriptor.
 * Each message consists of a size followed by the actual content (file path). The function
 * handles the following scenarios:
 * - End of pipe: Stops reading when no more data is available.
 * - Partial read or error: Logs an error message and exits with a specific error code.
 * - "DONE" signal: Stops reading when a message with size 0 is received.
 *
 * @param client The client object to which the file paths will be added.
 * @param readPipeFd The file descriptor of the read pipe.
 * @param clientIdx The index of the client, used for logging purposes.
 */
void readIncomingFiles(Client &client, int readPipeFd, int clientIdx)
{
    while (true)
    {
        // Read the message size first
        size_t messageSize;
        ssize_t bytesRead = read(readPipeFd, &messageSize, sizeof(messageSize));

        if (bytesRead == 0)
        {
            // End of the pipe; no more data to read
            break;
        }
        else if (bytesRead != sizeof(messageSize))
        {
            // Handle partial read or error
            DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Error reading message size from server", "debug.log");
            exit(160);
        }

        // Check if the message is a "DONE" signal
        if (messageSize == 0)
        {
            DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Received DONE signal from server", "debug.log");
            break;
        }

        // Read the actual message content (file path)
        std::vector<char> buffer(messageSize);
        bytesRead = read(readPipeFd, buffer.data(), messageSize);

        if (bytesRead != static_cast<ssize_t>(messageSize))
        {
            DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Error reading message content from server", "debug.log");
            exit(161);
        }

        // Parse te message to get the file path
        std::string message(buffer.begin(), buffer.end());
        DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Received message: " + message, "debug.log");

        client.addFile(message);
    }
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
    // client index and file index to the pipe so the server can figure out where to
    // send the incorrectly distributed files.
    client.verifyDataFilesDistribution(numClients, files, writePipeFd);

    // Indicate to the parent process that the client has finished verifying the files
    size_t doneSignal = 0;
    write(writePipeFd, &doneSignal, sizeof(doneSignal));

    // Wait until all clients have finished verifying their data files.
    // Server processes a list of data files and sends them to the correct distributor processes.
    // along with a signal so the distributor processes can proceed with updating its
    // list of files and starting the processing of the data files.
    readIncomingFiles(client, readPipeFd, clientIdx);

    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Verified data files distribution", "debug.log");

    // Start data processing, where the processor process will go through the list of files
    // and sort the lines based on their line numbers to ensure the correct order.
    // It will finally constuct a block of code from the sorted lines and send the results
    // via the same pipes to the server.
    client.initializeProcessor(writePipeFd);
    DEBUG_FILE("(distributor " + std::to_string(clientIdx) + ") Finished processing data files", "debug.log");

    // Grandchild processor process has finished processing the data files and has sent
    // the reconstructed code block to over this pipe.

    // The server is waiting for its children to finish processing the data files at this
    // point, so when this process finishes, the server will know data processing is complete.

    return 0;
}
