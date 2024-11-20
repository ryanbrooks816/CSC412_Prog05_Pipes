path3="./Programs/Version 3/"
path4="./Programs/Version 4/"
path5="./Programs/Version 5/"
debug_flag=""

if [ "$1" == "debug" ]; then
    debug_flag="-DDEBUG"
fi

# Make the executable sub-folders if they don't exist
mkdir -p ./Executables/Version\ 1
mkdir -p ./Executables/Version\ 2
mkdir -p ./Executables/Version\ 3
mkdir -p ./Executables/Version\ 4

g++ -Wall -std=c++20 $debug_flag ./Programs/Version\ 1/*.cpp -o ./Executables/Version\ 1/version1
g++ -Wall -std=c++20 $debug_flag ./Programs/Version\ 2/*.cpp -o ./Executables/Version\ 2/version2

g++ -Wall -std=c++20 $debug_flag "${path3}main.cpp" "${path3}server.cpp" "${path3}client.cpp" "${path3}testing.cpp" -o ./Executables/Version\ 3/version3
g++ -Wall -std=c++20 $debug_flag "${path3}distributeData.cpp" "${path3}client.cpp" "${path3}testing.cpp" -o ./Executables/Version\ 3/distributeData
g++ -Wall -std=c++20 $debug_flag "${path3}processData.cpp" "${path3}client.cpp" "${path3}testing.cpp" -o ./Executables/Version\ 3/processData

g++ -Wall -std=c++20 $debug_flag "${path4}main.cpp" "${path4}server.cpp" "${path4}client.cpp" "${path4}testing.cpp" -o ./Executables/Version\ 4/version4
g++ -Wall -std=c++20 $debug_flag "${path4}distributeData.cpp" "${path4}client.cpp" "${path4}testing.cpp" -o ./Executables/Version\ 4/distributeData
g++ -Wall -std=c++20 $debug_flag "${path4}processData.cpp" "${path4}client.cpp" "${path4}testing.cpp" -o ./Executables/Version\ 4/processData
