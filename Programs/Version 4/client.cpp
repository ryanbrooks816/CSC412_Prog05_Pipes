#include "client.h"
#include "testing.h"

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
 * @brief Verifies the distribution of data files among clients and writes the
 * verified files to the client's temporary file.
 *
 * This function goes through the files and verifies that each file
 * belongs to the correct client by reading the process index from the file and
 * writing the correct client index and file path to a temporary file. The server
 * will later read these temporary files to update all clients with the correct files.
 * 
 * This function will be run by the distributor child process as a part of its own process
 * and runs exclusively on its own subset of files opposed to the previous versions.
 *
 * @param numClients The number of clients.
 * @param files A vector of strings containing the subset of files to be verified 
 * and distributed by the current client.
 */
void Client::verifyDataFilesDistribution(int numClients, const std::vector<std::string> &files)
{
    std::string debugChFile = "debug_ch_" + std::to_string(this->clientIdx) + ".log";
    DEBUG_FILE("Verifying data files for client " + std::to_string(this->clientIdx), debugChFile);

    // Write to a temporary file for the client a list of files' correct client index
    // they belong to and the file path
    std::ofstream tempFile("tmp/ch_" + std::to_string(this->clientIdx) + ".txt");

    // Go through the specified subset of files and add them to the appropriate client's list
    for (size_t i = 0; i < files.size(); i++)
    {
        const std::string &file = files[i];

        std::string message = "Verifying: " + file;
        DEBUG_FILE(message, debugChFile);

        // Read the process index for the file to determine which client it belongs to
        int processIdx = this->getDataFileProcessIdx(file);

        std::string message2 = "Processing file: " + file + " for client process " + std::to_string(processIdx);
        DEBUG_FILE(message2, debugChFile);

        // Write the correct client index and the file path to the temporary file
        tempFile << processIdx << " " << file << std::endl;
    }

    tempFile.close();
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

/**
 * @brief Processes data files associated with the client and writes the results to a
 * temporary file.
 *
 * This function iterates over the list of files associated with the client, reads
 * their contents, and stores them in a vector. Each file's contents are represented
 * as a LineData object, which includes the line number and the code.
 * The lines are then sorted based on their line numbers to ensure the correct order.
 * Finally, the sorted lines are written in order to a temporary file to be read by the
 * server process.
 *
 * Invariant: The input data files properly have the lines associated with the client
 * that puts them in the correct order.
 *
 * @return A string containing the concatenated contents of all the data files,
 * ordered by line number.
 */
void Client::processDataFiles()
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

    // Write the code block to a temporary file
    std::ofstream tempFile("tmp/sch_" + std::to_string(this->clientIdx) + ".txt");

    for (const LineData &line : lines)
    {
        tempFile << line.code << std::endl;
    }

    tempFile.close();
}
