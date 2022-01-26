#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct Player {
    int playerNo;
    char mark;
};

pthread_mutex_t mutex;

// Global, shared variables that will be accessed in threads
int N;
char** matrix;
char winner = ' ';
int turn = 1;

/* Returns 1 if the game has reached a winning state, 0 if it has not. */
int winningState(char mark, int row, int col) {
    for (int i = 0; i < N; i++) {
        if (matrix[i][col] != mark) {
            break;
        }

        if (i == N - 1) {
            return 1;
        }
    }

    for (int i = 0; i < N; i++) {
        if (matrix[row][i] != mark) {
            break;
        }

        if (i == N - 1) {
            return 1;
        }
    }

    if (row == col) {
        for (int i = 0; i < N; i++) {
            if (matrix[i][i] != mark) {
                break;
            }

            if (i == N - 1) {
                return 1;
            }
        }
    }

    if (row + col == N - 1) {
        for (int i = 0; i < N; i++) {
            if (matrix[N - 1 - i][i] != mark) {
                break;
            }

            if (i == N - 1) {
                return 1;
            }
        }
    }

    return 0;
}

void* thread_func(void* arg) {
    struct Player* args = (struct Player*) arg;
    
    while (1) {
        pthread_mutex_lock(&mutex);

        if (winner != ' ' || turn >= N * N + 1) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        if (turn % 2 == args->playerNo % 2) {
            int row, col;

            do {
                row = rand() % N;
                col = rand() % N;
            } while(matrix[row][col] != ' ');

            matrix[row][col] = args->mark;
            turn++;
            printf("Player %c played on: (%d,%d)\n", args->mark, row, col);

            if (winningState(args->mark, row, col) == 1) {
                winner = args->mark;
            }

            pthread_mutex_unlock(&mutex);
        }
        
        else {
            pthread_mutex_unlock(&mutex);
        }
    }
}

int main(int argc, char* argv[]) {

    // Ensuring generation of different random numbers at each execution.
    clock_t start = clock();
    srand(time(NULL));

    if (argc == 1 || argc > 2) {
        printf("Error: Incorrect number of arguments.\n");
        printf("Please enter a positive integer N as an argument for the NxN matrix that will be generated for the Tic-Tac-Toe game.\n");
        exit(1);
    }

    // N will be entered as an argument from the command line
    N = atoi(argv[1]);

    if (N <= 0) {
        printf("Error: Passed argument is not a positive integer.\n");
        printf("Please enter a positive integer N as an argument for the NxN matrix that will be generated for the Tic-Tac-Toe game.\n");
        exit(1);
    }

    printf("Board Size: %dx%d\n", N, N);
    // NxN matrix for the game, all cells initialized as empty space initially.
    matrix = malloc(N * sizeof(char*));

    for (int i = 0; i < N; i++) {
        matrix[i] = malloc(N * sizeof(char));
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = ' ';
        }
    }

    // Main thread initializes the lock.
    pthread_mutex_init(&mutex, NULL);

    pthread_t thread1, thread2;
    
    struct Player p1 = {1, 'x'};
    struct Player p2 = {2, 'o'};

    // If at least one creation fails, the main thread terminates
    if (pthread_create(&thread1, NULL, thread_func, &p1) != 0) {
        printf("Error: thread1 creation failed.\n");
        exit(1);
    }

    if (pthread_create(&thread2, NULL, thread_func, &p2) != 0) {
        printf("Error: thread2 creation failed.\n");
        exit(1);       
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Game end\n");
    
    if (winner != ' ') {
        printf("Winner is %c\n", winner - 32);
    }
    
    else {
        printf("It is a tie\n");
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("[%c]", matrix[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < N; i++) {
        free(matrix[i]);
    }
 
    free(matrix);

    return 0;
}