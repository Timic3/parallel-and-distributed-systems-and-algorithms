#include <stdio.h>
#include <time.h>
#include <stdlib.h>

double *Random(int n) {
    srand(time(NULL));

    double *V = (double *) malloc(n * sizeof(double));
    for (int i = 0; i < n; ++i) {
        V[i] = (double) rand() / (double) RAND_MAX;
    }

    return V;
}

double **Matrix(double *A, int n, int r, int c) {
    double **B = (double **) calloc(r, sizeof(double *));
    for (int i = 0; i < r; ++i) {
        B[i] = (double *) calloc(c, sizeof(double));
    }

    int k = 0;
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            B[i][j] = A[k++];
            if (k >= n) return B;
        }
    }

    return B;
}

double *Max(double *A, int n) {
    double *max = &A[0];
    for (int i = 1; i < n; ++i) {
        if (A[i] > *max) max = &A[i];
    }
    return max;
}

void printVector(double *A, int n) {
    printf("1D:\n");
    for (int i = 0; i < n; ++i) {
        printf("%.2f ", A[i]);
    }
    printf("\n");
}

void printMatrix(double **M, int r, int c) {
    printf("2D:\n");
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            printf("%.2f ", M[i][j]);
        }
        printf("\n");
    }
}

void printMax(double *A) {
    printf("Najvecja vrednost: %.2f na naslovu: %p\n", *A, A);
}

int main() {
    int n;
    int rows;
    int cols;
    printf("Vnesi n: ");
    fflush(stdout);
    scanf("%d", &n);
    printf("\nVnesi r: ");
    fflush(stdout);
    scanf("%d", &rows);

    if (rows < 1 || n < 1) {
        printf("Stevilo vrstic (r) in velikost vektorja (n) morata biti nad 0.\n");
        return 1;
    }

    cols = (n + rows - 1) / rows;

    double *V = Random(n);
    printVector(V, n);

    double **M = Matrix(V, n, rows, cols);
    printMatrix(M, rows, cols);
    printf("\n");

    double *MN = Max(V, n);
    printMax(MN);

    for (int i = 0; i < rows; ++i) {
        free(M[i]);
    }
    free(M);
    free(V);

    return 0;
}
