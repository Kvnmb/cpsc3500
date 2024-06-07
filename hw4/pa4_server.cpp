// Kevin Bui
// March 9, 2024
// Last Revised: March 10, 2024
// pa4_client.cpp
// IDE: Visual Studio Code

#include <sys/types.h> // size_t, ssize_t
#include <sys/socket.h> // socket funcs
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // htons, inet_pton
#include <unistd.h> // close
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <ctime>
#include <sstream>
#include <cmath>

using namespace std;

const int MAXPENDING = 5;
const int MAXLOCATION = 100;
const int MINLOCATION = -100;

struct leaderboard {
    string username;
    long turns;
};

struct leaderboard lb[3];
long numEntries = 0;

pthread_mutex_t mutexLB;
pthread_mutex_t mutexEntries;

struct ThreadArgs {
    int clientSock; // socket to communicate with client
};

void *threadMain(void *args);

void processClient(int clientSock);

double distanceFormula(double x1, double y1, double x2, double y2);

int main(int argc, char* argv[])
{
    if(argc != 2){
        cout << "\nInvalid command line" << endl;
        exit(-1);
    }
    // initialize lock
    pthread_mutex_init(&mutexLB, NULL);
    pthread_mutex_init(&mutexEntries, NULL);

    // create listening socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0){
        cout << "Error with socket" << endl;
        exit(-1);
    }

    unsigned short servPort = stoi(argv[1]);

    // Convert dotted decimal address to int
    const char *IPAddr = "10.124.72.20";
    unsigned long servIP;
    int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
    if (status <= 0) exit(-1);

    struct sockaddr_in servAddr;
    // Set the fields
    servAddr.sin_family = AF_INET; // always AF_INET
    servAddr.sin_addr.s_addr = servIP;
    servAddr.sin_port = htons(servPort);

    // binds addr and port to socket
    status = bind(sock, (struct sockaddr *) &servAddr,
    sizeof(servAddr));
    if (status < 0) {
        cout << "Error with bind" << endl;
        exit(-1);
    }

    // set socket to listen
    status = listen(sock, MAXPENDING);
    if (status < 0) {
        cout << "Error with listen" << endl;
        exit(-1);
    }

    // while loop to check for an incoming connection
    while(true){
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientSock = accept(sock,(struct sockaddr *) &clientAddr,
        &addrLen);
        if (clientSock < 0) exit(-1);

        // Create and initialize argument struct
        ThreadArgs *args = new ThreadArgs;
        args -> clientSock = clientSock;

        // Create client thread
        pthread_t threadID;
        status = pthread_create(&threadID, NULL, threadMain,
        (void *) args);
        
        if (status != 0) exit(-1);
    }

    return 0;
}

void *threadMain(void *args)
{
    // Extract socket file descriptor from argument
    struct ThreadArgs *threadArgs = (struct ThreadArgs *) args;
    int clientSock = threadArgs->clientSock;
    delete threadArgs;

    // Communicate with client
    processClient(clientSock);

    // Reclaim resources before finishing
    pthread_detach(pthread_self());

    close(clientSock);
    return NULL;
}

void processClient(int clientSock)
{   
    unsigned seed = time(0);
    srand(seed); // seeds the random number generator with current time


    // initialize treasure location
    int treasureLocationX = rand() % (MAXLOCATION - MINLOCATION + 1) 
    + MINLOCATION;
    int treasureLocationY = rand() % (MAXLOCATION - MINLOCATION + 1)
    + MINLOCATION;

    cout << "(" << treasureLocationX  << ", " << treasureLocationY << ")"
    << endl;

    // receive username from client, length first

    long usernameLength;

    int bytesLeft = sizeof(long);
    long networkInt = 0;

    // char * used because char is 1 byte
    char *bp = (char *) &networkInt;

    while (bytesLeft) {
        int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
        if (bytesRecv <= 0) exit(-1);
        bytesLeft = bytesLeft - bytesRecv;
        bp = bp + bytesRecv;
    }
    usernameLength = ntohl(networkInt);

    // now extract name

    char * username = new char[usernameLength];

    bp = username;
    bytesLeft = usernameLength;

    while (bytesLeft) {
        int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
        if (bytesRecv <= 0) exit(-1);
        bytesLeft = bytesLeft - bytesRecv;
        bp = bp + bytesRecv;
    }


    // game start
    while(true){
        int turns = 1;
        // get length of guess
        int bytesLeft = sizeof(long);
        long networkInt = 0;

        // char * used because char is 1 byte
        char *bp = (char *) &networkInt;

        while (bytesLeft) {
            int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
            if (bytesRecv <= 0) exit(-1);
            bytesLeft = bytesLeft - bytesRecv;
            bp = bp + bytesRecv;
        }
        long guessLength = ntohl(networkInt);

        // get guess
        char *guess = new char[guessLength];
        bp = guess;
        bytesLeft = guessLength;

        while (bytesLeft) {
            int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
            if (bytesRecv <= 0) exit(-1);
            bytesLeft = bytesLeft - bytesRecv;
            bp = bp + bytesRecv;
        }
        
        stringstream ss;
        string x, y;
        ss.clear();
        ss.str(guess);
        getline(ss, x, ' ');
        getline(ss, y, ' ');

        delete [] guess;

        double guessX = stod(x);
        double guessY = stod(y);

        float distance = distanceFormula(guessX, guessY, treasureLocationX, treasureLocationY);

        // send float to client

        int bytesSent = send(clientSock, (void *) &distance, sizeof(float), 0);
        if (bytesSent != sizeof(float)) exit(-1);

        // win con
        if(distance == 0.0){
            // receive num of turns it took
            bytesLeft = sizeof(long);
            networkInt = 0;

            // char * used because char is 1 byte
            bp = (char *) &networkInt;

            while (bytesLeft) {
                int bytesRecv = recv(clientSock, bp, bytesLeft, 0);
                if (bytesRecv <= 0) exit(-1);
                bytesLeft = bytesLeft - bytesRecv;
                bp = bp + bytesRecv;
            }
            long numTurns = ntohl(networkInt);

            pthread_mutex_lock(&mutexLB);
            pthread_mutex_lock(&mutexEntries);
            if(numEntries < 3){
                lb[numEntries].username = username;
                lb[numEntries].turns = turns;
                numEntries++;

                // send to user
                networkInt = htonl(numEntries);
                bytesSent = send(clientSock, (void *) &networkInt, sizeof(long), 0);
                if (bytesSent != sizeof(long)) exit(-1);
                
            }
            pthread_mutex_unlock(&mutexLB);
            pthread_mutex_unlock(&mutexEntries);

            break;
        }
        turns++;
    }

    delete [] username;
    
}

double distanceFormula(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2-y1;

    return sqrt((dx * dx) + (dy * dy));
}