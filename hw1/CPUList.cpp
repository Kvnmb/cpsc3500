// Kevin Bui
// January 15, 2024
// Last Revised: January 15, 2024
// CPUList.cpp
// IDE: Visual Studio Code

#include <iostream>
#include "CPUList.h"
#include <fstream>
#include <cmath>
#include <iomanip>

using namespace std;

CPUList::CPUList(ifstream& inFile)
{
    int processNum, arrivalNum, burstNum;
    
    // sorts processes by lower ID and by arrival time
    while(inFile >> processNum >> arrivalNum >> burstNum){
        numProcesses++;
        Node *process = new Node;
        process->processID = processNum;
        process->arrivalTime = arrivalNum;
        process->burstTime = burstNum;
        sort(process);
    }
}

// insert function for LL
void CPUList::sort(Node *newNode)
{
    if(!head){
        head = newNode;
        head->next = nullptr;
    }else{
        Node *currPtr = head;
        Node *prevPtr = nullptr;
        while(currPtr && currPtr->arrivalTime <= newNode->arrivalTime) {
            // checks for same arrival time and sorts by processID
            if(currPtr->arrivalTime == newNode->arrivalTime){
                if(currPtr->processID > newNode->processID) break;
            }
            prevPtr = currPtr;
            currPtr = currPtr->next;
        }
        if(!currPtr){
            prevPtr->next = newNode;
            newNode->next = nullptr;
        }else if(!prevPtr){
            newNode->next = head;
            head = newNode;
        }else{
            prevPtr->next = newNode;
            newNode->next = currPtr;
        }
    }
}

void CPUList::runSchedule(string algorithm, int quantum)
{   
    if(algorithm != "RR" && quantum != 0){
        cout << "\nInvalid argument supplied.\n";
        return;
    }
    if(algorithm == "FCFS"){
        FCFS();
    }else if(algorithm == "SRT"){
        SRT();
    }else if(algorithm == "RR"){
        if(quantum == 0){
            cout << "\nNo time quantum supplied.\n";
            return;
        }
        RR(quantum);
    }else cout << "\nInvalid algorithm." ;
}

void CPUList::printStats(int elapsedTime, int idleTime)
{
    cout << "\nCPU Utilization: " << 
    round(((elapsedTime - idleTime) / double(elapsedTime)) * 100) << "%";

    cout << "\nAverage waiting time: " << fixed << setprecision(2) 
    << double(totalWait / double(numProcesses));

    cout << "\nWorst-case waiting time: " << maxWait;
}

void CPUList::calcValues(Node* currPtr)
{
    totalWait += calcWait(currPtr);

    maxWait = max(maxWait, calcWait(currPtr));
}

void CPUList::FCFS()
{
    int elapsedTime = 0;
    int idleTime = 0;

    Node *currPtr = head;
    // traverses from beginning of list to end since
    // the list is sorted by arrival time
    while(currPtr){
        // checks for idle time
        if(elapsedTime < currPtr->arrivalTime){
            cout << "\nTime " << elapsedTime << " Idle" ;
            idleTime += currPtr->arrivalTime - elapsedTime;
            elapsedTime = currPtr->arrivalTime;;
        }else{
            // adds process burst time to elapsed time
            cout << "\nTime " << elapsedTime << " Process " 
            << currPtr->processID;
            elapsedTime += currPtr->burstTime;
            currPtr->terminationTime = elapsedTime;

            calcValues(currPtr);

            currPtr = currPtr->next;
        }
    }
    
    printStats(elapsedTime, idleTime);
}

