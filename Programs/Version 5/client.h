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
    void verifyDataFilesDistribution(int numClients, const std::vector<std::string> &files, int writePipeFd);

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
    void initializeProcessor(int writePipeFd);

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
    void processDataFiles(int writePipeFd);

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
    void runProcessorChildProcess(int writePipeFd);

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
};

#endif // CLIENT_H
