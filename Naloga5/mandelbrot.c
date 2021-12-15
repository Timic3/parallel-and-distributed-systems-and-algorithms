#include <stdio.h>
#include <stdlib.h>
#include "FreeImage.h"
#include <math.h>
#include <CL/cl.h>
#include <omp.h>

// srun -n1 -G1 --reservation=fri ./mandelbrot
// gcc mandelbrot.c -O2 -lm -lOpenCL -fopenmp -Wl,-rpath,./ -L./ -l:"libfreeimage.so.3" -o mandelbrot

#define WORKGROUP_SIZE 512
#define MAX_SOURCE_SIZE 16384
#define GPU


void mandelbrotCPU(unsigned char *image, int width, int height) {
    float x0, y0, x, y, xtemp;
    int i, j;
    int color;
    int iter;
    int max_iteration = 800; // Max stevilo iteracij
    unsigned char max = 255; // Max vrednost barvnega kanala

    // Za vsak piksel v sliki
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            x0 = (float) j / width * (float) 3.5 - (float) 2.5; // Zacetna vrednost
            y0 = (float) i / height * (float) 2.0 - (float) 1.0;
            x = 0;
            y = 0;
            iter = 0;
            // Ponavljamo, dokler ne izpolnemo enega izmed pogojev
            while ((x * x + y * y <= 4) && (iter < max_iteration)) {
                xtemp = x * x - y * y + x0;
                y = 2 * x * y + y0;
                x = xtemp;
                iter++;
            }
            // Izracunamo barvo (magic: http://linas.org/art-gallery/escape/smooth.html)
            color = 1.0 + iter - log(log(sqrt(x*x + y * y))) / log(2.0);
            color = (8 * max * color) / max_iteration;
            if (color > max)
                color = max;
            // Zapisemo barvo RGBA (v resnici little endian BGRA)
            image[4 * i*width + 4 * j + 0] = color; // Blue
            image[4 * i*width + 4 * j + 1] = color; // Green
            image[4 * i*width + 4 * j + 2] = 0; // Red
            image[4 * i*width + 4 * j + 3] = 255; // Alpha
        }
    }
}

