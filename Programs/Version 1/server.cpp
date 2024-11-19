#include "server.h"
#include "testing.h"

/**
 * @brief Constructs a new Server object.
 *
 * This constructor initializes a Server object and creates the specified number of
 * client objects.
 *
 * @param numClients The number of clients to be managed by the server.
 */
Server::Server(int numClients)
{
    this->numClients = numClients;
    std::vector<Client> clients;
    for (int i = 0; i < numClients; i++)
    {
        clients.push_back(Client(i));
    }
    this->clients = clients;
    DEBUG_FILE("Server created with " + std::to_string(numClients) + " clients.", "debug.log");
}

/**
 * @brief Distributes data files evenly among clients to be processed.
 *
 * Retrieves all data files from the specified folder and distributes them evenly
 * among the number of clients. If the number of files is not perfectly divisible
 * by the number of clients, the remainder files are spread among that number of
 * clients.
 *
 * @param dataFolder The folder containing the data files to be distributed.
 */
void Server::distributeDataFiles(std::string dataFolder)
{
    // Get all the data files
    std::vector<std::string> files = getAllDataFiles(dataFolder);
    DEBUG_FILE("Retrieved " + std::to_string(files.size()) + " data files.", "debug.log");

    // Evenly distribute the files into the clients in a round-robin fashion
    int clientIdx = 0;
    for (size_t i = 0; i < files.size(); i++)
    {
        std::string file = files[i];

        std::string message = "Adding file " + file + " to client " + std::to_string(clientIdx);
        DEBUG_FILE(message, "debug.log");

        // Add the file to the client's list
        this->clients[clientIdx].addFile(file);

        // Move to the next client
        clientIdx = (clientIdx + 1) % numClients;
    }

    DEBUG_FILE("Distributed data files to clients.", "debug.log");
}

/**
 * @brief Retrieves all regular files from the specified folder.
 *
 * This function iterates through the directory specified by the folderPath
 * and collects the paths of all regular files into a vector of strings.
 *
 * @param folderPath The path to the folder from which to retrieve file paths.
 * @return std::vector<std::string> A vector containing the paths of all regular files in the specified folder.
 */
std::vector<std::string> Server::getAllDataFiles(const std::string &folderPath)
{
    std::vector<std::string> files;

    for (const auto &entry : std::filesystem::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            files.push_back(entry.path().string());
        }
    }
    return files;
}

/**
 * @brief Verifies the distribution of data files among clients and performs the work
 * of the distributor processes in this version.
 *
 * This function goes through each client and verifies if each client has received
 * the correct data files. If it finds a data file that doesn't belong to the client,
 * it will send the data to the correct client by adding the file to the appropriate
 * client's list.
 *
 * Invariants: The script launching the server process has correctly retrieved the
 * highest process index to properly distribute the data files. Otherwise, when veryfing
 * the distribution, it will try to access an index that does not exist.
 */
void Server::verifyDataFilesDistribution()
{
    // Use a vector of vectors to store the verified data files for each client
    std::vector<std::vector<std::string>> verifiedFiles = std::vector<std::vector<std::string>>(numClients);

    for (Client &client : clients)
    {
        DEBUG_FILE("Verifying data files for client " + std::to_string(client.getClientIdx()), "debug.log");

        // Verify that the client has received the correct data files. If not, send the data file to the correct client
        for (const std::string &file : client.getFiles())
        {
            std::string message = "Verifying: " + file;
            DEBUG_FILE(message, "debug.log");

            // Get the first part of the line up until the first space to determine the process index
            int processIndex = client.getDataFileProcessIdx(file);

            // In this version, just add the file to the appropriate client's list
            DEBUG_FILE("Adding file to client " + std::to_string(processIndex), "debug.log");
            verifiedFiles[processIndex].push_back(file);
        }
    }

    // Update the list of files for each client with the verified files
    for (int i = 0; i < numClients; i++)
    {
        this->clients[i].setFiles(verifiedFiles[i]);
    }

    DEBUG_FILE("Verified data files distribution.", "debug.log");
}

/**
 * @brief Processes data files for each client and combines the results.
 *
 * Processes each client's data files by reconstructing the block of code for each
 * client and returns the combined results.
 *
 * @return A string containing the combined results from processing each client's
 * data files.
 */
std::string Server::processDataFiles()
{
    std::string combinedResult;

    // For each client, process the data files, which produces the reconstructed block of code
    // and stack the blocks of code in order of the client index
    for (auto client : clients)
    {
        std::string result = client.processDataFiles();
        combinedResult += result;
    }

    return combinedResult;
}

/**
 * @brief Writes the given content to the specified output file.
 *
 * This function attempts to open the specified output file and write the provided
 * content to it. If the file cannot be opened, an error message is printed to std::cerr.
 *
 * @param outputFile The path to the output file where the content will be written.
 * @param content The content to be written to the output file.
 */
void Server::writeOutputFile(const std::string &outputFile, const std::string &content)
{
    std::string finalOutputFile = outputFile;
    if (finalOutputFile.substr(finalOutputFile.find_last_of(".") + 1) != "c")
    {
        finalOutputFile += ".c";
    }

    std::ofstream file(finalOutputFile);
    if (file.is_open())
    {
        file << content;
        file.close();
    }
    else
    {
        std::cerr << "Error opening output file: " << finalOutputFile << std::endl;
    }
}