void CPUList::SRT()
{
    int elapsedTime = 0;
    int idleTime = 0;
    int lastProcess = -1;
    Node *queuePtr = nullptr; // ready queue
    Node *temp = nullptr;
    Node *prevPtr = nullptr;

    // loop runs until both lists are empty (processes have run)
    while(head || queuePtr){
        // if nothing is in ready queue, takes first process in head
        if(!queuePtr){
            queuePtr = head;
            head = head->next;
            queuePtr->next = nullptr;
        }
        // idle check
        if(elapsedTime < queuePtr->arrivalTime){
            cout << "\nTime " << elapsedTime << " Idle" ;
            idleTime += queuePtr->arrivalTime - elapsedTime;
            elapsedTime = queuePtr->arrivalTime;
        }

        // this loop sorts the node from head into the queue by burst time
        while(head && head->arrivalTime == elapsedTime){
            prevPtr = nullptr;
            temp = queuePtr;

            // loop to traverse ready queue and find spot for node
            while(temp && ((temp->burstTime - temp->CPUTime <= head->burstTime)
            && temp->processID < head->processID)){
                prevPtr = temp;
                temp = temp->next;
            }

            // end of the list
            if(!temp){
                prevPtr->next = head;
                head = head->next;
                prevPtr->next->next = nullptr;
            }else{
                // beginning of list
                if(!prevPtr){
                    queuePtr = head;
                    head = head->next;
                    queuePtr->next = temp;
                // middle of list
                }else{
                    prevPtr->next = head;
                    head = head->next;
                    prevPtr->next->next = temp;
                }
            }
        }

        // set node to beginning of queue
        Node *currPtr = queuePtr;
        prevPtr = nullptr;
        while(currPtr){
            // will not print when the same process is on cpu
            if(lastProcess != currPtr->processID){
                cout << "\nTime " << elapsedTime << 
                " Process " << currPtr->processID;
            }

            lastProcess = currPtr->processID;

            // adds to elapsed time until new node arrives
            while(currPtr->burstTime != currPtr->CPUTime){
                if(head && head->arrivalTime == elapsedTime){
                    break;
                }
                currPtr->CPUTime++;
                elapsedTime++;
            }

            // checks for a finished process
            if(currPtr->burstTime == currPtr->CPUTime){
                currPtr->terminationTime = elapsedTime;
                lastProcess = currPtr->processID;

                calcValues(currPtr);

                // if the finished process is at the beginning of queue
                if(!prevPtr){
                    temp = currPtr;
                    currPtr = currPtr->next;
                    queuePtr = currPtr;
                    delete temp;
                }else{
                    temp = currPtr;
                    prevPtr->next = currPtr->next;
                    delete temp;
                    currPtr = prevPtr->next;
                }
            }else{
                // if process does not delete keep going
                lastProcess = currPtr->processID;
                prevPtr = currPtr;
                currPtr = currPtr->next;
            }

            // when new node arrives, break loop to start sorting
            if(head && head->arrivalTime == elapsedTime){
                break;
            }
        }
    }
    printStats(elapsedTime, idleTime);
}

void CPUList::RR(int quantum)
{
    int elapsedTime = 0;
    int idleTime = 0;
    int lastProcess = -1;
    Node *queuePtr = nullptr;
    Node *temp = nullptr;

    // loop runs until both lists are empty (processes have run)
    while(head || queuePtr){
        // if nothing is in ready queue, takes first process in head
        if(!queuePtr){
            queuePtr = head;
            head = head->next;
            queuePtr->next = nullptr;
        }
        // idle check
        if(elapsedTime < queuePtr->arrivalTime){
            cout << "\nTime " << elapsedTime << " Idle" ;
            idleTime += queuePtr->arrivalTime - elapsedTime;
            elapsedTime = queuePtr->arrivalTime;
        }

        // this loop puts nodes that just arrived at end of ready queue
        while(head && head->arrivalTime == elapsedTime){
            temp = queuePtr;
            if(temp){
                while(temp->next){
                    temp = temp->next;
                }
                temp->next = head;
                head = head->next;
                temp->next->next = nullptr;
            }
        }

        // set node to beginning of queue
        Node *currPtr = queuePtr;
        Node *prevPtr = nullptr;
        while(currPtr){

            // this loop will add processes that arrive right as
            // a process ends in the middle of the queue
            
            while(head && head->arrivalTime == elapsedTime){
                temp = head;
                head = head->next;
                
                prevPtr->next = temp;
                temp->next = currPtr;
                prevPtr = temp;
            }

            if(lastProcess != currPtr->processID){
                cout << "\nTime " << elapsedTime << " Process "
                << currPtr->processID;
            }
            // if the time quantum must be fully used
            if(currPtr->CPUTime < currPtr->burstTime - quantum){
                currPtr->CPUTime += quantum;
                elapsedTime += quantum;
            }else{
            // process ends before time quantum
                elapsedTime += currPtr->burstTime - currPtr->CPUTime;
                currPtr->CPUTime = currPtr->burstTime;
                currPtr->terminationTime = elapsedTime;
            }


            // checks for functions that arrive during a process's quantum
            while(head && head->arrivalTime < elapsedTime){
                temp = head;
                head = head->next;
                if(!prevPtr){
                    temp->next = currPtr;
                    queuePtr = temp;
                    prevPtr = temp;
                }else{
                    prevPtr->next = temp;
                    temp->next = currPtr;
                    prevPtr = temp;
                }
            }



            // checks for finished process
            if(currPtr->burstTime == currPtr->CPUTime){
                lastProcess = currPtr->processID;

                calcValues(currPtr);

                if(!prevPtr){
                    temp = currPtr;
                    currPtr = currPtr->next;
                    queuePtr = currPtr;
                    delete temp;
                }else{
                    temp = currPtr;
                    prevPtr->next = currPtr->next;
                    delete temp;
                    currPtr = prevPtr->next;
                }
            }else{
            // continues process if not deleted
                lastProcess = currPtr->processID;
                prevPtr = currPtr;
                currPtr = currPtr->next;
            }
        }
    }
    printStats(elapsedTime, idleTime);
}


int CPUList::calcWait(Node *node)
{   
    return node->terminationTime 
    - node->arrivalTime - node->burstTime;
}





// destructor
CPUList::~CPUList()
{
    Node *currPtr;
    Node *nextNode;

    currPtr = head;

    while(currPtr){
        nextNode = currPtr->next;
        delete currPtr;
        currPtr = nextNode;
    }
}