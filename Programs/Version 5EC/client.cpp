#include "client.h"
#include "testing.h"
#include "communications.h"

// Determines where the executables are located for calling the distributor and processor programs
std::string EXECUTABLES_PATH = "./Executables/Version 5EC/";

unsigned int PIPE_CAPACITY = 4096;

/**
 * Constructs a client that represents a "distributor" process, whose job is to
 * process a subset of the data files associated with the distributor process.
 *
 * @param clientIdx The index of the client.
 */
Client::Client(int clientIdx)
{
    this->clientIdx = clientIdx;
    DEBUG_FILE("Client id " + std::to_string(clientIdx) + " created.", "debug.log");
}

/**
 * Constructs a client that represents a "distributor" process, whose job is to
 * process a subset of the data files associated with the distributor process.
 *
 * @param clientIdx The index of the client.
 * @param filesStartIdx The starting index of the subset of data files to be processed by the client.
 * @param filesEndIdx The ending index of the subset of data files to be processed by the client.
 */
Client::Client(int clientIdx, int filesStartIdx, int filesEndIdx)
{
    this->clientIdx = clientIdx;
    this->filesStartIdx = filesStartIdx;
    this->filesEndIdx = filesEndIdx;
    DEBUG_FILE("Client id " + std::to_string(clientIdx) + " created.", "debug_ch_" + std::to_string(clientIdx) + ".log");
}

/**
 * Retrieves the client index.
 *
 * @return The index of the client.
 */
int Client::getClientIdx() const
{
    return clientIdx;
}

/**
 * Updates the client index.
 *
 * @param clientIdx The new index of the client.
 */
void Client::setClientIdx(int clientIdx)
{
    this->clientIdx = clientIdx;
}

/**
 * Gets the starting index of the subset of data files to be processed by the client.
 *
 * @return The starting index.
 */
int Client::getFilesStartIdx() const
{
    return filesStartIdx;
}

/**
 * @brief Sets the starting index of the subset of data files to be processed by the client.
 *
 * @param startIdx The new starting index.
 */
void Client::setFilesStartIdx(int startIdx)
{
    this->filesStartIdx = startIdx;
}

/**
 * @brief Gets the ending index of the subset of data files to be processed by the client.
 *
 * @return The ending index.
 */
int Client::getFilesEndIdx() const
{
    return filesEndIdx;
}

/**
 * @brief Sets the ending index of the subset of data files to be processed by the client.
 *
 * @param endIdx The new ending index.
 */
void Client::setFilesEndIdx(int endIdx)
{
    this->filesEndIdx = endIdx;
}

/**
 * Adds a file to the list of files.
 *
 * @param file The file to add to the list.
 */
void Client::addFile(const std::string &file)
{
    this->verifiedFiles.push_back(file);
}

/**
 * Retrieves the list of files.
 *
 * @return A vector containing the names of the files.
 */
std::vector<std::string> Client::getFiles() const
{
    return verifiedFiles;
}

/**
 * Retrieves the file at the specified index.
 *
 * @param index The index of the file to retrieve.
 * @return The file at the specified index.
 *
 * @throws std::out_of_range if the index is out of bounds.
 */
std::string Client::getFile(size_t index) const
{
    if (index < verifiedFiles.size())
    {
        return verifiedFiles[index];
    }
    else
    {
        throw std::out_of_range("Index out of range");
    }
}

/**
 * Updates the list of files.
 *
 * @param files The new list of verified files.
 */
void Client::setFiles(std::vector<std::string> files)
{
    this->verifiedFiles = files;
}

/**
 * @brief Verifies the distribution of data files among clients and sneds the
 * incorrectly distributed files to the server for redistribution.
 *
 * This function reads the process index for each file to determine which client
 * it belongs to. If the file belongs to the current client, it is added to the
 * list of verified files. If not, the correct client index and file path are
 * written to the pipe so the server can send the file to the correct client.
 *
 * @param numClients The number of clients.
 * @param files A vector of strings containing the subset of files to be verified
 * and distributed by the current client.
 * @param writePipeFd The file descriptor for the write end of the pipe.
 */
