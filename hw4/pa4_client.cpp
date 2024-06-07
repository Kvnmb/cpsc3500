// Kevin Bui
// March 9, 2024
// Last Revised: March 9, 2024
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
#include <sstream>
#include <iomanip>

using namespace std;

int main(int argc, char* argv[])
{
    if(argc != 3){
        cout << "Invalid command line";
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0){
        cout << "Error with socket" << endl;
        exit(-1);
    }

    char *IPAddr = argv[1];
    unsigned short servPort = (unsigned short)stoi(argv[2]);

    // Convert dotted decimal address to int
    unsigned long servIP;
    int status = inet_pton(AF_INET, IPAddr, (void *) &servIP);
    if (status <= 0) exit(-1);

    // set the fields
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET; // always AF_INET
    servAddr.sin_addr.s_addr = servIP;
    servAddr.sin_port = htons(servPort);

    status = connect(sock, (struct sockaddr *) &servAddr,
    sizeof(servAddr));
    if (status < 0) {
        cout << "Error with connect" << endl;
        exit(-1);
    }

    // connection successful

    string username;
    cout << "\nWelcome to Treasure Hunt!\nEnter your name:  ";
    getline(cin, username);

    const char *msg = username.c_str();
    long msgLength = username.length();

    // send username length
    long networkInt = htonl(msgLength);
    int bytesSent = send(sock, (void *) &networkInt, sizeof(long), 0);
    if (bytesSent != sizeof(long)) exit(-1);
    
    bytesSent = send(sock, (void *) msg, msgLength, 0);
    if (bytesSent != msgLength) exit(-1);

    
    long turns = 1;
    string x, y;
    stringstream ss;
    // loop until game ends
    while(true){
        string guess = "";
        cout << "\n\nTurn: " << turns;
        cout << "\nEnter a guess (x y): ";
        
        while(true){
            getline(cin, guess);
            ss.clear();
            ss.str(guess);
            getline(ss, x, ' ');
            getline(ss, y, ' ');
            
            ss >> x >> y;

            int xCheck = stoi(x);
            int yCheck = stoi(y);

            // validity check
            if(xCheck < -100 || xCheck > 100 || yCheck < -100 || yCheck > 100){
                cout << "\nInvalid input, try again: ";
            }else break;
        }

        // now send guesses to server

        // give delimiter to parse later
        string sendGuess = x + " " + y;
        msg = sendGuess.c_str();
        long guessLength = sendGuess.length();

        // send guess to server
        long networkInt = htonl(guessLength);
        bytesSent = send(sock, (void *) &networkInt, sizeof(long), 0);
        if (bytesSent != sizeof(long)) exit(-1);
        
        bytesSent = send(sock, (void *) msg, guessLength, 0);
        if (bytesSent != msgLength) exit(-1);


        // receive distance from treasure (float)
        float distance;

        int bytesLeft = sizeof(float);
        float networkFloat = 0;

        // char * used because char is 1 byte
        char *bp = (char *) &networkFloat;

        while (bytesLeft) {
            int bytesRecv = recv(sock, bp, bytesLeft, 0);
            if (bytesRecv <= 0) exit(-1);
            bytesLeft = bytesLeft - bytesRecv;
            bp = bp + bytesRecv;
        }
        distance = networkFloat;

        cout << endl << fixed << setprecision(2) << "Distance to treasure: "
        << distance << endl;


        // check for win condition
        if(distance == 0.0){
            cout << "\nCongratulations! You found the treasure!!" << endl
            << "It took " << turns << "turns to find the treasure.\n\n" ;

            // send turn count to server for leaderboard check
            networkInt = htonl(turns);
            int bytesSent = send(sock, (void *) &networkInt, sizeof(long), 0);
            if (bytesSent != sizeof(long)) exit(-1);

            break;
        }
        // not correct guess
        turns++;
    }

    // check leaderboard after game

    // get length of leaderboard first
    int bytesLeft = sizeof(long);
    networkInt = 0;

    // char * used because char is 1 byte
    char *bp = (char *) &networkInt;

    while (bytesLeft) {
        int bytesRecv = recv(sock, bp, bytesLeft, 0);
        if (bytesRecv <= 0) exit(-1);
        bytesLeft = bytesLeft - bytesRecv;
        bp = bp + bytesRecv;
    }
    long leaderboardLength = ntohl(networkInt);

    // get lb
    char* leaderboard = new char[leaderboardLength];
    bp = leaderboard;
    bytesLeft = leaderboardLength;

    while (bytesLeft) {
        int bytesRecv = recv(sock, bp, bytesLeft, 0);
        if (bytesRecv <= 0) exit(-1);
        bytesLeft = bytesLeft - bytesRecv;
        bp = bp + bytesRecv;
    }
    
    cout << "\nLeaderboard: " << endl << leaderboard << endl;

    close(sock);

    delete [] leaderboard;
    return 0;
}