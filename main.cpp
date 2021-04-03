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
//pthread_t reception;
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
    int pID = *(int *) &patientID;
    printf("print from patient %i\n", pID);
    pthread_exit(NULL);
}

void *receptionist(void *receptionistID)
{
    while(true)
    {
        //int patientID = patientsAwaitingRegistration.pop();
        printf("print from receptionist\n");
        break;
    }

    pthread_exit(NULL);
}

void *nurse(void *nurseID)
{
    //while(true)
    //{
        int nID = *(int *) &nurseID;
        printf("print from nurse %i\n", nID);
        //break;
    //}

    pthread_exit(NULL);
}

void *doctor(void *doctorID)
{
    //while(true)
    //{
    int dID = *(int *) &doctorID;
    printf("print from doctor %i\n", dID);
        //break;
    //}

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

    patients = new pthread_t[patientCount];
    nurses = new pthread_t[doctorCount];
    doctors = new pthread_t[doctorCount];
    int threadError;

    for(int i = 0; i < patientCount; i++)
    {
        // Create patient threads
        threadError = pthread_create(&patients[i], NULL, patient, (void *) i);

        if(threadError)
        {
            cout << "Error: Unable to create patient thread with id " + to_string(i) + ".";
            exit(-1);
        }
    }

    for(int i = 0; i < doctorCount; i++)
    {
        // Create doctor and nurse threads
        threadError = pthread_create(&nurses[i], NULL, nurse, (void *) i);

        if(threadError)
        {
            cout << "Error: Unable to create nurse thread with id " + to_string(i) + ".";
            exit(-1);
        }

        threadError = pthread_create(&doctors[i], NULL, doctor, (void *) i);

        if(threadError)
        {
            cout << "Error: Unable to create doctor thread with id " + to_string(i) + ".";
            exit(-1);
        }
    }

    //printf("doctor count: %i\n", doctorCount);
   // for(int i = 0; i < doctorCount; i++)
    //{
    //    printf("doctor threads: %i\n", doctors[i]);
    //}
    
    
    return 0;
}