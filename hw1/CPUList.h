// Kevin Bui
// January 15, 2024
// Last Revised: January 15, 2024
// CPUList.h
// IDE: Visual Studio Code

#ifndef CPULIST_H
#define CPULIST_H
#include <iostream>
#include <fstream>

using namespace std;


// Class invariants: CPU scheduler is implemented as a linked list

// Object stores the number of processes, total and max wait time, and the
// total burst time for calculations


class CPUList
{
    private:
    struct Node{ 
        int processID = 0;
        int arrivalTime = 0;
        int burstTime = 0;
        int CPUTime = 0;
        int terminationTime = 0;
        Node *next = nullptr;
    };
    Node *head = nullptr;
    int numProcesses = 0;
    int totalWait = 0;
    int maxWait = 0;

    // sorts list based on its arrival time
    // precondition: valid file inputted
    void sort(Node *newNode);
    // postcondition: linked list is sorted in ascending order
    // of arrival time

    // calls processes based on arrival time
    // precondition: valid file inputted
    void FCFS();
    // postcondition: processes are scheduled in FCFS and
    // data is calculated

    // calls processes based on time quantum scheduling
    // precondition: time quantum given
    void RR(int quantum);
    // postcondition: processes are scheduled in RR and
    // data is calculated

    // sorts list based on shortest burst time
    // precondition: valid file inputted
    void SRT();
    // postcondition: processes are scheduled in SRT and
    // data is calculated

    // returns the wait time of a terminated process
    // precondition: valid node inputted
    int calcWait(Node *node);
    // postcondition: returns wait time

    // prints scheduler data
    // precondition: processes have finished running
    void printStats(int elapsedTime, int idleTime);
    // postcondition: calculated data values are printed

    // calculates data values
    // precondition: process has finished running
    void calcValues(Node* currPtr);
    // postcondition: calculated data values are stored

    public:
    // constructor
    // precondition: valid file name inputted
    CPUList(ifstream& inFile);
    // postcondition: construction of a linked list in object

    // chooses which algorithm to run
    // precondition: valid algorithm name and quantum inputted for RR
    void runSchedule(string algorithm, int quantum = 0);
    // postcondition: runs the given process

    // destructor
    // precondition: none
    ~CPUList(); 
    // postcondition: dynamic memory is freed
};


#endif


// Implementation Invariants: Nodes hold data for each process

// Termination time is calculated by setting it equal to when
// the CPUTime is equal to the burstTime

// CPUTime is incremented as the algorithm runs

// Linked list is sorted based on arrival time

