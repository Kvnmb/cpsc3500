// Kevin Bui
// January 27, 2024
// Last Revised: January 29, 2024
// IDE: Visual Studio Code
// vowels.cpp: multithreaded vowel count

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <pthread.h>
using namespace std;

const int NUM_FILES = 20;
const char vowels[5] = {'A', 'E', 'I', 'O', 'U'};
const int NUM_VOWELS = 5;

void* processFile(void *fileName);

int main()
{
    int vowelCount[NUM_VOWELS] = {0};
    const string directory = "/home/fac/elarson/cpsc3500/files/";
    string *fileName;
    pthread_t tid[NUM_FILES];

    // puts threads to work on each file
    // does not abort program if thread has error
    for(int x = 1; x <= NUM_FILES; x++){
        // creates dynamic memory to pass into thread argument parameter
        fileName = new string(directory + "file" + to_string(x) + ".txt");
        if(pthread_create(&tid[x - 1], NULL, processFile,
          (void *)fileName) != 0){
            perror("\nError creating thread");
            exit(-1);
        }
    }

    // pointers to store return values from pthread_join
    int *returnedArray = nullptr;
    int **returnedArrayPointer = &returnedArray;

    for(int x = 0; x < NUM_FILES; x++){
        // phthread_join returns an array of the number of each
        // vowel in their file
        if(pthread_join(tid[x], (void **) returnedArrayPointer) != 0){
            perror("\nError with thread join");
            exit(-1);
        }
        // checks for if thread did not open file
        if(!returnedArray) continue;

        // add values to main array then delete dynamic memory
        for(int x = 0; x < NUM_VOWELS; x++){
            vowelCount[x] += returnedArray[x];
        }
        delete [] returnedArray;
        returnedArray = nullptr;
    }
    
    // print
    for(int x = 0; x < NUM_VOWELS; x++){
        cout << endl << vowels[x] << ": " << vowelCount[x];
    }
    cout << endl;

    return 0;
}

void *processFile(void *fileName)
{
    // converts void pointer to correct type
    string *input = (string *) fileName;
    string file = *input;

    // frees memory after storing it in local variable
    delete input;
    
    ifstream inFile(file);

    if(inFile){
        char fileValue;
        int *countVowels = new int[NUM_VOWELS]();
        // while loop and switch case to check for vowels until EOF
        while(inFile >> fileValue){
            // increment if vowel
            switch(fileValue){
                case 'a':
                case 'A':
                    countVowels[0]++;
                    break;
                case 'e':
                case 'E':
                    countVowels[1]++;
                    break;
                case 'i':
                case 'I':
                    countVowels[2]++;
                    break;
                case 'o':
                case 'O':
                    countVowels[3]++;
                    break;
                case 'u':
                case 'U':
                    countVowels[4]++;
                    break;
                default:
                    break;
            }
        }
        inFile.close();

        // return the int array of vowels counted
        pthread_exit((void *) countVowels);
    }else{
        cout << "\nFile " << file << " failed";
        // returns null for error check in main
        pthread_exit(NULL);
    }
}