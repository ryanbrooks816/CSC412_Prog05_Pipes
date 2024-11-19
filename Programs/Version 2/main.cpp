#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "server.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <highestProcessIdx> <dataFolder> <outputFile>" << std::endl;
        return 26;
    }

    else if (argc > 4)
    {
        std::cerr << "Usage: " << argv[0] << " <highestProcessIdx> <dataFolder> <outputFile>" << std::endl;
        return 27;
    }

    // First argument contains the highest process index
    // Add 1 to represent the number of clients
    // Script running the program has already verified that the highest process index is an integer
    int numClients = std::stoi(argv[1]) + 1;

    // Second argument contains the path to the data folder
    // Script running the program has already verified that the data folder exists
    std::string dataFolder = argv[2];

    // Third argument contains the path to the output file
    std::string outputFile = argv[3];

#ifdef DEBUG
    // Clear the debug log file
    std::ofstream debugFile("debug.log");
    debugFile.close();
#endif

    // Each data file represents a line in a block of code. The line contains
    // the index of the process or "block" it belongs to. The server will distribute
    // the data files to the distributor processes based on this index.
    // The line also contains the line number in the block of code and finally the
    // line of code itself.

    // Launch the server process
    Server server(numClients);

    // Get all the data files from the specified folder and distribute them among the clients
    std::vector<std::string> dataFiles = server.getAllDataFiles(dataFolder);
    server.distributeDataFiles(dataFiles);

    // We then need to verify that the data files have been distributed correctly
    // meaning that each distributor process has received the data files that belong
    // to it. If a data file is found that doesn't belong to the distributor process,
    // it will be sent to the correct distributor process.

    // Verify the distribution of data files
    server.verifyDataFilesDistribution(dataFiles);

    // Then we need to process the data files. Each distributor process will read the
    // data files it has received and sort the lines back into the correct order.
    // It will finally combine the lines back into a block of code.
    // The server then combines each block of code back into the original program.

    // Perform the data processing
    std::string reconstructedCode = server.processDataFiles();

    // Lastly, the server outputs the reconstructed program to a file.
    server.writeOutputFile(outputFile, reconstructedCode);

    return 0;
}
