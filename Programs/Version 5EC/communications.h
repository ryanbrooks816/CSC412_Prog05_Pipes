#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include "testing.h"

const size_t CHUNCK_LIMIT = 64;

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
void writeToPipe(int writePipeFd, const std::string &message, const std::string &debugFile);

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
std::string readFromPipe(int readPipeFd, const std::string &debugFile);