void Client::verifyDataFilesDistribution(int numClients, const std::vector<std::string> &files, int writePipeFd)
{
    std::string debugChFile = "debug_ch_" + std::to_string(this->clientIdx) + ".log";
    DEBUG_FILE("Verifying data files for client " + std::to_string(this->clientIdx), debugChFile);

    // Go through the specified subset of files and add them to the appropriate client's list
    for (const auto &file : files)
    {
        // Read the process index for the file to determine which client it belongs to
        int processIdx = this->getDataFileProcessIdx(file);

        std::string message2 = "Processing file: " + file + " for client process " + std::to_string(processIdx);
        DEBUG_FILE(message2, debugChFile);

        // Figure out if the file belongs to the current client.
        // If so, add it to the list of verified files.
        if (processIdx == this->clientIdx)
        {
            this->addFile(file);
        }

        // If not, write the correct client index and the file path to the pipe so it can be
        // sent to the server (who will then send it to the correct client)
        else
        {
            // Write the correct client index and the file path to the pipe
            std::string serverMessage = std::to_string(processIdx) + " " + file;
            writeToPipe(writePipeFd, serverMessage, debugChFile);
        }
    }
}

/**
 * @brief Initializes the processor process to sort and combine the data files
 * contents into a single block of code.
 *
 * This function forks a child process and launches the processor program.
 * The arguments passed to the "processor" executable include:
 * - The path to the "processor" executable.
 * - The write end of the pipe to send the results to the distributor process.
 * - The client index.
 * - The number of verified files.
 * - The list of verified files.
 *
 * Invariant: Distributor process has updated the client's list of verified files.
 *
 * @param writePipeFd The file descriptor for the write end of the pipe.
 * @return A string containing the combined results from processing each client's
 * data files.
 *
 * @throws std::runtime_error if the fork fails or execvp fails.
 */
void Client::initializeProcessor(int writePipeFd)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        this->runProcessorChildProcess(writePipeFd);
        exit(0); // Exit child process
    }
    else if (pid > 0)
    {
        // Parent process should wait for the child process to finish
        int status;
        wait(&status);
        DEBUG_FILE("Processed data files for client " + std::to_string(this->clientIdx), "debug.log");
    }
    else
    {
        std::cerr << "Forking processor child process failed" << std::endl;
        exit(171);
    }

    DEBUG_FILE("Finished processing data files for client " + std::to_string(this->clientIdx), "debug.log");
}

/**
 * @brief Runs the processor child process to sort and combine the data files.
 *
 * This function is called by the child process to run the processor program.
 * The processor is sent the files belonging to the current client that were distributed
 * via the distributor parent process and process them by sorting the lines based on their
 * line numbers, and combines them into a single block of code. The results are written via
 * a pipe to be read by the distributor (parent) process and eventually processed by the server.
 *
 * @param writePipeFd The file descriptor for the write end of the pipe.
 */
void Client::runProcessorChildProcess(int writePipeFd)
{
    // Child (Grandchild) process
    size_t numFiles = this->verifiedFiles.size();

    // Precompute the total number of arguments
    unsigned int baseArgs = 4;
    size_t totalArgs = baseArgs + numFiles + 1;

    // Create a vector of strings to store the arguments
    std::vector<std::string> args(totalArgs);
    args[0] = std::string(EXECUTABLES_PATH + "processor");
    args[1] = std::to_string(writePipeFd);
    args[2] = std::to_string(this->clientIdx);
    args[3] = std::to_string(numFiles);

    // Add the subset of files for the current client to the argument list
    for (size_t j = 0; j < numFiles; j++)
    {
        args[j + baseArgs] = this->verifiedFiles[j];
    }

    // Convert the vector of strings to a vector of char* for execvp
    std::vector<char *> c_args(totalArgs);
    for (size_t i = 0; i < totalArgs; i++)
    {
        c_args[i] = const_cast<char *>(args[i].c_str());
    }
    c_args[totalArgs - 1] = nullptr; // Null-terminate the argument list

    DEBUG_FILE("Launching processor for client " + std::to_string(this->clientIdx), "debug.log");

    // Call the child process's own program to process the data files
    execvp(std::string(EXECUTABLES_PATH + "processor").c_str(), c_args.data());

    // Exit if execvp fails
    perror("execvp failed");
    exit(121);
}

