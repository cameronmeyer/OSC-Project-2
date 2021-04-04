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
int exitedPatients = 0;
int* patientsInAppointment;

pthread_t* patients;
pthread_t reception;
pthread_t* nurses;
pthread_t* doctors;

sem_t awaitReceptionist;
sem_t* registerPatient;
sem_t* receptionDeskDeparture;
sem_t* awaitNurse;
sem_t* waitingRoomDeparture;
sem_t* doctorOfficeArrival;
sem_t* doctorBeginAppointment;
sem_t* listenSymptoms;
sem_t* listenAdvice;
sem_t* doctorEndAppointment;
sem_t adjustExitedPatients;

queue<int> patientsAwaitingRegistration;
queue<int>*  patientsAwaitingDoctor;

void *patient(void *patientID)
{
    int pID = *(int *) &patientID;
    int doctorID = rand() % doctorCount;

    // Enter room and wait for receptionist
    printf("Patient %i enters waiting room, waits for receptionist\n", pID);
    patientsAwaitingRegistration.push(pID);
    sem_post(&awaitReceptionist);

    // Wait for receptionist to finish, then leave the receptionist desk
    sem_wait(&registerPatient[pID]);
    printf("Patient %i leaves receptionist and sits in waiting room\n", pID);
    sem_post(&receptionDeskDeparture[pID]);

    // Wait to see your doctor
    patientsAwaitingDoctor[doctorID].push(pID);
    sem_post(&awaitNurse[doctorID]);

    // Wait to be taken to the doctor's office
    sem_wait(&waitingRoomDeparture[doctorID]);

    // Signal that we are entering the office
    printf("Patient %i enters doctor %i's office\n", pID, doctorID);
    sem_post(&doctorOfficeArrival[doctorID]);

    // Wait for the doctor to listen to your symptoms
    sem_wait(&listenSymptoms[doctorID]);

    // Signal the doctor to begin the advice
    printf("Patient %i receives advice from doctor %i\n", pID, doctorID);
    sem_post(&listenAdvice[doctorID]);

    // Patient leaves clinic
    sem_wait(&adjustExitedPatients);
    printf("Patient %i leaves\n", pID);
    if(++exitedPatients == patientCount){ exit(0); }
    sem_post(&adjustExitedPatients);

    pthread_exit(NULL);
}

void *receptionist(void *receptionistID)
{
    while(true)
    {
        // When a patient needs registration, retreive them from the queue
        sem_wait(&awaitReceptionist);
        int patientID = patientsAwaitingRegistration.front();
        patientsAwaitingRegistration.pop();

        // Signal the patient to leave the receptionist desk
        printf("Receptionist registers patient %i\n", patientID);
        sem_post(&registerPatient[patientID]);
        sem_wait(&receptionDeskDeparture[patientID]);
    }

    pthread_exit(NULL);
}

void *nurse(void *nurseID)
{
    int nID = *(int *) &nurseID;
    
    while(true)
    {
        // Wait for a patient to want to see your doctor
        sem_wait(&awaitNurse[nID]);
        int patientID = patientsAwaitingDoctor[nID].front();
        patientsAwaitingDoctor[nID].pop();

        // Take patient to doctor's office
        printf("Nurse %i takes patient %i to doctors office\n", nID, patientID);
        sem_post(&waitingRoomDeparture[nID]);

        sem_wait(&doctorOfficeArrival[nID]);

        // Signal doctor that a patient is ready to be seen
        patientsInAppointment[nID] = patientID;
        sem_post(&doctorBeginAppointment[nID]);

        // Wait for appointment to complete
        sem_wait(&doctorEndAppointment[nID]);
    }

    pthread_exit(NULL);
}

void *doctor(void *doctorID)
{
    int dID = *(int *) &doctorID;

    while(true)
    {
        // Wait to begin appointment until the nurse brings in a patient
        sem_wait(&doctorBeginAppointment[dID]);
        int patientID = patientsInAppointment[dID];

        printf("Doctor %i listens to symptoms from patient %i\n", dID, patientID);
        sem_post(&listenSymptoms[dID]);

        sem_wait(&listenAdvice[dID]);

        // When the patient leaves, signal the nurse that the appointment is over and a new patient may be seen
        sem_post(&doctorEndAppointment[dID]);
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

    patientsAwaitingRegistration = queue<int>();
    patientsAwaitingDoctor = new queue<int>[doctorCount];
    patientsInAppointment = new int[doctorCount];

    sem_init(&awaitReceptionist, 1, 0);

    registerPatient = new sem_t[patientCount];
    for(int i = 0; i < patientCount; i++)
    {
        sem_init(&registerPatient[i], 1, 0);
    }

    receptionDeskDeparture = new sem_t[patientCount];
    for(int i = 0; i < patientCount; i++)
    {
        sem_init(&receptionDeskDeparture[i], 1, 0);
    }

    awaitNurse = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&awaitNurse[i], 1, 0);
    }

    waitingRoomDeparture = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&waitingRoomDeparture[i], 1, 0);
    }

    doctorOfficeArrival = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&doctorOfficeArrival[i], 1, 0);
    }

    doctorBeginAppointment = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&doctorBeginAppointment[i], 1, 0);
    }

    listenSymptoms = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&listenSymptoms[i], 1, 0);
    }

    listenAdvice = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&listenAdvice[i], 1, 0);
    }

    doctorEndAppointment = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&doctorEndAppointment[i], 1, 0);
    }
    
    sem_init(&adjustExitedPatients, 1, 1);

    printf("Run with %i patients, %i nurses, %i doctors\n\n", patientCount, doctorCount, doctorCount);

    patients = new pthread_t[patientCount];
    nurses = new pthread_t[doctorCount];
    doctors = new pthread_t[doctorCount];
    int threadError;

    setbuf(stdout, NULL);

    // Create patient threads
    for(int i = 0; i < patientCount; i++)
    {
        threadError = pthread_create(&patients[i], NULL, patient, (void *) i);

        if(threadError)
        {
            cout << "Error: Unable to create patient thread with id " + to_string(i) + ".";
            exit(-1);
        }
    }

    // Create receptionist thread
    threadError = pthread_create(&reception, NULL, receptionist, (void *) 0);

    if(threadError)
    {
        cout << "Error: Unable to create receptionist thread.";
        exit(-1);
    }

    // Create doctor and nurse threads
    for(int i = 0; i < doctorCount; i++)
    {
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
    
    pthread_exit(NULL);
    return 0;
}