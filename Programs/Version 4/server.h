#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
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
     * @brief Destroys the Server object.
     *
     * This destructor cleans up any resources used by the Server object.
     */
    ~Server();

    /**
     * @brief Distributes a list of data files evenly among the clients.
     *
     * This function takes a vector of file paths and distributes them among the available
     * clients. Each client is assigned a range of files to process. The distribution
     * ensures that any remainder files are distributed one per client until exhausted.
     *
     * @param files A vector of strings representing the names of the data files to be distributed.
     */
    void distributeDataFiles(const std::vector<std::string> &files);

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
     *
     * @param files A vector of strings representing the names of the data files
     */
    void verifyDataFilesDistribution(const std::vector<std::string> &files);

    /**
     * @brief Reads temporary files created by child processes during the data distribution
     * processand updates the clients' file lists.
     *
     * This function iterates over the number of clients and attempts to open a corresponding
     * temporary file for each client. Expects the temporary files to be named and formatted
     * in a specific way. Then it updates the clients' file lists with the appropriate files.
     *
     * @throws std::runtime_error if a temporary file cannot be opened.
     */
    void readDistributorTempFiles();

    /**
     * @brief Processes data files for each client and combines the results.
     *
     * Processes each client's data files by reading the temporary files created by the first
     * generation of child processes and then updates the clients' file lists. Then it forks
     * another child process to handle data processing. Each child process will launch their
     * own program to process the data files, reconstructing the block of code for each
     * client and writes the results to a temporary file (sch_<clientIdx>.txt).
     *
     * @return A string containing the combined results from processing each client's
     * data files.
     */
    std::string processDataFiles();

    /**
     * @brief Reads temporary files created by child processes during the data processing
     * process and combines the results.
     *
     * This function iterates over the number of clients and attempts to open a corresponding
     * temporary file for each client. Expects the temporary files to be named and formatted
     * in a specific way. Then it reads the contents of each file and combines them into
     * a single string representing the complete code block.
     *
     * @return A string containing the combined results from processing each client's
     * data files.
     *
     * @throws std::runtime_error if a temporary file cannot be opened.
     */
    std::string readDataProcessingTempFiles();

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
};

#endif // SERVER_H
