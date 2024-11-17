#ifndef TESTING_H
#define TESTING_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <type_traits>

/**
 * @brief Converts a vector of elements to a string representation.
 *
 * This function takes a vector of elements of any type and converts it to a
 * string representation in the format "[elem1, elem2, ..., elemN]".
 *
 * @tparam T The type of elements in the vector.
 * @param vec The vector to be converted to a string.
 * @return A string representation of the vector.
 */
template <typename T>
std::string toString(const std::vector<T> &vec)
{
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        oss << vec[i];
        if (i < vec.size() - 1)
        {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

#ifdef DEBUG

/**
 * Define the DEBUG macro to enable debug output. Utilize the DEBUG_CONSOLE and DEBUG_FILE macros
 * to output debug messages to the console and a file, respectively.
 *
 * DEBUG_CONSOLE(Out): Prints the debug message `Out` to the console if debugging is enabled.
 * DEBUG_FILE(Out, filePath): Writes the debug message `Out` to the specified file if debugging is enabled.
 */

// DEBUG_FILE: Writes the debug message to the specified file, followed by a newline
#define DEBUG_FILE(message, filePath)                \
    {                                                \
        std::ofstream file(filePath, std::ios::app); \
        file << message << std::endl;                \
        file.close();                                \
    }

// DEBUG_CONSOLE: Prints the debug message to the console, followed by a newline
#define DEBUG_CONSOLE(message)             \
    {                                      \
        std::cout << message << std::endl; \
    }

#else

#define DEBUG_CONSOLE(...)
#define DEBUG_FILE(...)

#endif // DEBUG

#endif // TESTING_H
