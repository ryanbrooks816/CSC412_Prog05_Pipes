#include <iostream>
#include <vector>
#include <string>
#include "client.h"
#include "testing.h"

int main(int argc, char *argv[])
{
    // Get the number of clients
    int numClients = std::stoi(argv[1]);

    // Get the client index
    int clientIdx = std::stoi(argv[2]);

    // Get the files start and end index
    // (to replicate the data of the client's in original process)
    int filesStartIdx = std::stoi(argv[3]);
    int filesEndIdx = std::stoi(argv[4]);

    // Get the list of files
    std::vector<std::string> files = std::vector<std::string>(filesEndIdx - filesStartIdx);

    for (int i = 5; i < argc; i++)
    {
        files[i - 5] = argv[i];
    }

    // Create a client object
    Client client(clientIdx, filesStartIdx, filesEndIdx);

    // Verify the distribution of data files
    client.verifyDataFilesDistribution(numClients, files);

    return 0;
}
