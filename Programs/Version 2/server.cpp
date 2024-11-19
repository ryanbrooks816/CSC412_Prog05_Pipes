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
 * @brief Destroys the Server object.
 *
 * This destructor cleans up any resources used by the Server object.
 */
Server::~Server()
{
    // Delete the tmp folder
    std::filesystem::remove_all("tmp");
}

/**
 * @brief Distributes a list of data files evenly among the clients.
 *
 * This function takes a vector of file paths and distributes them among the available
 * clients. Each client is assigned a range of files to process. The distribution
 * ensures that any remainder files are distributed one per client until exhausted.
 *
 * @param files A vector of strings representing the names of the data files to be distributed.
 */
void Server::distributeDataFiles(const std::vector<std::string> &files)
{
    int numFiles = files.size();
    DEBUG_FILE("Retrieved " + std::to_string(numFiles) + " data files.", "debug.log");

    // Evenly distribute the files into the clients by computing the start and end indices
    // in the list of files for each client
    int filesPerClient = numFiles / numClients;
    int remainder = numFiles % numClients;

    int startIndex = 0;
    for (int i = 0; i < numClients; i++)
    {
        int endIndex = startIndex + filesPerClient + (i < remainder ? 1 : 0);

        // Sim update the client with its slice of the data files
        this->clients[i].setFilesStartIdx(startIndex);
        this->clients[i].setFilesEndIdx(endIndex);

        DEBUG_FILE("Client " + std::to_string(i) + " will process files " + std::to_string(startIndex) + " to " + std::to_string(endIndex - 1), "debug.log");
        startIndex = endIndex;
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
 *
 * @param files A vector of strings representing the names of the data files
 */
void Server::verifyDataFilesDistribution(const std::vector<std::string> &files)
{
    // Make sure the tmp directory exists
    std::filesystem::create_directory("./tmp");

    // Fork a child process for each client that will call a function to verify the data files
    // and send any files that don't belong to the client to the correct client
    for (int i = 0; i < this->numClients; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            this->clients[i].verifyDataFilesDistribution(this->numClients, files);
            exit(0);
        }
    }

    // Parent process waits for all child processes to finish
    for (int i = 0; i < this->numClients; i++)
    {
        int status;
        wait(&status);
    }

    std::cout << "Verified data files distribution." << std::endl;

    // Read the temporary files created by the child processes and update the clients' file lists
    this->readDistributorTempFiles(files);

    DEBUG_FILE("Verified data files distribution.", "debug.log");
}

/**
 * @brief Reads temporary files created by child processes during the data distribution
 * processand updates the clients' file lists.
 *
 * This function iterates over the number of clients and attempts to open a corresponding
 * temporary file for each client. Expects the temporary files to be named and formatted
 * in a specific way. Then it updates the clients' file lists with the appropriate files.
 *
 * @param files A vector of strings representing the file paths to be assigned to clients.
 *
 * @throws std::runtime_error if a temporary file cannot be opened.
 */
void Server::readDistributorTempFiles(const std::vector<std::string> &files)
{
    // Read the temporary files created by the child processes and update the clients' file lists
    for (int i = 0; i < this->numClients; i++)
    {
        // Expect the temporary files to be named ch_<clientIdx>.txt
        // Lines should contain the client index and the file index
        std::ifstream tempFile("tmp/ch_" + std::to_string(i) + ".txt");
        if (tempFile.is_open())
        {
            std::string line;
            while (std::getline(tempFile, line))
            {
                // Extract the process index and file index from the line
                std::istringstream iss(line);
                int processIdx, fileIdx;
                if (iss >> processIdx >> fileIdx)
                {
                    // Add the file to the client's list
                    this->clients[processIdx].addFile(files[fileIdx]);
                }
            }
            tempFile.close();
        }
        else
        {
            std::cerr << "Error opening temporary ch file for client " << i << std::endl;
            exit(42);
        }
    }
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
    // Make sure the tmp directory exists
    std::filesystem::create_directory("tmp");

    std::string combinedResult;

    // For each client, process the data files, which produces the reconstructed block of code
    // and stack the blocks of code in order of the client index
    for (auto client : clients)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            client.processDataFiles();
            exit(0);
        }
    }

    // Parent process waits for all child processes to finish
    for (int i = 0; i < this->numClients; i++)
    {
        int status;
        wait(&status);
    }

    // Read the temporary files created by the child processes and combine the results
    // Return the entire combined program
    combinedResult = this->readDataProcessingTempFiles();

    DEBUG_FILE("Processed data files.", "debug.log");

    return combinedResult;
}

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
std::string Server::readDataProcessingTempFiles()
{
    std::string combinedResult;

    // Read the temporary files created by each child process and retrieve the combined code block
    for (int i = 0; i < this->numClients; i++)
    {
        // Expect the temporary files to be named sch_<clientIdx>.txt
        // Lines should contain lines of code sorted and processed by the child process
        std::ifstream tempFile("tmp/sch_" + std::to_string(i) + ".txt");
        if (tempFile.is_open())
        {
            std::string line;
            while (std::getline(tempFile, line))
            {
                combinedResult += line + "\n";
            }
            tempFile.close();
        }
        else
        {
            std::cerr << "Error opening temporary sch file for client " << i << std::endl;
            exit(43);
        }
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
