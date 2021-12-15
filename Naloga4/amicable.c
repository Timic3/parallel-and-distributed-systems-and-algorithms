#include <stdio.h>
#include <omp.h>
#include <math.h>

// salloc -n1 --cpus-per-task=8 --reservation=fri
// gcc -O2 -lm -fopenmp -o amicable amicable.c

#define N 100000
#define T 8
#define MULTITHREADED
// #define TERNARY_OP_TEST
// #define PRINT_NUMBERS

int A[N];

int sum_divisors(int n);

int main() {
    omp_set_num_threads(T);
    int sum = 0;

    double start, end;
    start = omp_get_wtime();

    #ifdef MULTITHREADED
        // schedule(static, x): x iteracij dobi vsak thread (po intervalih), do konca
        // schedule(dynamic, x): ko se nit sprosti, ji doda (x) paket(ov) (overhead za gledanje sprostitve)
        // schedule(guided, x): podobno kot dynamic, na zacetku dodeljuje vecje bloke, potem se pa zmanjsuje (min: x)
        // schedule(auto): prevajalnik odloca kaj je najboljse
        // #pragma omp parallel for schedule(static, 10000)
        // #pragma omp parallel for schedule(dynamic, 10000)
        // #pragma omp parallel for schedule(guided, 10000)
        #pragma omp parallel for schedule(auto)
    #endif
    for (int i = 1; i < N; ++i) {
        A[i] = sum_divisors(i);
    }

    #ifdef PRINT_NUMBERS
        printf("Prijateljska stevila:\n");
    #endif

    #ifdef MULTITHREADED
        // Ne rabimo schedule, ker je zahtevnost enakovredna
        // #pragma omp parallel for
        #pragma omp parallel for reduction(+:sum)
    #endif
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < i; ++j) {
            if (A[i] == j && A[j] == i) {
                #ifdef PRINT_NUMBERS
                    printf("%d in %d\n", i, j);
                #endif
                sum += (i + j);
            }
        }
    }

    end = omp_get_wtime();

    printf("Vsota prijateljskih stevil: %d, cas: %.6f\n", sum, end - start);
}


int sum_divisors(int n) {
    int sum = 0;
    for (int i = 2; i <= sqrt(n); ++i) {
        if (n % i == 0) {
            #ifdef TERNARY_OP_TEST
                sum += (i + ((n / i == i) ? 0 : n / i));
            #else
                if ((n / i) == i) {
                    sum += i;
                } else {
                    sum += (i + n / i);
                }
            #endif
        }
    }
    return sum + 1;
}

/*
Vse mere so v sekundah

Pohitritev: S = Ts/Tp
Ts = 9.826342
T1 = 5.964270
T2 = 4.919870
T4 = 2.603705
T8 = 1.605733
T16 = 0.811345
T32 = 0.401806
----------
S1 = 1.647534 = 2.6x
S2 = 1.997276 = 2x
S4 = 3.773984 = 3.8x
S8 = 6.119536 = 6x
S16 = 12.111175 = 12x
S32 = 24.455438 = 24.4x

+--------------+----------+------------+--------------+-------------+---------------+------------+-------------+----------+
| T \ schedule |   auto   | static(10) | static(1000) | dynamic(10) | dynamic(1000) | guided(10) | guided(100) |   min    |
+--------------+----------+------------+--------------+-------------+---------------+------------+-------------+----------+
|            1 | 5.994705 |   5.987256 |     5.986128 |    7.913048 |      5.964270 |   7.898157 |    5.964707 | 5.964270 |
|            2 | 4.947977 |   4.919870 |     4.927482 |    5.880649 |      5.203501 |   6.094085 |    4.937628 | 4.919870 |
|            4 | 2.603705 |   2.947287 |     2.950106 |    3.426653 |      2.942352 |   3.430252 |    2.943666 | 2.603705 |
|            8 | 1.616746 |   1.608853 |     1.609237 |    1.850234 |      1.605733 |   1.865517 |    1.606433 | 1.605733 |
|           16 | 0.811345 |   0.876354 |     0.831294 |    0.965841 |      0.882535 |   0.962228 |    0.883430 | 0.811345 |
|           32 | 0.443368 |   0.407923 |     0.443935 |    0.489895 |      0.401806 |   0.491771 |    0.408765 | 0.401806 |
+--------------+----------+------------+--------------+-------------+---------------+------------+-------------+----------+

-------------------------------
- T = 1
-------------------------------
auto = 5.994705

static(10) = 5.987256
static(100) = 5.999372
static(1000) = 5.986128
static(10000) = 5.985882

dynamic(10) = 7.913048
dynamic(100) = 7.899952
dynamic(1000) = 5.964270
dynamic(10000) = 5.990862

guided(10) = 7.898157
guided(100) = 7.905409
guided(1000) = 5.964707
guided(10000) = 5.964043

-------------------------------
- T = 2
-------------------------------
auto = 4.947977

static(10) = 4.919870
static(100) = 4.923788
static(1000) = 4.927482
static(10000) = 4.931846

dynamic(10) = 5.880649
dynamic(100) = 5.881063
dynamic(1000) = 5.203501
dynamic(10000) = 5.102301

guided(10) = 6.094085
guided(100) = 5.927689
guided(1000) = 4.937628
guided(10000) = 5.074227

-------------------------------
- T = 4
-------------------------------
auto = 2.603705

static(10) = 2.947287
static(100) = 2.949936
static(1000) = 2.950106
static(10000) = 2.957815

dynamic(10) = 3.426653
dynamic(100) = 3.431503
dynamic(1000) = 2.942352
dynamic(10000) = 2.922168

guided(10) = 3.430252
guided(100) = 3.429836
guided(1000) = 2.943666
guided(10000) = 2.948343

-------------------------------
- T = 8
-------------------------------
auto = 1.616746

static(10) = 1.608853
static(100) = 1.606165
static(1000) = 1.609237
static(10000) = 1.620368

dynamic(10) = 1.850234
dynamic(100) = 1.845573
dynamic(1000) = 1.605733
dynamic(10000) = 1.439116

guided(10) = 1.865517
guided(100) = 1.849525
guided(1000) = 1.606433
guided(10000) = 1.617477

-------------------------------
- T = 16
-------------------------------
auto = 0.811345

static(10) = 0.876354
static(100) = 0.828866
static(1000) = 0.831294
static(10000) = 0.838482

dynamic(10) = 0.965841
dynamic(100) = 0.961070
dynamic(1000) = 0.882535
dynamic(10000) = 0.869905

guided(10) = 0.962228
guided(100) = 0.961515
guided(1000) = 0.883430
guided(10000) = 0.872565

-------------------------------
- T = 32
-------------------------------
auto = 0.443368

static(10) = 0.407923
static(100) = 0.437734
static(1000) = 0.443935
static(10000) = 0.410994

dynamic(10) = 0.489895
dynamic(100) = 0.488845
dynamic(1000) = 0.401806
dynamic(10000) = 0.454548

guided(10) = 0.491771
guided(100) = 0.487370
guided(1000) = 0.408765
guided(10000) = 0.459448

*/
