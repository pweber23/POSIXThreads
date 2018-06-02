/*******************************************
 * Paul Weber
 * 1306210
 * COSC 3380
 * Assignment 3
 * Posix Threads and mutex application simulator
 * *******************************************/


#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <queue>
#include <iostream>
#include <string>
#include <iomanip>
#include <pthread.h>
#include <cstdint>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <signal.h>

using namespace std;

static pthread_mutex_t traffic_lock;

static pthread_mutex_t WB_access;

static pthread_mutex_t BB_access;

static pthread_cond_t clear;

static int whittierBound;

static int bearBound;

static int arrivedForWhittier;
int tunnellFullCount = 0;

static int arrivedForBearValley;

static int maxCarsInTunnel;
static int numCarsInTunnel = 0;

static string dir;

static vector<int> arrivalArray;
static vector<int> tunnelArray;
static vector<string> destinationArray;
static int WB;
static int BB;
static int maxCar;
static int internalCounter;
static int delay;

static int totalCars;



/******************************************
*
* Create New thread for Whittier-bound cars
*
*******************************************/

void* car_whittier(void** arg) {
    int carID = (int)arg[0];
    
    cout << "Car #" << carID << " going to Whittier arrives at the tunnel." << endl;
    
    arrivedForWhittier++;
    
    if(numCarsInTunnel >= maxCarsInTunnel && dir == "WB") {
            tunnellFullCount++;
    }
    
    
    while(numCarsInTunnel >= maxCarsInTunnel || dir != "WB") {
        pthread_cond_wait(&clear, &traffic_lock);
    }
    pthread_mutex_unlock(&traffic_lock);
    
    cout << "Car #" << carID << " going to Whittier enters the tunnel." << endl;
    numCarsInTunnel++;
    
    sleep(((int)arg[1]));
    
    cout << "Car #" << carID << " going to Whittier exits the tunnel." << endl;
    numCarsInTunnel--;
    
    pthread_exit((void*) 0);
}

/**********************************************
*
* Create New thread for Bear Valley-bound cars
*
***********************************************/

void*  car_bearValley(void** arg) {
    int carID = (int)arg[0];
    
    cout << "Car #" << carID << " going to Bear Valley arrives at the tunnel." << endl;
    
    arrivedForBearValley++;
    
    if(numCarsInTunnel >= maxCarsInTunnel && dir == "BB") {
            tunnellFullCount++;
    }
    
    while(numCarsInTunnel >= maxCarsInTunnel || dir != "BB") {
        pthread_cond_wait(&clear, &traffic_lock);
    }
    pthread_mutex_unlock(&traffic_lock);
    
    
    cout << "Car #" << carID << " going to Bear Valley enters the tunnel." << endl;
    numCarsInTunnel++;
    
    sleep(((int)arg[1]));
    
    cout << "Car #" << carID << " going to Bear Valley exits the tunnel." << endl;
    numCarsInTunnel--;
    
    pthread_exit((void*) 0);
    
}

/***********************************************
*
* Create New thread for tunnel
*
************************************************/

void* tunnel(void* arg) {
    
    while(true) {
        
        cout << "The tunnel is now open to Whittier‐bound traffic." << endl;
        pthread_mutex_lock(&traffic_lock);
        dir = "WB";
        pthread_cond_broadcast(&clear);
        pthread_mutex_unlock(&traffic_lock);
        sleep(5);
        
        cout << "The tunnel is now closed to all traffic." << endl;
        pthread_mutex_lock(&traffic_lock);
        dir = "NO";
        pthread_cond_broadcast(&clear);
    
        sleep(5);
        pthread_mutex_unlock(&traffic_lock);
        
        cout << "The tunnel is now open to Bear Valley‐bound traffic." << endl;
        pthread_mutex_lock(&traffic_lock);
        dir = "BB";
        pthread_cond_broadcast(&clear);
        sleep(5);
        pthread_mutex_unlock(&traffic_lock);
        
        
        cout << "The tunnel is now closed to ALL traffic." << endl;
        pthread_mutex_lock(&traffic_lock);
        dir = "NO";
        pthread_cond_broadcast(&clear);
       pthread_mutex_unlock(&traffic_lock);
        
        sleep(5);
    }
}

/***********************************************
*
* Print summary of tunnel traffic
*
************************************************/

void printSummary() {
    cout << endl;
    cout << arrivedForWhittier << " car(s) going to Whittier arrived at the tunnel." << endl;
    cout << arrivedForBearValley << " car(s) going to Bear Valley arrived at the tunnel." << endl;
    cout << tunnellFullCount << " car(s) had to wait because the tunnel was full." << endl;
}

/**************************************************
*
* Main Process
*
***************************************************/

int main(int argc, char* argv[])
{
    
    string fileName;
        cout <<"Enter file name: ";
        cin >> fileName;
        ifstream inFile;
        inFile.open(fileName.c_str(), ios::in);
        
        if(inFile.fail())
        {
            cerr << "Error Opening File" << endl;
            exit(1);
        }
        int a,b,size;
        size =1;
        int carCounter =1;
        int arrivalTime;
        int tunnelTime;
        string destination;
        inFile >> maxCar;
        string ignoreFirst;
        getline(inFile, ignoreFirst);
        while(inFile >> arrivalTime >> destination >> tunnelTime)
        {
            arrivalArray.resize(size, arrivalTime);
            tunnelArray.resize(size, tunnelTime);
            destinationArray.resize(size, destination);
            if(destination =="WB")
            {
                WB++;
                totalCars++;
                size++;
            }
            else if(destination =="BB")
            {
                BB++;
                totalCars++;
                size++;
            }
            
        }
        
    cout << "arrivalArray contains:";
    for (int i=0;i<arrivalArray.size();i++)
        cout << ' ' << arrivalArray[i];
    cout << '\n';
   
    cout << "tunnelArray contains:";
    for (int i=0;i<tunnelArray.size();i++)
        cout << ' ' << tunnelArray[i];
    cout << '\n';
    
    cout << "destinationArray contains:";
    for (int i=0;i<destinationArray.size();i++)
        cout << ' ' << destinationArray[i];
    cout << '\n';
    
    int numCars = arrivalArray.size();
    
    maxCarsInTunnel = maxCar;
    
    int travelTime;
    
    int counter = 1;
    int pid = 0;
    
    
    pthread_t tid;
    pthread_t cartid[numCars];
    
    pthread_mutex_init(&traffic_lock, NULL);
    
    pthread_create(&tid, NULL, tunnel, (void*) numCars);
    
    while(pid < numCars) {
        
        sleep(arrivalArray[pid]);
        
        travelTime = tunnelArray[pid];
        
        int* args[2];
        
        args[0] = counter;
        
        args[1] = travelTime;
        
        if(destinationArray[pid] == "WB") {
            pthread_create(&cartid[pid], NULL, car_whittier, (void**) args);
        }
        else {
            pthread_create(&cartid[pid], NULL, car_bearValley, (void**) args);
        }
        
        pid++;
        counter++;
    }
    
    for(int i = 0; i < numCars; i++) {
        pthread_join(cartid[i], NULL);
    }
    
    pthread_kill(tid, 0);
    
    printSummary();
    
    return 0;
}