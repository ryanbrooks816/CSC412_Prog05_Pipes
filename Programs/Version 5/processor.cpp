#include <iostream>
#include <vector>
#include <string>
#include "client.h"
#include "testing.h"

int main(int argc, char *argv[])
{  
    // Parent-to-child pipe (write end)
    int writePipeFd = std::stoi(argv[1]);

    // Get the client index
    int clientIdx = std::stoi(argv[2]);

    // Get the number of files
    int numFiles = std::stoi(argv[3]);

    // Get the list of files
    std::vector<std::string> files = std::vector<std::string>(numFiles);

    // Create a client object
    Client client(clientIdx);

    // Update the list of files
    for (int i = 0; i < numFiles; i++)
    {
        files[i] = argv[i + 4];
    }

    client.setFiles(files); // Set the list of verified files once again

    // Process the data files and reconstruct the block of code
    client.processDataFiles(writePipeFd);

    // Since we're using the same pipe Fd that has been sent through the process chain
    // we'll write to that, and the distribor parent process will handle signaling the
    // server to process these results in the same way it signaled before.

    DEBUG_FILE("(processor " + std::to_string(clientIdx) + ") Processed data files", "debug.log");

    return 0;
}
