#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>

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
     * Updates the list of files.
     *
     * @param files The new list of (verified) files.
     */
    void setFiles(const std::vector<std::string> &files);

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
    std::string processDataFiles();

private:
    /**
     * The index of the client.
     */
    int clientIdx;

    /**
     * A vector of strings representing the file paths associated with the client.
     */
    std::vector<std::string> files;
};

#endif // CLIENT_H