void mandelbrotGPU(unsigned char *image, int width, int height) {
    int imageSize = height * width * sizeof(unsigned char) * 4;
    cl_int ret;

    FILE *fp = fopen("mandelbrot.cl", "r");
    if (!fp) {
        fprintf(stderr, "Could not open kernel.\n");
        exit(1);
    }

    // Zapisi Kernel v RAM
    char *source_str = (char*) malloc(MAX_SOURCE_SIZE);
    size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    source_str[source_size] = '\0';
    fclose(fp);

    // Podatki o platformi
    cl_platform_id platform_id[10];
    cl_uint ret_num_platforms;
    ret = clGetPlatformIDs(10, platform_id, &ret_num_platforms); // Max. stevilo platform, kazalec na platforme, dejansko stevilo platform

    // Podatki o napravi
    cl_device_id device_id[10];
    cl_uint ret_num_devices;
    // Delali bomo s platform_id[0] na GPU
    ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 10, device_id, &ret_num_devices);				
        // izbrana platforma, tip naprave, koliko naprav nas zanima
        // kazalec na naprave, dejansko "stevilo naprav

    // Kontekst
    cl_context context = clCreateContext(NULL, 1, &device_id[0], NULL, NULL, &ret);
        // kontekst: vkljucene platforme - NULL je privzeta, stevilo naprav, 
        // kazalci na naprave, kazalec na call-back funkcijo v primeru napake
        // dodatni parametri funkcije, stevilka napake
 
    // Ukazna vrsta
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);
        // kontekst, naprava, INORDER/OUTOFORDER, napake

    // Delitev dela
    size_t local_item_size = WORKGROUP_SIZE;
    size_t num_groups = (((width * height) - 1) / local_item_size + 1);
    size_t global_item_size = num_groups * local_item_size;

    // Alokacija pomnilnika na napravi
    cl_mem image_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                      imageSize, NULL, &ret);
  
    // Priprava programa
    cl_program program = clCreateProgramWithSource(context, 1, (const char **) &source_str, NULL, &ret);
            // kontekst, stevilo kazalcev na kodo, kazalci na kodo,
            // stringi so NULL terminated, napaka
 
    // Prevajanje
    ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);
            // program, stevilo naprav, lista naprav, opcije pri prevajanju,
            // kazalec na funkcijo, uporabniski argumenti

    // Log
    size_t build_log_len;
    char *build_log;
    ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);
            // program, naprava, tip izpisa, 
            // maksimalna dolzina niza, kazalec na niz, dejanska dolzina niza
    build_log = (char *) malloc(sizeof(char) * (build_log_len + 1));
    ret = clGetProgramBuildInfo(program, device_id[0], CL_PROGRAM_BUILD_LOG, build_log_len, build_log, NULL);
    // printf("%s\n", build_log);
    free(build_log);

    // scepec: priprava objekta
    cl_kernel kernel = clCreateKernel(program, "mandelbrot", &ret);
            // program, ime scepca, napaka

    double start, end;
    start = omp_get_wtime();
    // scepec: argumenti
    ret |= clSetKernelArg(kernel, 0, sizeof(cl_int), (void *) &width);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_int), (void *) &height);
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &image_mem_obj);
            // scepec, stevilka argumenta, velikost podatkov, kazalec na podatke

    // scepec: zagon
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
            // vrsta, scepec, dimenzionalnost, mora biti NULL,
            // kazalec na stevilo vseh niti, kazalec na lokalno stevilo niti, 
            // dogodki, ki se morajo zgoditi pred klicem

    // Kopiranje rezultatov
    ret = clEnqueueReadBuffer(command_queue, image_mem_obj, CL_TRUE, 0, imageSize, image, 0, NULL, NULL);
            // branje v pomnilnik iz naprave, 0 = offset
            // zadnji trije - dogodki, ki se morajo zgoditi prej

    end = omp_get_wtime();
    printf("Velikost slike %dx%d, GPU (paralelni) cas: %.6fs\n", width, height, end - start);

    // Ciscenje
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(image_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
}

int main() {
    int width = 3840;
    int height = 2160;

    // Rezerviramo prostor za sliko (RGBA)
    unsigned char *image = (unsigned char *) malloc(height * width * sizeof(unsigned char) * 4);

    #ifndef GPU
        double start, end;
        start = omp_get_wtime();

        mandelbrotCPU(image, width, height);

        end = omp_get_wtime();
        printf("Velikost slike %dx%d, CPU (sekvencni) cas: %.6fs\n", width, height, end - start);
    #else
        mandelbrotGPU(image, width, height);
    #endif

    // Shranimo sliko
    FIBITMAP *dst = FreeImage_ConvertFromRawBits(image, width, height, ((32 * width + 31) / 32) * 4,
        32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
    FreeImage_Save(FIF_PNG, dst, "mandelbrot.png", 0);
    free(image);
    return 0;
}

/*
Velikost slike 640x480, GPU (paralelni) cas: 0.001746s, CPU (sekvencni) cas: 0.284533s, pohitritev: 163x
Velikost slike 800x600, GPU (paralelni) cas: 0.002529s, CPU (sekvencni) cas: 0.444202s, pohitritev: 176x
Velikost slike 1600x900, GPU (paralelni) cas: 0.005622s, CPU (sekvencni) cas: 1.332188s, pohitritev: 237x
Velikost slike 1920x1080, GPU (paralelni) cas: 0.007672s, CPU (sekvencni) cas: 1.916104s, pohitritev: 250x
Velikost slike 3840x2160, GPU (paralelni) cas: 0.023260s, CPU (sekvencni) cas: 7.647959s, pohitritev: 329x
Velikost slike 15360x8640, GPU (paralelni) cas: 0.344610s, CPU (sekvencni) cas: 122.220239s, pohitritev: 354x
*/
