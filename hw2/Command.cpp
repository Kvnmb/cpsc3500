// Command.cpp: Stores a parsed command line.

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include "Command.h"
using namespace std;

Command::Command()
{
  numArgs = 0;
  argList = NULL;
}

Command::~Command()
{
  if (numArgs > 0) {
    for (int i = 0; i < numArgs; i++) {
      delete [] argList[i];
    }
    delete [] argList;
  }
}


void Command::parseCommandString(string commandStr)
{
  // Reset
  if (numArgs > 0) {
    for (int i = 0; i < numArgs; i++) {
      delete [] argList[i];
    }
    delete [] argList;
  }
  numArgs = 0;
  argList = NULL;
  
  // Count the number of tokens
  istringstream tokenStream(commandStr);
  string tempStr;
  int numTokens = 0;
  while (tokenStream >> tempStr) {
    numTokens++;
  }

  // Check for empty command line
  if (numTokens == 0) return;

  // Create an array of pointers for the tokens
  argList = new char *[numTokens + 1];
  if (argList == NULL) {
    cout << "Internal error: new failed" << endl;
    exit(-1);
  } 

  // Insert pointers to tokens into the argument array
  istringstream commandStream(commandStr);
  while (commandStream >> tempStr) {
    if (tempStr == "&") continue;
    argList[numArgs] = new char [tempStr.length() + 1];
    if (argList[numArgs] == NULL) {
      cout << "Internal error: new failed" << endl;
      exit(-1);
    } 
    strcpy(argList[numArgs], tempStr.c_str());
    numArgs++;
  }

  // Put in final NULL pointer
  argList[numArgs] = NULL;
}     

string Command::getArg(int index) const
{
  if (index < 0 || index >= numArgs) return "";
  return argList[index];
}
