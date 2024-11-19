#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include "client.h"

class Server
{
public:
    /**
     * @brief Constructs a new Server object.
     *
     * This constructor initializes a Server object and creates the specified number of
     * client objects.
     *
     * @param numClients The number of clients to be managed by the server.
     */
    Server(int numClients);

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
    void distributeDataFiles(std::string dataFolder);

    /**
     * @brief Verifies the distribution of data files among clients and performs the work
     * of the distributor processes in this version.
     *
     * This function goes through each client and verifies if each client has received
     * the correct data files. If it finds a data file that doesn't belong to the client,
     * it will send the data to the correct client by adding the file to the appropriate
     * client's list.
     */
    void verifyDataFilesDistribution();

    /**
     * @brief Processes data files for each client and combines the results.
     *
     * Processes each client's data files by reconstructing the block of code for each
     * client and returns the combined results.
     *
     * @return A string containing the combined results from processing each client's
     * data files.
     */
    std::string processDataFiles();

    /**
     * @brief Writes the given content to the specified output file.
     *
     * This function attempts to open the specified output file and write the provided
     * content to it. If the file cannot be opened, an error message is printed to std::cerr.
     *
     * @param outputFile The path to the output file where the content will be written.
     * @param content The content to be written to the output file.
     */
    void writeOutputFile(const std::string &outputFile, const std::string &content);

private:
    std::vector<Client> clients;
    int numClients;

    /**
     * @brief Retrieves all regular files from the specified folder.
     *
     * This function iterates through the directory specified by the folderPath
     * and collects the paths of all regular files into a vector of strings.
     *
     * @param folderPath The path to the folder from which to retrieve file paths.
     * @return std::vector<std::string> A vector containing the paths of all regular files in the specified folder.
     */
    std::vector<std::string> getAllDataFiles(const std::string &folderPath);
};

#endif // SERVER_H
