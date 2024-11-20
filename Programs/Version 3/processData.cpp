#include <iostream>
#include <vector>
#include <string>
#include "client.h"
#include "testing.h"

int main(int argc, char *argv[])
{
    // Get the client index
    int clientIdx = std::stoi(argv[1]);

    // Get the number of files
    int numFiles = std::stoi(argv[2]);

    // Get the list of files
    std::vector<std::string> files = std::vector<std::string>(numFiles);

    // Create a client object
    Client client(clientIdx);

    // Update the list of files
    for (int i = 0; i < numFiles; i++)
    {
        files[i] = argv[i + 3];
    }

    client.setFiles(files); // Set the list of verified files once again

    // Process the data files and reconstruct the block of code
    client.processDataFiles();

    return 0;
}