/**
 * @brief Processes data files associated with the client and writes the results to a
 * pipe to be read by the parent distributor process.
 *
 * This function iterates over the list of files associated with the client, reads
 * their contents, and stores them in a vector. Each file's contents are represented
 * as a LineData object, which includes the line number and the code.
 * The lines are then sorted based on their line numbers to ensure the correct order.
 * Finally, the sorted lines are written in order to a pipe to be read by the distributor
 * process.
 *
 * Invariant: The input data files properly have the lines associated with the client
 * that puts them in the correct order.
 *
 * @param writePipeFd The file descriptor for the write end of the pipe.
 * @return A string containing the concatenated contents of all the data files,
 * ordered by line number.
 */
void Client::processDataFiles(int writePipeFd)
{
    std::string debugChFile = "debug_sch_" + std::to_string(this->clientIdx) + ".log";

    // Create a vector to store all the lines
    std::vector<LineData> lines = std::vector<LineData>(this->verifiedFiles.size());

    for (size_t i = 0; i < this->verifiedFiles.size(); i++)
    {
        std::string file = this->verifiedFiles[i];

        std::string message = "Processing data file " + file + " for client " + std::to_string(this->clientIdx);
        DEBUG_FILE(message, debugChFile);

        // Get the line data and put them in order based on the line number
        LineData lineData = this->getDataFileContents(file);
        lines[i] = lineData;
    }

    // Sort the lines based on the line number
    std::sort(lines.begin(), lines.end(), [](const LineData &a, const LineData &b)
              { return a.lineNum < b.lineNum; });

    // Write the sorted lines to the pipe, moving back up the communication chain
    // so the distributor can receive the sorted lines and combine them into a single block of code.
    std::string message;
    for (const auto &line : lines)
    {
        message += line.code + "\n";
    }

    // Write the combined message to the pipe
    writeToPipe(writePipeFd, message, debugChFile);
}

/**
 * @brief Retrieves the process index from the first line of a given file.
 *
 * Opens the specified file and reads the first line to extract
 * an integer value representing the process index. If the file cannot be opened
 * or the first line does not contain a valid integer, the function returns -1.
 *
 * @param filename The path to the file containing the process index.
 * @return The process index read from the file, or -1 if an error occurs.
 */
int Client::getDataFileProcessIdx(const std::string &filename)
{
    std::ifstream file(filename);
    int processIdx = -1;

    if (!file.is_open())
    {
        std::cerr << "Error opening data file: " << filename << std::endl;
        return processIdx;
    }

    std::string line;
    if (std::getline(file, line))
    {
        std::istringstream iss(line);
        iss >> processIdx;
    }

    return processIdx;
}

/**
 * @brief Reads the contents of a file and extracts specific data.
 *
 * This function opens a file specified by the given filename, reads the first line,
 * and extracts the process index, line number, and code from the line. The extracted
 * values are stored in a LineData structure and returned.
 *
 * @param filename The name of the file to read.
 * @return A LineData structure containing the extracted values. If the file cannot be
 *         opened, an empty LineData structure is returned.
 */
Client::LineData Client::getDataFileContents(const std::string &filename)
{
    std::ifstream file(filename);
    LineData allValues;

    if (!file.is_open())
    {
        std::cerr << "Error opening data file: " << filename << std::endl;
        return allValues;
    }

    std::string line;
    if (std::getline(file, line))
    {
        // Extract process index, line number, and code from the line
        std::istringstream iss(line);
        int processIdx, lineNum;
        std::string code;

        if (iss >> processIdx >> lineNum)
        {
            // Skip the first space after lineNum
            if (iss.peek() == ' ')
            {
                iss.get();
            }
            // Get the rest of the line as the code, including leading whitespace
            std::getline(iss, code);

            allValues.processIdx = processIdx;
            allValues.lineNum = lineNum;
            allValues.code = code;
        }
    }

    return allValues;
}
