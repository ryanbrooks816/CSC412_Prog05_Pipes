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
std::string Server::initializeDistributor(const std::vector<std::string> &files)
{
    // Create pipes for each child
    std::vector<int> childToParentPipes(numClients);
    std::vector<int> parentToChildPipes(numClients);
    std::vector<pid_t> childPIDs(numClients);

    // Fork a child process for each client that will call a function to verify the data files
    // and send any files that don't belong to the client to the correct client
    for (int i = 0; i < this->numClients; i++)
    {
        // Create pipes for child-to-parent and parent-to-child communication
        int pipeChildToParent[2]; // [0] = read, [1] = write
        int pipeParentToChild[2]; // [0] = read, [1] = write

        if (pipe(pipeChildToParent) == -1 || pipe(pipeParentToChild) == -1)
        {
            std::cerr << "Creating pipes failed" << std::endl;
            exit(150);
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            close(pipeChildToParent[0]); // Close read end of child-to-parent pipe
            close(pipeParentToChild[1]); // Close write end of parent-to-child pipe

            // Pass the client's index, the write end of the child to parent pipe,
            // the read end of the parent to child pipe, and the list of files to the child process
            this->runDistributorChildProcess(i, pipeChildToParent[1], pipeParentToChild[0], files);
            exit(0); // Exit child process
        }
        else if (pid > 0)
        {
            // Parent process
            childPIDs[i] = pid;
            childToParentPipes[i] = pipeChildToParent[0]; // Read end of child-to-parent pipe (for receiving signals)
            parentToChildPipes[i] = pipeParentToChild[1]; // Write end of parent-to-child pipe (for sending signals)

            close(pipeChildToParent[1]); // Close write end in parent
            close(pipeParentToChild[0]); // Close read end in parent
        }
        else
        {
            std::cerr << "Forking distributor child process failed" << std::endl;
            exit(160);
        }
    }

    DEBUG_FILE("Launched child processes to verify data files distribution.", "debug.log");

    // Wait for all child processes to send verified data files through pipes
    // Create a vector to store any incorrectly distributed files for redistribution
    std::vector<std::vector<std::string>> incorrectlyDistributedFiles = this->awaitDistributorProcesses(childToParentPipes);

    // Redistribute any incorrectly distributed files by sending them to the correct clients
    this->redistributeDataFiles(incorrectlyDistributedFiles, parentToChildPipes);

    // Distributor process do some work, create their own children, process data, etc
    // Wait for all child processes to finish processing the data files
    for (int i = 0; i < this->numClients; i++)
    {
        int status;
        waitpid(childPIDs[i], &status, 0);
    }

    DEBUG_FILE("Finished distributing and processing data files.", "debug.log");

    // Children have finsihed processing the data files since they've terminated.
    // Now We can retrieve the code blocks sent by the children.
    std::vector<std::string> combinedResults = this->collectProcessedDataResults(childToParentPipes);

    // Concatenate the combined results from all clients
    return std::accumulate(combinedResults.begin(), combinedResults.end(), std::string());
}

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
void Server::runDistributorChildProcess(int i, int writePipeFd, int readPipeFd, const std::vector<std::string> &files)
{
    int numFiles = this->clients[i].getFilesEndIdx() - this->clients[i].getFilesStartIdx();

    // Precompute the total number of arguments
    unsigned int baseArgs = 7;
    size_t totalArgs = baseArgs + numFiles + 1;

    // Create a vector of strings to store the arguments

    std::vector<std::string> args(totalArgs);
    args[0] = std::string(EXECUTABLES_PATH + "distributor");
    args[1] = std::to_string(writePipeFd);
    args[2] = std::to_string(readPipeFd);
    args[3] = std::to_string(this->numClients);
    args[4] = std::to_string(i);
    args[5] = std::to_string(this->clients[i].getFilesStartIdx());
    args[6] = std::to_string(this->clients[i].getFilesEndIdx());

    // Add the subset of files for the current client to the argument list
    int argsStartIdx = baseArgs;
    for (int j = this->clients[i].getFilesStartIdx(); j < this->clients[i].getFilesEndIdx(); ++j)
    {
        args[argsStartIdx++] = files[j];
    }

    // Convert the vector of strings to a vector of char* for execvp
    std::vector<char *> c_args(totalArgs);
    for (size_t i = 0; i < totalArgs; i++)
    {
        c_args[i] = const_cast<char *>(args[i].c_str());
    }
    c_args[totalArgs - 1] = nullptr; // Null-terminate the argument list

    DEBUG_FILE("Launched a distributor process for client " + std::to_string(i), "debug.log");

    // Call the child process's own program to verify the distribution of data files
    execvp(std::string(EXECUTABLES_PATH + "distributor").c_str(), c_args.data());

    // Exit if execvp fails
    perror("execvp failed");
    exit(120);
}

/**
 * @brief Waits for distributor processes to send messages through pipes and collects
 * incorrectly distributed files.
 *
 * This function listens to multiple child-to-parent pipes for messages from distributor
 * processes. Each message contains the incorrectly distributed file path and the client
 * index it belongs to. The function reads these messages, parses them, and
 * stores the file paths in a vector for each client. The function continues to read
 * from each pipe until a "DONE" signal is received or the pipe is closed, meaning that
 * distributor process has finished verifying the data files and is waiting for the server
 * to redistribute any incorrectly distributed files before continuing.
 *
 * @param childToParentPipes A vector of file descriptors for the child-to-parent pipes.
 * @return A vector of vectors containing the file paths of incorrectly distributed files
 * for each client.
 */
