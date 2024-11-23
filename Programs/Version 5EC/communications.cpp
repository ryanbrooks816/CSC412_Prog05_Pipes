#include "communications.h"

/**
 * @brief Reads a message from the specified pipe file descriptor.
 *
 * This function reads a message from the specified pipe file descriptor. The message
 * is expected to be sent in chunks, where each chunk consists of a size followed by
 * the actual data. The function reads the number of chunks first, then reads each
 * chunk and concatenates them to form the complete message.
 *
 * @param readPipeFd The file descriptor for the read end of the pipe.
 * @param debugFile The name of the debug file for logging.
 * @return The complete message read from the pipe.
 */
void writeToPipe(int writePipeFd, const std::string &message, const std::string &debugFile)
{
    size_t messageSize = message.size();
    size_t chunks = (messageSize + CHUNCK_LIMIT - 1) / CHUNCK_LIMIT;

    // Write the number of chunks
    if (write(writePipeFd, &chunks, sizeof(chunks)) == -1)
    {
        DEBUG_FILE("Failed to write number of chunks to pipe", debugFile);
        exit(158);
    }

    // Write each chunk
    for (size_t i = 0; i < chunks; ++i)
    {
        size_t start = i * CHUNCK_LIMIT;
        size_t chunkSize = std::min(CHUNCK_LIMIT, messageSize - start);

        // Write the size of the current chunk
        if (write(writePipeFd, &chunkSize, sizeof(chunkSize)) == -1)
        {
            DEBUG_FILE("Failed to write chunk size to pipe", debugFile);
            exit(159);
        }

        // Write the actual chunk data
        if (write(writePipeFd, message.c_str() + start, chunkSize) == -1)
        {
            DEBUG_FILE("Failed to write chunk to pipe", debugFile);
            exit(160);
        }
    }

    DEBUG_FILE("Wrote message to pipe: " + message + " in " + std::to_string(chunks) + " chunks", debugFile);
}

/**
 * @brief Reads a message from the specified pipe file descriptor.
 *
 * This function reads a message from the specified pipe file descriptor. The message
 * is expected to be sent in chunks, where each chunk consists of a size followed by
 * the actual data. The function reads the number of chunks first, then reads each
 * chunk and concatenates them to form the complete message.
 *
 * @param readPipeFd The file descriptor for the read end of the pipe.
 * @param debugFile The name of the debug file for logging.
 * @return The complete message read from the pipe.
 */
std::string readFromPipe(int readPipeFd, const std::string &debugFile)
{
    // Read the number of chunks
    size_t chunks;
    ssize_t bytesRead = read(readPipeFd, &chunks, sizeof(chunks));
    if (bytesRead == 0)
    {
        // End of the pipe
        DEBUG_FILE("Pipe closed", debugFile);
        return "";
    }
    else if (bytesRead != sizeof(chunks))
    {
        // Handle partial read or error
        DEBUG_FILE("Error reading number of chunks from pipe", debugFile);
        exit(158);
    }

    std::string message;
    message.reserve(chunks * CHUNCK_LIMIT);

    for (size_t i = 0; i < chunks; ++i)
    {
        // Read the size of the current chunk
        size_t chunkSize;
        bytesRead = read(readPipeFd, &chunkSize, sizeof(chunkSize));
        if (bytesRead != sizeof(chunkSize))
        {
            DEBUG_FILE("Error reading chunk size from pipe", debugFile);
            exit(159);
        }

        // Read the actual chunk data
        std::vector<char> buffer(chunkSize);
        bytesRead = read(readPipeFd, buffer.data(), chunkSize);
        if (bytesRead != static_cast<ssize_t>(chunkSize))
        {
            DEBUG_FILE("Error reading chunk data from pipe", debugFile);
            exit(160);
        }

        message.append(buffer.begin(), buffer.end());
    }

    return message;
}

