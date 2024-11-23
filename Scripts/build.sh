path3="./Programs/Version 3/"
path4="./Programs/Version 4/"
path5="./Programs/Version 5/"
path6="./Programs/Version 5EC/"
debug_flag=""

if [ "$1" == "debug" ]; then
    debug_flag="-DDEBUG"
fi

# Make the executable sub-folders if they don't exist
# mkdir -p ./Executables/Version\ 1
# mkdir -p ./Executables/Version\ 2
# mkdir -p ./Executables/Version\ 3
# mkdir -p ./Executables/Version\ 4
# mkdir -p ./Executables/Version\ 5
# mkdir -p ./Executables/Version\ 5EC

# g++ -Wall -std=c++20 $debug_flag ./Programs/Version\ 1/*.cpp -o ./Executables/Version\ 1/version1
# g++ -Wall -std=c++20 $debug_flag ./Programs/Version\ 2/*.cpp -o ./Executables/Version\ 2/version2

# g++ -Wall -std=c++20 $debug_flag "${path3}main.cpp" "${path3}server.cpp" "${path3}client.cpp" "${path3}testing.cpp" -o ./Executables/Version\ 3/version3
# g++ -Wall -std=c++20 $debug_flag "${path3}distributor.cpp" "${path3}client.cpp" "${path3}testing.cpp" -o ./Executables/Version\ 3/distributor
# g++ -Wall -std=c++20 $debug_flag "${path3}processor.cpp" "${path3}client.cpp" "${path3}testing.cpp" -o ./Executables/Version\ 3/processor

# g++ -Wall -std=c++20 $debug_flag "${path4}main.cpp" "${path4}server.cpp" "${path4}client.cpp" "${path4}testing.cpp" -o ./Executables/Version\ 4/version4
# g++ -Wall -std=c++20 $debug_flag "${path4}distributor.cpp" "${path4}client.cpp" "${path4}testing.cpp" -o ./Executables/Version\ 4/distributor
# g++ -Wall -std=c++20 $debug_flag "${path4}processor.cpp" "${path4}client.cpp" "${path4}testing.cpp" -o ./Executables/Version\ 4/processor

# g++ -Wall -std=c++20 $debug_flag "${path5}main.cpp" "${path5}server.cpp" "${path5}client.cpp" "${path5}testing.cpp" -o ./Executables/Version\ 5/version5
# g++ -Wall -std=c++20 $debug_flag "${path5}distributor.cpp" "${path5}client.cpp" "${path5}testing.cpp" -o ./Executables/Version\ 5/distributor
# g++ -Wall -std=c++20 $debug_flag "${path5}processor.cpp" "${path5}client.cpp" "${path5}testing.cpp" -o ./Executables/Version\ 5/processor

g++ -Wall -std=c++20 $debug_flag "${path6}main.cpp" "${path6}server.cpp" "${path6}client.cpp" "${path6}communications.cpp" -o ./Executables/Version\ 5EC/version5EC
g++ -Wall -std=c++20 $debug_flag "${path6}distributor.cpp" "${path6}client.cpp" "${path6}communications.cpp" -o ./Executables/Version\ 5EC/distributor
g++ -Wall -std=c++20 $debug_flag "${path6}processor.cpp" "${path6}client.cpp" "${path6}communications.cpp" -o ./Executables/Version\ 5EC/processor
