#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

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
     * @brief Verifies the distribution of data files among clients and later launches
     * subprocesses for data distribution and processing.
     *
     * This function verifies if each client has received the correct data files by forking
     * a child processes to verify data files. The function creates pipes to communicate between
     * the parent and child processes. Each child process verifies the data files and sends any files
     * that don't belong to the client to the correct client. The parent process waits for all child
     * processes to complete verification and then signals them to proceed wiith processing.
     *
     * @param files A vector of strings representing the data files to be verified.
     */
    std::string initializeDistributor(const std::vector<std::string> &files);

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
     * @brief Runs the distributor child process for a specific client.
     *
     * This function prepares the arguments and executes the distributor program
     * for the specified client. It constructs the argument list based on the
     * client's file indices and the total number of clients.
     *
     * @param i The index of the client for which the distributor process is run.
     * @param writePipeFd The file descriptor for the write end of the pipe.
     * @param readPipeFd The file descriptor for the read end of the pipe.
     * @param files A vector of file paths to be distributed among clients.
     */
    void runDistributorChildProcess(int i, int writePipeFd, int readPipeFd, const std::vector<std::string> &files);

    /**
     * @brief Awaits the distributor child processes to finish verifying the data files.
     *
     * This function waits for all child processes to finish verifying the data files
     * (where they write the correct client index and file index to a temporary file)
     * and signals them to proceed with the distribution verification and processing of
     * the data files.
     *
     * @param childToParentPipes A vector of file descriptors for the read end of the pipes.
     * @param parentToChildPipes A vector of file descriptors for the write end of the pipes.
     */
    void awaitDistributorProcesses(std::vector<int> &childToParentPipes, std::vector<int> &parentToChildPipes);

    /**
     * @brief Collects the combined results from the completed child processes.
     *
     * This function reads the combined blocks of code sent over pipes from the distributor
     * child processes and stores them in a vector of strings. The results are read from the
     * child-to-parent pipes.
     *
     * @param childToParentPipes A vector of file descriptors for the read end of the pipes.
     * @return std::vector<std::string> A vector of strings containing the combined results from the child processes.
     */
    std::vector<std::string> collectProcessedDataResults(std::vector<int> &childToParentPipes);
};

#endif // SERVER_H
