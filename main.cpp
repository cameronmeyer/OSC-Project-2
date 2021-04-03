// Author: Cameron Meyer (cdm180003)
// Project 1
// CS 4348.001


#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <queue>

using namespace std;

int doctorCount = 0;
int patientCount = 0;

pthread_t* patients;
pthread_t reception;
pthread_t* nurses;
pthread_t* doctors;

sem_t patientRegistration;
/*sem_t 
sem_t 
sem_t 
sem_t 
sem_t 
sem_t 
sem_t */

queue<int> patientsAwaitingRegistration;
queue<int>*  patientsAwaitingDoctor;

void *patient(void *patientID)
{
    //patientsAwaitingRegistration.push(patientID);
    pthread_exit(NULL);
}

void *receptionist(void *receptionistID)
{
    while(true)
    {
        //int patientID = patientsAwaitingRegistration.pop();
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if(argc == 3)
    {
        try
        {
            // Assign number of doctors and patients
            string::size_type sz;
            doctorCount = stoi(argv[1], &sz);
            patientCount = stoi(argv[2], &sz);
        }
        catch(...)
        {
            string errorMessage = "Error: Arguments could not be converted to valid integers.";
            throw new invalid_argument(errorMessage);
            exit(-1);
        }
    }
    else
    {
        string errorMessage = "Error: Invalid argument list. Expected 2 arguments, but found " + to_string(argc-1) + ".";
        throw new invalid_argument(errorMessage);
        exit(-1);
    }

    if(doctorCount < 1 || doctorCount > 3)
    {
        string errorMessage = "Error: Invalid number of doctors. Expected between 1-3 doctors, but found " + to_string(doctorCount) + ".";
        throw new invalid_argument(errorMessage);
        exit(-1);
    }

    if(patientCount < 1 || patientCount > 30)
    {
        string errorMessage = "Error: Invalid number of patients. Expected between 1-30 patients, but found " + to_string(patientCount) + ".";
        throw new invalid_argument(errorMessage);
        exit(-1);
    }

    pthread_t patientThreads[patientCount];
    pthread_t doctorThreads[doctorCount];
    pthread_t nurseThreads[doctorCount];
    int threadSuccess;

    for(int i = 0; i < patientCount; i++)
    {
        // Create patient threads
    }

    for(int i = 0; i < doctorCount; i++)
    {
        // Create doctor threads
    }
    
    return 0;
}