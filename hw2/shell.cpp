// Kevin Bui
// January 27, 2024
// Last Revised: January 29, 2024
// IDE: Visual Studio Code
// shell.cpp: Basic command line shell that allows users to execute programs.

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include "Command.h"
using namespace std;

int main ()
{
    string commandLine;
    Command line;

    // loop runs until quit or Ctrl + C
    while(true){
        cout << "\nshell> ";

        // receive user input
        getline(cin, commandLine);

        line.parseCommandString(commandLine);

        string arg0 = line.getArg(0);
        int numArgs = line.getNumArgs();

        // checks if user inputs only enter
        if(arg0 == "") continue;

        // checks for quit as well as valid parameters (no parameters)
        if(arg0 == "quit"){
            if(numArgs == 1) break;
            else{
                cout << "\nInvalid command line";
                continue;
            }
            // checks for cd command as well as path, only takes 2 arguments
        }else if(arg0 == "cd"){
            if(numArgs == 1){
                chdir(getenv("HOME"));
            }else if(numArgs == 2){
                if(chdir(line.getArg(1).c_str()) != 0)
                perror("Error");
            }else{
                cout << "\nInvalid command line";
                continue;
            }
        }else{
            // assume fork execution next
            pid_t pid = fork();
            if(pid < 0){
                perror("Error");
                exit(-1);
            }else if(pid == 0){
                execvp(arg0.c_str(), line.getArgList());
                perror("Error");
                break;
            }else{
                wait(NULL);
            }
        }
    }
    return 0;
}