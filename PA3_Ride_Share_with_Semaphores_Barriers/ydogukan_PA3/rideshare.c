#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

struct Fan {
    char team;
    sem_t* sem_own_team;
    sem_t* sem_other_team;
    sem_t* sem_own_team_inner;
    sem_t* sem_other_team_inner;
};

sem_t mutex, sem_A, sem_B, sem_A2, sem_B2;
pthread_barrier_t barrier;

// Threads from different teams will use the same routine
void* fan_routine(void* args) {
    struct Fan* fan = (struct Fan*) args;
    sem_wait(&mutex); // Lock the mutex the access shared variables
    printf("Thread ID: %ld, Team: %c, I am looking for a car\n", pthread_self(), fan->team);
    sem_wait(&*(fan->sem_own_team)); // Use one of the resources, 4 resources available for each team at the start
    int waiting_own_team, waiting_other_team;
    sem_getvalue(&*(fan->sem_own_team), &waiting_own_team); // Get how many fans are waiting from the team of the fan
    sem_getvalue(&*(fan->sem_other_team), &waiting_other_team); // Get how many fans are waiting from the other team

    if (waiting_own_team <= 0) { // If there are more than or equal to 4 fans waiting from the team of the fan
        // Signal other waiting 3 threads to wake up from their deep slumber
        sem_post(&*(fan->sem_own_team_inner)); 
        sem_post(&*(fan->sem_own_team_inner));
        sem_post(&*(fan->sem_own_team_inner));

        // All 4 resources of the team had been used, free those resources so more fans from the team of the fan can ask for a ride
        sem_post(&*(fan->sem_own_team));
        sem_post(&*(fan->sem_own_team));
        sem_post(&*(fan->sem_own_team));
        sem_post(&*(fan->sem_own_team));

        printf("Thread ID: %ld, Team: %c, I have found a spot in a car\n", pthread_self(), fan->team);
        pthread_barrier_wait(&barrier); // Wait until all 4 fans are seated in the car
        printf("Thread ID: %ld, Team: %c, I am the captain and driving the car\n", pthread_self(), fan->team); // Declare as captain
        sem_post(&mutex); // Unlock the mutex
    }

    else if (waiting_own_team == 2 && waiting_other_team <= 2) { // If there are 2 fans from the team of the fan and 2 or more fans waiting from the other team
        // Signal other waiting 3 threads to wake up from their deep slumber
        sem_post(&*(fan->sem_own_team_inner));
        sem_post(&*(fan->sem_other_team_inner));
        sem_post(&*(fan->sem_other_team_inner));

        // Free 2 resources from each team so 2 more fans can ask for a ride share from each team
        sem_post(&*(fan->sem_own_team));
        sem_post(&*(fan->sem_own_team));
        sem_post(&*(fan->sem_other_team));
        sem_post(&*(fan->sem_other_team));

        printf("Thread ID: %ld, Team: %c, I have found a spot in a car\n", pthread_self(), fan->team);
        pthread_barrier_wait(&barrier); // Wait until all 4 fans are seated in the car
        printf("Thread ID: %ld, Team: %c, I am the captain and driving the car\n", pthread_self(), fan->team); // Declare as captain
        sem_post(&mutex); // Unlock the mutex
    }

    else { // No valid combination for car seating
        sem_post(&mutex); // Unlock the mutex
        sem_wait(&*(fan->sem_own_team_inner)); // Wait with a 'inner' sephamore (acts like an inner waiting queue)
        printf("Thread ID: %ld, Team: %c, I have found a spot in a car\n", pthread_self(), fan->team);
        pthread_barrier_wait(&barrier); // Wait until all 4 fans are seated in the car
    }
}

int main(int argc, char* argv[]) {
    // Requirement of both the command, and two other arguments
    if (argc != 3) {
        printf("The main terminates\n");
        exit(1);
    }

    int num_A = atoi(argv[1]), num_B = atoi(argv[2]);
    
    // Checking if the given numbers are even, and if the sum is divisible by 4 (also if they are negative)
    if (num_A % 2 != 0 || num_B % 2 != 0 || (num_A + num_B) % 4 != 0 || num_A < 0 || num_B < 0) {
        printf("The main terminates\n");
        exit(1);
    }

    // Making space for enough threads, initializing semaphores and the barrier
    pthread_t threads[num_A + num_B];
    sem_init(&mutex, 0, 1);
    sem_init(&sem_A, 0, 4);
    sem_init(&sem_B, 0, 4);
    sem_init(&sem_A2, 0, 0);
    sem_init(&sem_B2, 0, 0);
    pthread_barrier_init(&barrier, NULL, 4);

    // Arguments for threads, since only one fan routine will be used, there is a need to pass semaphores' addresses
    struct Fan fan_A = {'A', &sem_A, &sem_B, &sem_A2, &sem_B2};
    struct Fan fan_B = {'B', &sem_B, &sem_A, &sem_B2, &sem_A2};

    // Random thread initialization rather than sequential
    clock_t start = clock();
    srand(time(NULL));
    int temp_A = num_A, temp_B = num_B;

    for (int i = 0; i < num_A + num_B; i++) {
        if (temp_A != 0 && temp_B != 0) {
            int n = rand() % 2;
            
            if (n == 0) {
                if (pthread_create(&threads[i], NULL, &fan_routine, &fan_A)) {
                    printf("Error: thread_A creation failed.\n");
                }
                temp_A--;
            }

            else {
                if (pthread_create(&threads[i], NULL, &fan_routine, &fan_B)) {
                    printf("Error: thread_A creation failed.\n");
                }
                temp_B--;
            }
        }

        else if (temp_A == 0){
            if (pthread_create(&threads[i], NULL, &fan_routine, &fan_B)) {
                printf("Error: thread_A creation failed.\n");
            }
        }

        else {
            if (pthread_create(&threads[i], NULL, &fan_routine, &fan_A)) {
                printf("Error: thread_A creation failed.\n");
            }
        }
    }

    // Sequential thread initialization, A's first, then B's. Early context switches result in only groups of A, hence the random initialization.
    /*
    for (int i = 0; i < num_A + num_B; i++) {
        if (i < num_A) {
            if (pthread_create(&threads[i], NULL, &fan_routine, &fan_A)) {
                printf("Error: thread_A creation failed.\n");
            }
        }

        else {
            if (pthread_create(&threads[i], NULL, &fan_routine, &fan_B)) {
                printf("Error: thread_B creation failed.\n");
            }
        }
    }
    */

    for (int i = 0; i < num_A + num_B; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroying everything to free allocated space
    sem_destroy(&mutex);
    sem_destroy(&sem_A);
    sem_destroy(&sem_B);
    sem_destroy(&sem_A2);
    sem_destroy(&sem_B2);    
    pthread_barrier_destroy(&barrier);

    printf("The main terminates\n");

    return 0;
}