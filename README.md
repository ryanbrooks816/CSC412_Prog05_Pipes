# CSC412_Prog05_Pipes

Details of implementation specifics are in the report. 

The program makes use of the testing files by compiling with the debug flag. Use the build.sh script like so for debugging:

`./Scripts/build.sh debug`.

Also note earlier versions of the program write files to a tmp folder create at the root of the project. It's deleted after the program has finished with it. Same for the debug folder if the program is compiled with the debug script.

The scripts folder contains scripts to run each version, all expected to be run at the root of the project.

An Executables folder is also created at the root of the project and is where the script compiles to. The versions using the `exec` call expect the required executables there and will exit with an error if they're not.

I'm submitting extra credit (there was only 1 offered in this assingment).

It has its own script in the Scripts folder called exactly the same way as the others. It uses it's own folder in Programs as well as in Executables. I just wrote it as Version 5EC so no naming requirements were given.