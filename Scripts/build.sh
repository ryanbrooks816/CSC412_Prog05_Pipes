path="./Programs/Version 3/"
debug_flag=""

if [ "$1" == "debug" ]; then
    debug_flag="-DDEBUG"
fi

# g++ -Wall -std=c++20 $debug_flag "${path}main.cpp" "${path}server.cpp" "${path}client.cpp" "${path}testing.cpp" -o ./Executables/version1
# g++ -Wall -std=c++20 $debug_flag "${path}main.cpp" "${path}server.cpp" "${path}client.cpp" "${path}testing.cpp" -o ./Executables/version2

g++ -Wall -std=c++20 $debug_flag "${path}main.cpp" "${path}server.cpp" "${path}client.cpp" "${path}testing.cpp" -o ./Executables/version3
g++ -Wall -std=c++20 $debug_flag "${path}distributeData.cpp" "${path}client.cpp" "${path}testing.cpp" -o ./Executables/distributeData
g++ -Wall -std=c++20 $debug_flag "${path}processData.cpp" "${path}client.cpp" "${path}testing.cpp" -o ./Executables/processData