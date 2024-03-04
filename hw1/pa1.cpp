// Kevin Bui
// January 15, 2024
// Last Revised: January 15, 2024
// pa1.cpp
// IDE: Visual Studio Code

#include <iostream>
#include <fstream>
#include <string>
#include "CPUList.h"

using namespace std;


int main(int argc, char* argv[])
{
    ifstream inFile(argv[1]);
    if(inFile){
        CPUList list(inFile);

        // checks for whether to supply time quantum for function
        // as well as a valid number of arguments
        if(argc == 3){
            list.runSchedule(argv[2]);
        }else if(argc == 4){
            try{
                list.runSchedule(argv[2], stoi(argv[3]));
            }catch(const exception&){
                cout << "\nInvalid quantum supplied.";
                return 0;
            }
        }else{
            cout << "\nInvalid number of arguments.\n" ;
        }
        inFile.close();
    }else cout << "\nError with the file.";

    cout << endl;

    return 0;
}

