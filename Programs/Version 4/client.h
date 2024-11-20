#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>

extern std::string EXECUTABLES_PATH;

class Client
{
public:
    /**
     * Constructs a client that represents a "distributor" process, whose job is to
     * process a subset of the data files associated with the distributor process.
     *
     * @param clientIdx The index of the client.
     */
    Client(int clientIdx);

    /**
     * Constructs a client that represents a "distributor" process, whose job is to
     * process a subset of the data files associated with the distributor process.
     *
     * @param clientIdx The index of the client.
     * @param filesStartIdx The starting index of the subset of data files to be processed by the client.
     * @param filesEndIdx The ending index of the subset of data files to be processed by the client.
     */
    Client(int clientIdx, int filesStartIdx, int filesEndIdx);

    /**
     * Retrieves the client index.
     *
     * @return The index of the client.
     */
    int getClientIdx() const;

    /**
     * Updates the client index.
     *
     * @param clientIdx The new index of the client.
     */
    void setClientIdx(int clientIdx);

    /**
     * Gets the starting index of the subset of data files to be processed by the client.
     *
     * @return The starting index.
     */
    int getFilesStartIdx() const;

    /**
     * Sets the starting index of the subset of data files to be processed by the client.
     *
     * @param startIdx The new starting index.
     */
    void setFilesStartIdx(int startIdx);

    /**
     * Gets the ending index of the subset of data files to be processed by the client.
     *
     * @return The ending index.
     */
    int getFilesEndIdx() const;

    /**
     * Sets the ending index of the subset of data files to be processed by the client.
     *
     * @param endIdx The new ending index.
     */
    void setFilesEndIdx(int endIdx);

    /**
     * Adds a file to the list of files.
     *
     * @param file The file to add to the list.
     */
    void addFile(const std::string &file);

    /**
     * Retrieves the list of files.
     *
     * @return A vector containing the names of the files.
     */
    std::vector<std::string> getFiles() const;

    /**
     * Retrieves the file at the specified index.
     *
     * @param index The index of the file to retrieve.
     * @return The file at the specified index.
     *
     * @throws std::out_of_range if the index is out of bounds.
     */
    std::string getFile(size_t index) const;

    /**
     * Updates the list of files.
     *
     * @param files The new list of verified files.
     */
    void setFiles(std::vector<std::string> files);

    /**
     * @brief Verifies the distribution of data files among clients and writes the
     * verified files to the client's temporary file.
     *
     * This function goes through the files and verifies that each file
     * belongs to the correct client by reading the process index from the file and
     * writing the correct client index and file index to a temporary file. The server
     * will later read these temporary files to update all clients with the correct files.
     *
     * This function will be run by the distributor child process as a part of its own process
     * and runs exclusively on its own subset of files opposed to the previous versions.
     *
     * @param numClients The number of clients.
     * @param files A vector of strings containing the subset of files to be verified
     * and distributed by the current client.
     */
    void verifyDataFilesDistribution(int numClients, const std::vector<std::string> &files);

    /**
     * @brief Reads temporary files created by all child processes during the data distribution
     * process and updates the current cilent's verified files list.
     *
     * This function iterates over the number of clients and attempts to open a corresponding
     * temporary file for each client. Expects the temporary files to be named and formatted
     * in a specific way. Then it updates the client's verified file lists with the files
     * that match the client's index.
     *
     * @param numClients The number of clients.
     *
     * @throws std::runtime_error if a temporary file cannot be opened.
     */
    void readDistributorTempFiles(int numClients);

    /**
     * @brief Initializes the processor process to sort and combine the data files
     * contents into a single block of code.
     *
     * This function forks a child process and launches the processor program.
     * The arguments passed to the "processor" executable include:
     * - The path to the "processor" executable.
     * - The client index.
     * - The number of verified files.
     * - The list of verified files.
     *
     * Invariant: Tmporary files have been created and processed into the client's verified
     * files list by the runDistributorChildProcess function.
     *
     * Invaiant: The tmp folder exists, which we know it does because the distributor
     * process creates it before running the child processes.
     *
     * @return A string containing the combined results from processing each client's
     * data files.
     *
     * @throws std::runtime_error if the fork fails or execvp fails.
     */
    void initializeProcessor();

    /**
     * @brief Runs the processor child process to sort and combine the data files.
     *
     * This function is called by the child process to run the processor program.
     * The processor program reads the temporary files created by the distributor
     * process, sorts the lines based on their line numbers, and combines them into
     * a single block of code. The results are written to a temporary file to be read
     * by distributor (parent) process and eventually processed by the server.
     *
     */
    void runProcessorChildProcess();

    /**
     * @brief Reads temporary file created by child processe during the data processing
     * process and combines the results.
     *
     * This function attempts to open a corresponding temporary file containing the processed
     * lines of code from the client's data files. Expects the temporary files to be named
     * and formatted in a specific way. Then it reads the contents of each file and combines
     * them into a single string representing the complete code block.
     *
     * @return A string containing the combined results from processing the client's
     * data files.
     *
     * @throws std::runtime_error if a temporary file cannot be opened.
     */
    std::string Client::readDataProcessingTempFile();

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
    int getDataFileProcessIdx(const std::string &filename);

    /**
     * @struct LineData
     * @brief Represents a line of code with associated metadata.
     *
     * Holds information about a specific line of code from a data file,
     * including the index of the process, the line number, and the actual
     * code content.
     */
    struct LineData
    {
        int processIdx;
        int lineNum;
        std::string code;
    };

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
    LineData getDataFileContents(const std::string &filename);

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
    void processDataFiles();

private:
    /**
     * The index of the client.
     */
    int clientIdx;

    /**
     * The starting index of the subset of data files to be processed by the client.
     */
    int filesStartIdx;

    /**
     * The ending index of the subset of data files to be processed by the client.
     */
    int filesEndIdx;

    /**
     * A vector of strings containing the file paths verified to belong to the client.
     */
    std::vector<std::string> verifiedFiles;
};

#endif // CLIENT_H
