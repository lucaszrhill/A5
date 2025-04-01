#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define THREAD_COUNT 5
#define RESOURCE_TYPES 4

// Global variables
int Available[RESOURCE_TYPES];
int Max[THREAD_COUNT][RESOURCE_TYPES];
int Allocation[THREAD_COUNT][RESOURCE_TYPES] = {0};
int Need[THREAD_COUNT][RESOURCE_TYPES];

// Function prototypes
void readInput(const char *filename);
void displayMaximumResources();
void displayStatus();
bool isSafeState(int safeSequence[]);
void requestResources(int customerID, int request[]);
void runSafeSequence();

// Function to read input from file
void readInput(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[100];
    for (int t = 0; t < THREAD_COUNT; t++) {
        if (fgets(line, sizeof(line), file) != NULL) {
            sscanf(line, "%d,%d,%d,%d", &Max[t][0], &Max[t][1], &Max[t][2], &Max[t][3]);
        } else {
            printf("Error reading line %d from file.\n", t);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);

    // Initialize Need matrix
    for (int t = 0; t < THREAD_COUNT; t++) {
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            Need[t][r] = Max[t][r] - Allocation[t][r];
        }
    }
}

// Function to display Maximum Resources
void displayMaximumResources() {
    printf("Maximum Resources from file:\n");
    for (int t = 0; t < THREAD_COUNT; t++) {
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            printf("%d ", Max[t][r]);
        }
        printf("\n");
    }
}

// Function to display current system status
void displayStatus() {
    printf("Available Resources:\n");
    for (int i = 0; i < RESOURCE_TYPES; i++) {
        printf("%d ", Available[i]);
    }
    printf("\n");

    printf("Allocated Resources:\n");
    for (int t = 0; t < THREAD_COUNT; t++) {
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            printf("%d ", Allocation[t][r]);
        }
        printf("\n");
    }

    printf("Need Resources:\n");
    for (int t = 0; t < THREAD_COUNT; t++) {
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            printf("%d ", Need[t][r]);
        }
        printf("\n");
    }
}

// Function to check if system is in a safe state
bool isSafeState(int safeSequence[]) {
    int Work[RESOURCE_TYPES];
    bool Finish[THREAD_COUNT] = {false};

    for (int i = 0; i < RESOURCE_TYPES; i++) {
        Work[i] = Available[i];
    }

    int count = 0;
    while (count < THREAD_COUNT) {
        bool found = false;

        for (int t = 0; t < THREAD_COUNT; t++) {
            if (!Finish[t]) {
                bool canAllocate = true;

                for (int i = 0; i < RESOURCE_TYPES; i++) {
                    if (Need[t][i] > Work[i]) {
                        canAllocate = false;
                        break;
                    }
                }

                if (canAllocate) {
                    for (int i = 0; i < RESOURCE_TYPES; i++) {
                        Work[i] += Allocation[t][i];
                    }
                    Finish[t] = true;
                    safeSequence[count++] = t;
                    found = true;
                }
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

// Function to request resources
void requestResources(int customerID, int request[]) {
    printf("Customer/Thread %d is requesting resources...\n", customerID);

    // Validate request
    for (int i = 0; i < RESOURCE_TYPES; i++) {
        if (request[i] > Need[customerID][i] || request[i] > Available[i]) {
            printf("Request exceeds Need or Available resources. Request denied.\n");
            return;
        }
    }

    // Temporarily allocate resources
    for (int i = 0; i < RESOURCE_TYPES; i++) {
        Available[i] -= request[i];
        Allocation[customerID][i] += request[i];
        Need[customerID][i] -= request[i];
    }

    // Check if the system is still in a safe state
    int safeSequence[THREAD_COUNT];
    if (isSafeState(safeSequence)) {
        printf("State is safe, and request is satisfied.\n");
    } else {
        printf("State is unsafe. Rolling back allocation...\n");
        for (int i = 0; i < RESOURCE_TYPES; i++) {
            Available[i] += request[i];
            Allocation[customerID][i] -= request[i];
            Need[customerID][i] += request[i];
        }
    }
}

// Function to execute threads in a safe sequence
void runSafeSequence() {
    int safeSequence[THREAD_COUNT];
    if (!isSafeState(safeSequence)) {
        printf("No safe sequence exists. System is in an unsafe state.\n");
        return;
    }

    printf("Safe Sequence is: ");
    for (int i = 0; i < THREAD_COUNT; i++) {
        printf("%d ", safeSequence[i]);
    }
    printf("\n");

    for (int i = 0; i < THREAD_COUNT; i++) {
        int customerID = safeSequence[i];
        printf("--> Customer/Thread %d\n", customerID);
        printf("Allocated resources: ");
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            printf("%d ", Allocation[customerID][r]);
        }
        printf("\nNeeded: ");
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            printf("%d ", Need[customerID][r]);
        }
        printf("\nAvailable: ");
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            printf("%d ", Available[r]);
        }
        printf("\nThread has started\nThread has finished\nThread is releasing resources\n");

        for (int r = 0; r < RESOURCE_TYPES; r++) {
            Available[r] += Allocation[customerID][r];
            Allocation[customerID][r] = 0;
        }

        printf("New Available: ");
        for (int r = 0; r < RESOURCE_TYPES; r++) {
            printf("%d ", Available[r]);
        }
        printf("\n");
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != RESOURCE_TYPES + 1) {
        printf("Usage: ./banker <resource1> <resource2> <resource3> <resource4>\n");
        return 1;
    }

    // Initialize Available resources
    for (int i = 0; i < RESOURCE_TYPES; i++) {
        Available[i] = atoi(argv[i + 1]);
    }

    printf("Number of Customers: %d\n", THREAD_COUNT);
    printf("Current Available resources: ");
    for (int i = 0; i < RESOURCE_TYPES; i++) {
        printf("%d ", Available[i]);
    }
    printf("\n");

    // Read Maximum Resources from file
    readInput("sample_in_banker.txt");

    // Display Maximum Resources
    displayMaximumResources();

    // Command loop
    char command[20];
    while (true) {
        printf("Enter Command: ");
        scanf("%s", command);

        if (strcmp(command, "RQ") == 0) {
            int customerID;
            int request[RESOURCE_TYPES];
            scanf("%d", &customerID);
            for (int i = 0; i < RESOURCE_TYPES; i++) {
                scanf("%d", &request[i]);
            }
            requestResources(customerID, request);
        } else if (strcmp(command, "Run") == 0) {
            runSafeSequence();
        } else if (strcmp(command, "Status") == 0) {
            displayStatus();
        } else if (strcmp(command, "Exit") == 0) {
            break;
        } else {
            printf("Invalid command. Try again.\n");
        }
    }

    return 0;
}
