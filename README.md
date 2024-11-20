# CSC412_Prog05_Pipes
 
- Report stuff

Version 2 exposed me to the first forking part of the assignment.
Updating the loops in the verifyDataFilesDistribution and processDataFiles functions
from the server was easy based on how I designed version 1, but I did have to do a lot of
reorganizign in the client to support writing the data to files.
Before I handeled it with a matrix for all verified files and since I was linear I could just update whatever row in the verified files matrix with the new file and at the
end update the files for all the clients with the ones in the verified matrix.
I scrapped all of that and made a child function int he client to handle things
that's called after it forks. It is cleaned now and just writes the client index it properly belongs to and the index in the main files array (that's now exclusively passed by reference this version to have better memory management). Updating the sibling child processes was a bit easier. All I had to do was make it write to a file instead of returning a string and duplicated the code from the first generation function to read the child temp files. So that both the verify distrubtion and data file processing functions of the server call child functions in the client class, they write their output to temp files, then both the functions in server call a function (once the children are done) to read the temp files and process the data.

For version 3, implementing exec functions somewhat conflicted wit hthe object oriented server and client structure I had going on.
Previously the server would create the clients, various functions would be called in main, and eventually the server's method for distributing the data
files would be called and a child function would be forked for each client off of that. Then it would run the client's method to handle that. Same for data
proessing. I had to modify that so the server's function forked and then called exec instead of calling a method on each client to be run by the child process
it would call exec on a program to handle distributring the data--still handeled by each child. Then that program we need arguments, creates a child function
and calls the child's method to handle the data distrubution like before. Exact same idea for the data processing setup. 

The main modifications happened in those two functions. Consequently, I had to change the way files was passed around in other parts of the program to support data being written to files from different sources--now the exec programs. One example of this is in the readDistributorTempFiles function. The other change I had to make was modifying the fucntion to verify data distrubtions (the one called by exec program) because now it's working on a subset of files instead of a specified start and end index in all of the files.

Initially I didn't like this because it added uneeded complexity and runtime bloat to the program with constructing the arguments, especially for the subset of files
needed to be run by each client. However, Professor Herve explained to me that exec is a good idea when there's already an existing architecture and it would be
easy to just call exec and pass some data instead of integrating it in the system. This problem is very simple and already had planned for the grand scope
of what's happening so it feels a bit akward to implement. 

It was sligthly tricky getting the arguments to be passed to the exec functions right (and then make sure those outputted the data correctly like in version 2) to finally have the server read back the data. 

In report talk about v3 changes and the limit that could be reached for execvp

For version 4, I started off by breaking the server's verifyDataFilesDistribution functon into 2. 1 to handle the forking and another to prepare all the arguments for the exec call. This is because I had to set up pipes in this function between the server and all its clients, both reading and writing ones pipes. That's main purpose of this function now. Besides adding pipes to that function, I modified the distributor program, called by exec, to know when all its siblings have completed since now the distributor program would initialize the processor program, not the server. I was able to use the pipes to allow the distributor processes launched in the exec program to know when its siblings had finished to eventually call its grandchild process for data processing.

Talk about whether named or unnamed pipes were used.

I needed pipes since the server was no longer handling the next step in the program, the distributor program, just one of n clients, needed to know when its n siblings have all finished so it can move onto data processing. I kept track of the number of siblings complete by having the distributor program write to the server via pipes when it was done writing to the temporary files. The server would wait until all clients had signaled through reading pipes, and then write to the clients, signaling that they can continue, and closing the writing pipes. 

The next step involved modfying the client class and dsitributor program further to call the processDataFiles function not in the server but in the client. This also included other functions in the server, so they all had to be moved to the client class now. The new process invovles the distributor program reading all the data files produced by it (now that all distributor clients are done), and it can then update the clients with the files it found as belonging to that client. Only minor changes required to this function after moving it to the client's class. After that, I knew I needed to replicate a similar structure to verifying the data file's distribution like before.

Like normal it would call client functions to handle this, so the old processDataFiles in the client that was called by the main server one is now the main one called by the distributor program as a method of the client class. It's renamed to initializeProcessor to now provide the main forking and launching of the processor process. Similarly, the old verifyDataFilesDistribution function the server was renamed to initializeDistributor for the same reason. The processor function would be called from the distributor function and launches the grandchild (processor) process. That program didn't require any changes. Overall, I needed to do a lot of restructuring to get the pipes to work so the distributor processes could communicate and also to move a few functiosn that were handeled by the server to the client, but the program now has two main interactions from main, to launching the server, whcih branches off of initializeDistributor, which then calls initializeProcessor. Nothing needed to change in the client's processData function because it just writes to files. That part didn't need pipes in this version. For version 5, I would need to remove all instances of writing to files, which I think will greatly streamline the code and reduce arguments being passed around.