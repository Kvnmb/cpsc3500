// Command.h: Stores a parsed command line.

#ifndef COMMAND_H
#define COMMAND_H

#include <string>
using namespace std;

class Command
{
  public:

    // Constructor
    Command();

    // Destructor
    ~Command();

    // Divides commandStr into a list of arguments using white space
    // as a delimiter.
    void parseCommandString(string commandStr);

    // Returns the number of arguments in the parsed command line.
    int getNumArgs() const { return numArgs; }

    // Returns the corresponding argument using the provided index.
    // Returns "" if the provided index is out of bounds.
    string getArg(int index) const; 

    // Returns a list of arguments.
    char **getArgList() const { return argList; }

  private:

    int numArgs;	// number of arguments
    char **argList;	// argument array (NULL if numArgs is 0)
};
  
#endif
