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
    this->files = std::vector<std::string>();
    DEBUG_FILE("Client id " + std::to_string(clientIdx) + " created.", "debug.log");
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
 * Adds a file to the list of files.
 *
 * @param file The file to add to the list.
 */
void Client::addFile(const std::string &file)
{
    this->files.push_back(file);
}

/**
 * Retrieves the list of files.
 *
 * @return A vector containing the names of the files.
 */
std::vector<std::string> Client::getFiles() const
{
    return files;
}

/**
 * Updates the list of files.
 *
 * @param files The new list of (verified) files.
 */
void Client::setFiles(const std::vector<std::string> &files)
{
    this->files = files;
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
        std::cerr << "Error opening file: " << filename << std::endl;
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
        std::cerr << "Error opening file: " << filename << std::endl;
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
 * @brief Processes data files associated with the client and concatenates their
 * contents into a single code block.
 *
 * This function iterates over the list of files associated with the client, reads
 * their contents, and stores them in a vector. Each file's contents are represented
 * as a LineData object, which includes the line number and the code.
 * The lines are then sorted based on their line numbers to ensure the correct order.
 * Finally, the sorted lines are concatenated into a single string representing the
 * complete code block.
 * 
 * Invariant: The input data files properly have the lines associated with the client
 * that puts them in the correct order.
 *
 * @return A string containing the concatenated contents of all the data files,
 * ordered by line number.
 */
std::string Client::processDataFiles()
{
    // Create a vector to store all the lines
    std::vector<LineData> lines = std::vector<LineData>(this->files.size());

    for (size_t i = 0; i < this->files.size(); i++)
    {
        std::string file = this->files[i];

        std::string message = "Processing data file " + file + " for client " + std::to_string(this->clientIdx);
        DEBUG_FILE(message, "debug.log");

        // Get the line data and put them in order based on the line number
        LineData lineData = this->getDataFileContents(file);
        lines[i] = lineData;
    }

    // Sort the lines based on the line number
    std::sort(lines.begin(), lines.end(), [](const LineData &a, const LineData &b)
              { return a.lineNum < b.lineNum; });

    // Concatenate all the lines to form the final code block
    std::string codeBlock;
    for (const LineData &line : lines)
    {
        codeBlock += line.code + "\n";
    }

    return codeBlock;
}