std::vector<std::vector<std::string>> Server::awaitDistributorProcesses(std::vector<int> &childToParentPipes)
{
    // Store any incorrectly distributed files for redistribution
    std::vector<std::vector<std::string>> incorrectlyDistributedFiles(this->numClients);

    // Iterate over all child-to-parent pipes
    for (int i = 0; i < this->numClients; ++i)
    {
        while (true)
        {
            // Read the size of the incoming message
            size_t messageSize;
            ssize_t bytesRead = read(childToParentPipes[i], &messageSize, sizeof(messageSize));

            if (bytesRead == 0)
            {
                // End of the pipe; no more data to read
                DEBUG_FILE("Pipe closed for client " + std::to_string(i), "debug.log");
                break;
            }
            else if (bytesRead != sizeof(messageSize))
            {
                // Handle partial read or error
                DEBUG_FILE("Error reading message size from client " + std::to_string(i), "debug.log");
                exit(160);
            }

            // Check if the message is a "DONE" signal
            if (messageSize == 0)
            {
                DEBUG_FILE("Received DONE signal from client " + std::to_string(i), "debug.log");
                break; // Exit the loop for this client
            }

            // Read the actual message content
            std::vector<char> buffer(messageSize);
            bytesRead = read(childToParentPipes[i], buffer.data(), messageSize);

            if (bytesRead != static_cast<ssize_t>(messageSize))
            {
                DEBUG_FILE("Error reading message content from client " + std::to_string(i), "debug.log");
                exit(161);
            }

            // Parse the message (expected format: "processIdx filePath")
            std::string message(buffer.begin(), buffer.end());
            DEBUG_FILE("Received message: " + message, "debug.log");

            // Add the file to the list of incorrectly distributed files based on client index
            size_t spacePos = message.find(' ');
            if (spacePos != std::string::npos)
            {
                int processIdx = std::stoi(message.substr(0, spacePos));
                std::string filePath = message.substr(spacePos + 1);

                incorrectlyDistributedFiles[processIdx].push_back(filePath);
            }
            else
            {
                DEBUG_FILE("Malformed message from client " + std::to_string(i) + ": " + message, "debug.log");
            }
        }
    }

    DEBUG_FILE("Verified data files distribution for all clients.", "debug.log");

    return incorrectlyDistributedFiles;
}

/**
 * @brief Redistributes incorrectly distributed data files to the appropriate clients.
 *
 * This function iterates over a list of incorrectly distributed files for each client
 * and sends the file paths to the respective clients through pipes. It also sends an
 * ending signal to indicate that the redistribution is complete so that the distributor
 * can immediately resume and process the data files.
 *
 * Each inner vector in incorrectlyDistributedFiles corresponds to a specific client.
 * The function writes the size of each file path followed by the actual file path
 * to the respective client's pipe. An ending signal (size = 0) is sent after all
 * files for a client have been redistributed.
 *
 * @param incorrectlyDistributedFiles A vector of vectors containing file paths that need
 * to be redistributed for each client.
 * @param parentToChildPipes A vector of pipe file descriptors used to communicate with
 * each client.
 */
void Server::redistributeDataFiles(const std::vector<std::vector<std::string>> &incorrectlyDistributedFiles, std::vector<int> &parentToChildPipes)
{
    // Iterate over all clients
    for (size_t i = 0; i < incorrectlyDistributedFiles.size(); ++i)
    {
        // Skip if there are no incorrectly distributed files for this client
        if (incorrectlyDistributedFiles[i].empty())
        {
            continue;
        }

        // Iterate over files that need to be redistributed to client `i`
        for (const auto &file : incorrectlyDistributedFiles[i])
        {
            std::string debugMessage = "Redistributing file " + file + " to client " + std::to_string(i);
            DEBUG_FILE(debugMessage, "debug.log");

            // Write the file path size to the pipe
            size_t messageSize = file.size();
            if (write(parentToChildPipes[i], &messageSize, sizeof(messageSize)) == -1)
            {
                DEBUG_FILE("Failed to write file size to pipe for client " + std::to_string(i), "debug.log");
                exit(162);
            }

            // Write the actual file path to the pipe
            if (write(parentToChildPipes[i], file.c_str(), messageSize) == -1)
            {
                DEBUG_FILE("Failed to write file path to pipe for client " + std::to_string(i), "debug.log");
                exit(163);
            }
        }

        // Send an ending signal (size = 0) to indicate redistribution is complete
        size_t endSignal = 0;
        if (write(parentToChildPipes[i], &endSignal, sizeof(endSignal)) == -1)
        {
            DEBUG_FILE("Failed to send ending signal to client " + std::to_string(i), "debug.log");
            exit(164);
        }
    }
}

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
std::vector<std::string> Server::collectProcessedDataResults(std::vector<int> &childToParentPipes)
{
    std::vector<std::string> combinedResults(this->numClients);

    for (int i = 0; i < this->numClients; i++)
    {
        if (childToParentPipes[i] != -1)
        {
            // Read the size of the result
            size_t resultSize;
            ssize_t bytesRead = read(childToParentPipes[i], &resultSize, sizeof(resultSize));
            if (bytesRead != sizeof(resultSize))
            {
                DEBUG_FILE("Failed to read result size from client " + std::to_string(i), "debug.log");
                exit(165);
            }

            // Read the actual result content, a block of code
            std::string result(resultSize, '\0');
            bytesRead = read(childToParentPipes[i], result.data(), resultSize);
            if (bytesRead != static_cast<ssize_t>(resultSize))
            {
                DEBUG_FILE("Incomplete read from client " + std::to_string(i), "debug.log");
                exit(166);
            }

            // Store the block of code in the vector of combined results
            combinedResults[i] = result;
            DEBUG_FILE("Received combined result from client " + std::to_string(i) + ": " + result, "debug.log");

            // Close the pipe after reading the result
            close(childToParentPipes[i]);
            childToParentPipes[i] = -1;
        }
    }

    return combinedResults;
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
