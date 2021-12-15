#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <unistd.h>

#define N 100000 // Velikost tabele
#define T 8 // Stevilo niti (mora biti manjse ali enako N / 2)
#define OPTIMIZATION // Optimizacija (koncaj program, ko je sortirano)
// #define PRINT_NUMBERS // Izpisi stevilke na zacetku in koncu

#define RANDOM_MAX 1000000 // Random do stevilke
#define RANDOM_MIN 0 // Random od stevilke

// salloc -n1 --cpus-per-task=8 --reservation=fri
// gcc -O2 -lpthread -o bubble bubble.cpp

typedef struct {
    pthread_t thread;
    int id;
    int start;
    int pairs;
} thread;

pthread_barrier_t barrier;
#ifdef OPTIMIZATION
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

int numbers[N];
thread threads[T];

#ifdef OPTIMIZATION
    int thread_sorts = 0;
#endif

void *sort(void *params);
bool is_sorted();

int main(int argc, char **argv) {
    srand(time(NULL));
    pthread_barrier_init(&barrier, NULL, T);

    for (int i = 0; i < N; ++i) {
        numbers[i] = rand() % (RANDOM_MAX + 1 - RANDOM_MIN) + RANDOM_MIN;
    }

    printf("Sorting %d numbers with %d threads\n", N, T);
    #ifdef PRINT_NUMBERS
        for (int i = 0; i < N; ++i) {
            printf("%d ", numbers[i]);
        }
        printf("\n---------------------------------------------------------------------\n");
    #endif

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int n = 0;

    for (int i = 0; i < T; ++i) {
        threads[i].id = i;
        threads[i].start = n;
        threads[i].pairs = (i + 1) * (N / 2) / T - i * (N / 2) / T;
        n = n + threads[i].pairs * 2;
        pthread_create(&threads[i].thread, NULL, &sort, (thread *) &threads[i]);
    }

    for (int i = 0; i < T; ++i) {
        pthread_join(threads[i].thread, NULL);
    }

    gettimeofday(&end, NULL);

    #ifdef PRINT_NUMBERS
        for (int i = 0; i < N; ++i) {
            printf("%d ", numbers[i]);
        }
        printf("\n");
    #endif

    if (is_sorted()) {
        printf("Stevilke so urejene pravilno! :)");
    } else {
        printf("Stevilke niso urejene pravilno! :(");
    }
    printf("\n");

    // double = 8 bajtov, long long = 8 bajtov
    // Ce bi uporabili long, bi se pri 2000 sekundah zgodil izliv/overflow
    double elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    #ifdef OPTIMIZATION
        printf("Cas za sortiranje: %.6f sekund z optimizacijo", elapsed / 1000000.0);
    #else
        printf("Cas za sortiranje: %.6f sekund brez optimizacije", elapsed / 1000000.0);
    #endif
    printf("\n");

    return 0;
}

void *sort(void *params) {
    thread *t = (thread*) params;

    #ifdef OPTIMIZATION
        short consecutive_sorts = 0;
    #endif
    for (int i = 0; i < N; ++i) {
        #ifdef OPTIMIZATION
            bool failed_sort = false;
        #endif
        int start = t->start + (i % 2);
        for (int j = 0; j < t->pairs; ++j) {
            int index = start + j * 2;
            if (index + 1 >= N) break;
            if (numbers[index] > numbers[index + 1]) {
                int tmp = numbers[index];
                numbers[index] = numbers[index + 1];
                numbers[index + 1] = tmp;

                #ifdef OPTIMIZATION
                    failed_sort = true;
                #endif
            }
        }

        #ifdef OPTIMIZATION
            if (!failed_sort) {
                ++consecutive_sorts;
            } else {
                consecutive_sorts = 0;
            }

            if (consecutive_sorts >= 2) {
                consecutive_sorts = 2; // Prepreci overflow
                pthread_mutex_lock(&mutex);
                ++thread_sorts;
                pthread_mutex_unlock(&mutex);
            }
        #endif
        pthread_barrier_wait(&barrier);
        #ifdef OPTIMIZATION
            if (thread_sorts == T) {
                return 0;
            }
            thread_sorts = 0;
        #endif
    }

    return 0;
}

bool is_sorted() {
    int prev = numbers[0];
    for (int i = 1; i < N; ++i) {
        if (prev > numbers[i]) return false;
        prev = numbers[i];
    }
    return true;
}
