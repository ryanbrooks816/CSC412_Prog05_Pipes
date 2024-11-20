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
#ifndef DEBUG
    std::filesystem::remove_all("tmp");
#endif
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
void Server::initializeDistributor(const std::vector<std::string> &files)
{
    // Make sure the tmp directory exists
    std::filesystem::create_directory("./tmp");

    // Keep track of the number of clients that have been verified to know when
    // all clients have been processed
    int clientsVerified = 0;

    // Create pipes for each child
    std::vector<int> childToParentPipes(numClients);
    std::vector<int> parentToChildPipes(numClients);
    std::vector<pid_t> childPIDs(numClients);

    // Fork a child process for each client that will call a function to verify the data files
    // and send any files that don't belong to the client to the correct client
    for (int i = 0; i < this->numClients; i++)
    {
        // Create pipes for each child process
        int pipeChildToParent[2], pipeParentToChild[2];
        if (pipe(pipeChildToParent) == -1 || pipe(pipeParentToChild) == -1)
        {
            perror("pipe failed");
            exit(150);
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            close(pipeChildToParent[0]); // Close read end of child-to-parent pipe
            close(pipeParentToChild[1]); // Close write end of parent-to-child pipe

            this->runDistributorChildProcess(i, pipeChildToParent[1], pipeParentToChild[0], files);
            exit(0); // Exit child process
        }
        else if (pid > 0)
        {
            // Parent process
            childPIDs[i] = pid;
            childToParentPipes[i] = pipeChildToParent[0]; // Read end of child-to-parent pipe
            parentToChildPipes[i] = pipeParentToChild[1]; // Write end of parent-to-child pipe

            close(pipeChildToParent[1]); // Close write end in parent
            close(pipeParentToChild[0]); // Close read end in parent

            DEBUG_FILE("Launched child processes to verify data files distribution.", "debug.log");

            // Waits for child signals
            while (clientsVerified < numClients)
            {
                for (int i = 0; i < numClients; ++i)
                {
                    if (childToParentPipes[i] != -1)
                    {
                        int clientIdx;
                        ssize_t bytesRead = read(childToParentPipes[i], &clientIdx, sizeof(clientIdx));
                        if (bytesRead > 0)
                        {
                            ++clientsVerified;
                            close(childToParentPipes[i]); // Close pipe once data is read
                            childToParentPipes[i] = -1;   // Mark as closed
                        }
                    }
                }
            }

            DEBUG_FILE("Verified data files distribution for all clients.", "debug.log");

            // Notify all children to proceed
            for (int i = 0; i < numClients; ++i)
            {
                int signal = 1; // Arbitrary signal
                write(parentToChildPipes[i], &signal, sizeof(signal));
                close(parentToChildPipes[i]); // Close write end after signaling
            }

            // Wait for all child processes to finish
            for (int i = 0; i < numClients; i++)
            {
                int status;
                waitpid(childPIDs[i], &status, 0);
            }
        }
        else
        {
            perror("fork failed");
            exit(160);
        }
    }

    DEBUG_FILE("Finished distributing and processing data files.", "debug.log");
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
