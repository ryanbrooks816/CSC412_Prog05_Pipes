/*
1. Your script will search through all the data files to determine 
the total number of processes required for the project (use regex). 
Each data  file has the process index in the first line. Highest 
process index  found in the data files will determine the number of 
processes to be created. The script will then launch the 
parent/server process, with the desired number of processes and 
data folder’s path as arguments.

2. Data Distribution:
- A parent (server) process is launched by the script
- Creates distributor processes
- The parent process will create appropriate communication channels
- Handles communication with and between the distributor processes

The child processes (distributors) will then proceed to distribute 
the data files between themselves, that is, to attribute each data 
file to the process that should work on it.
- Spread the data evenly among the processes
- Each distributor process will go through the list of data files
  and determine if it's in the right distributor or not. 
    - Read the first line of the file to determine the process
      index
    - Compare it to the distributor's index. If it's the same,
      add it to the distributor's "to-do list". If not, send some
      data back to the server process. Send the distributor process
      that should handle the data file and the file's index.
    - i.e. it sends "2 0" indicating that the distributor with
      index 2 should handle the file with index 0.
- All communication is done through the server. The server will
  receive the data file index and the distributor index and will
  send the data file to the distributor process. The distributor
  index is sent both ways in the communication, ensuring that the
  server knows which distributor to send the data file to and that
  the distrubutor can confirm that it didn't get another 
  distributor's data file.

3. Data Processing:
- Each distributor process will create a "processor" child process.
- It sends all the data files in its "to-do list" to the processor
  process.
- Executes the processor process:
    An example of what the data is we're sending:
    Imagine a C program that has been split into multiple data files.
    Each data file is randonly named. We need to put it back together.
    So in this example the C program was split into 2 processes.
    That means there's 2 data files, (which means 2 distributors,
    d0 and d1). Each line in the data file contains a line of code
    prefixed with the process index and the line number.
    The processor process will read the data file and sort the lines
    back into the correct order (using the std sort function).
    Each processor, remember, is only dealing with 1 block of code.
    The block of code is split into many data files. Concurently,
    the processors will sort the lines back into the correct order.
    Now, each processor will have the sorted block of code.
    We must also remove the process index and line number from the
    data file. We only want the code (before putting the fragments
    back together).
- The processor will then pass the reconstructed block of code back
  to the distributor process.

4. Integration:
- Each distributor process will receive the reconstructed block of 
  code from the processor process and then send it back to the server.
- The server must write the code in the correct order to an output
  file.

Data validation 
Depending how the user of your script may produce the data folder’s 
path, the string passed as argument to your program may or may not 
end with a / character. You have to check for that and, if the / 
is missing, append the character to the path before sending it to 
the distributor processes.
Similarly, your must verify whether the name of the output file has 
a .c extension, and add one if it is missing.

*/