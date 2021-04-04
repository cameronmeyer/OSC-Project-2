// Author: Cameron Meyer (cdm180003)
// Project 2
// CS 4348.001


#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <queue>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <exception>

using namespace std;

int doctorCount = 0;
int patientCount = 0;
int exitedPatients = 0;
int* desiredDoctors;
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
queue<int>* patientsAwaitingDoctor;

void *patient(void *patientID)
{
    int pID = *(int *) &patientID;
    int doctorID = rand() % doctorCount;
    desiredDoctors[pID] = doctorID;

    // Enter room and signal that you'd like to speak with the receptionist
    printf("Patient %i enters waiting room, waits for receptionist\n", pID);
    patientsAwaitingRegistration.push(pID);
    sem_post(&awaitReceptionist);

    // Wait for receptionist to complete your registration, then leave the receptionist desk
    sem_wait(&registerPatient[pID]);
    printf("Patient %i leaves receptionist and sits in waiting room\n", pID);
    sem_post(&receptionDeskDeparture[pID]);

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
        int doctorID = desiredDoctors[patientID];
        patientsAwaitingRegistration.pop();

        // Signal the patient to leave the receptionist desk, then wait for them to walk away
        printf("Receptionist registers patient %i\n", patientID);
        sem_post(&registerPatient[patientID]);
        sem_wait(&receptionDeskDeparture[patientID]);

        // Signal the appropriate nurse that their patient is ready for their appointment
        patientsAwaitingDoctor[doctorID].push(patientID);
        sem_post(&awaitNurse[doctorID]);
    }

    pthread_exit(NULL);
}

void *nurse(void *nurseID)
{
    int nID = *(int *) &nurseID;
    
    while(true)
    {
        // Wait for a patient to want to see your doctor and retreive them from the queue
        sem_wait(&awaitNurse[nID]);
        int patientID = patientsAwaitingDoctor[nID].front();
        patientsAwaitingDoctor[nID].pop();

        // Take patient to doctor's office
        printf("Nurse %i takes patient %i to doctor's office\n", nID, patientID);
        sem_post(&waitingRoomDeparture[nID]);

        // Wait for patient to recognize they are now inside the doctor's office
        sem_wait(&doctorOfficeArrival[nID]);

        // Signal doctor that their patient is ready to be seen
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

        // Signal to the patient that they may now list off their symptoms
        printf("Doctor %i listens to symptoms from patient %i\n", dID, patientID);
        sem_post(&listenSymptoms[dID]);

        // Wait for the patient to listen to all your advice
        sem_wait(&listenAdvice[dID]);

        // When the patient leaves, signal the nurse that the appointment is over and a new patient may be seen
        sem_post(&doctorEndAppointment[dID]);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Check for correct number of arguments
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
            printf("Error: Arguments could not be converted to valid integers.\n");
            exit(-1);
        }
    }
    else
    {
        printf("Error: Invalid argument list. Expected 2 arguments, but found %i.\n", (argc-1));
        exit(-1);
    }

    // Check that the doctor count is valid
    if(doctorCount < 1 || doctorCount > 3)
    {
        printf("Error: Invalid number of doctors. Expected between 1-3 doctors, but found %i.\n", doctorCount);
        exit(-1);
    }

    // Check that the patient count is valid
    if(patientCount < 1 || patientCount > 30)
    {
        printf("Error: Invalid number of patients. Expected between 1-30 patients, but found %i.\n", patientCount);
        exit(-1);
    }

    // Set up data structures for threads
    patientsAwaitingRegistration = queue<int>();
    patientsAwaitingDoctor = new queue<int>[doctorCount];
    desiredDoctors = new int[patientCount];
    patientsInAppointment = new int[doctorCount];

    // Initialize all semaphores
    sem_init(&awaitReceptionist, 1, 0);

    // Semaphores indexed by patient
    registerPatient = new sem_t[patientCount];
    receptionDeskDeparture = new sem_t[patientCount];
    for(int i = 0; i < patientCount; i++)
    {
        sem_init(&registerPatient[i], 1, 0);
        sem_init(&receptionDeskDeparture[i], 1, 0);
    }

    // Semaphores indexed by doctor
    awaitNurse = new sem_t[doctorCount];
    waitingRoomDeparture = new sem_t[doctorCount];
    doctorOfficeArrival = new sem_t[doctorCount];
    doctorBeginAppointment = new sem_t[doctorCount];
    listenSymptoms = new sem_t[doctorCount];
    listenAdvice = new sem_t[doctorCount];
    doctorEndAppointment = new sem_t[doctorCount];
    for(int i = 0; i < doctorCount; i++)
    {
        sem_init(&awaitNurse[i], 1, 0);
        sem_init(&waitingRoomDeparture[i], 1, 0);
        sem_init(&doctorOfficeArrival[i], 1, 0);
        sem_init(&doctorBeginAppointment[i], 1, 0);
        sem_init(&listenSymptoms[i], 1, 0);
        sem_init(&listenAdvice[i], 1, 0);
        sem_init(&doctorEndAppointment[i], 1, 0);
    }
    
    sem_init(&adjustExitedPatients, 1, 1);

    // Print run info
    printf("Run with %i patients, %i nurses, %i doctors\n\n", patientCount, doctorCount, doctorCount);

    // Create thread containers
    patients = new pthread_t[patientCount];
    nurses = new pthread_t[doctorCount];
    doctors = new pthread_t[doctorCount];
    int threadError;

    // Reset stdout buffer to ensure proper output
    setbuf(stdout, NULL);

    // Create patient threads
    for(int i = 0; i < patientCount; i++)
    {
        threadError = pthread_create(&patients[i], NULL, patient, (void *) i);

        if(threadError)
        {
            printf("Error: Unable to create patient thread with id %i.\n", i);
            exit(-1);
        }
    }

    // Create receptionist thread
    threadError = pthread_create(&reception, NULL, receptionist, (void *) 0);

    if(threadError)
    {
        printf("Error: Unable to create receptionist thread.\n");
        exit(-1);
    }

    // Create doctor and nurse threads
    for(int i = 0; i < doctorCount; i++)
    {
        threadError = pthread_create(&nurses[i], NULL, nurse, (void *) i);

        if(threadError)
        {
            printf("Error: Unable to create nurse thread with id %i.\n", i);
            exit(-1);
        }

        threadError = pthread_create(&doctors[i], NULL, doctor, (void *) i);

        if(threadError)
        {
            printf("Error: Unable to create doctor thread with id %i.\n", i);
            exit(-1);
        }
    }
    
    pthread_exit(NULL);
    return 0;
}