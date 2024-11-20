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

