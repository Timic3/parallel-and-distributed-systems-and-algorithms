#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeImage.h"
#include <math.h>
#include <CL/cl.h>
#include <omp.h>

#define BINS 256
#define MAX_SOURCE_SIZE 16384
#define GPU

struct histogram {
    unsigned int *R;
    unsigned int *G;
    unsigned int *B;
};

void histogramCPU(unsigned char *imageIn, histogram H, int width, int height) {
    // Each color channel is 1 byte long, there are 4 channels BLUE, GREEN, RED and ALPHA
    // The order is BLUE|GREEN|RED|ALPHA for each pixel, we ignore the ALPHA channel when computing the histograms
    for (int i = 0; i < (height); i++) {
        for (int j = 0; j < (width); j++) {
            H.R[imageIn[(i * width + j) * 4 + 2]]++;
            H.G[imageIn[(i * width + j) * 4 + 1]]++;
            H.B[imageIn[(i * width + j) * 4]]++;
        }
    }
}

void printHistogram(histogram H) {
    printf("Colour\tNo. Pixels\n");
    for (int i = 0; i < BINS; i++) {
        if (H.B[i]>0)
            printf("%dB\t%d\n", i, H.B[i]);
        if (H.G[i]>0)
            printf("%dG\t%d\n", i, H.G[i]);
        if (H.R[i]>0)
            printf("%dR\t%d\n", i, H.R[i]);
    }
}

void histogramGPU(unsigned char *image_in, histogram H, int width, int height) {
    int imageSize = height * width * sizeof(unsigned char) * 4;
    cl_int ret;

    FILE *fp = fopen("img_hist.cl", "r");
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
    size_t local_item_size = BINS;
    size_t num_groups = (((width * height) - 1) / local_item_size + 1);
    size_t global_item_size = num_groups * local_item_size;

    // Alokacija pomnilnika na napravi
    cl_mem image_mem_obj_in = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      imageSize, image_in, &ret);
    cl_mem image_hist_obj_r_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                      256 * sizeof(unsigned int), NULL, &ret);
    cl_mem image_hist_obj_g_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                      256 * sizeof(unsigned int), NULL, &ret);
    cl_mem image_hist_obj_b_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                      256 * sizeof(unsigned int), NULL, &ret);
  
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
    printf("%s\n", build_log);
    free(build_log);

    // scepec: priprava objekta
    cl_kernel kernel = clCreateKernel(program, "histogram", &ret);
            // program, ime scepca, napaka

    double start, end;
    start = omp_get_wtime();
    // scepec: argumenti
    ret |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &image_mem_obj_in);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &image_hist_obj_r_out);
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &image_hist_obj_g_out);
    ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *) &image_hist_obj_b_out);
    ret |= clSetKernelArg(kernel, 4, sizeof(cl_int), (void *) &width);
    ret |= clSetKernelArg(kernel, 5, sizeof(cl_int), (void *) &height);
            // scepec, stevilka argumenta, velikost podatkov, kazalec na podatke

    // scepec: zagon
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
            // vrsta, scepec, dimenzionalnost, mora biti NULL,
            // kazalec na stevilo vseh niti, kazalec na lokalno stevilo niti, 
            // dogodki, ki se morajo zgoditi pred klicem

    // Kopiranje rezultatov
    ret = clEnqueueReadBuffer(command_queue, image_hist_obj_r_out, CL_TRUE, 0, 256 * sizeof(unsigned int), H.R, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, image_hist_obj_g_out, CL_TRUE, 0, 256 * sizeof(unsigned int), H.G, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, image_hist_obj_b_out, CL_TRUE, 0, 256 * sizeof(unsigned int), H.B, 0, NULL, NULL);
            // branje v pomnilnik iz naprave, 0 = offset
            // zadnji trije - dogodki, ki se morajo zgoditi prej

    end = omp_get_wtime();
    printf("Velikost slike %dx%d, GPU (paralelni) cas: %.6fs\n", width, height, end - start);

    // Ciscenje
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(image_mem_obj_in);
    ret = clReleaseMemObject(image_hist_obj_r_out);
    ret = clReleaseMemObject(image_hist_obj_g_out);
    ret = clReleaseMemObject(image_hist_obj_b_out);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
}

int main(void) {
    // Load image from file
    FIBITMAP *imageBitmap = FreeImage_Load(FIF_PNG, "15360x8640.png", 0);
    // Convert it to a 32-bit image
    FIBITMAP *imageBitmap32 = FreeImage_ConvertTo32Bits(imageBitmap);

    // Get image dimensions
    int width = FreeImage_GetWidth(imageBitmap32);
    int height = FreeImage_GetHeight(imageBitmap32);
    int pitch = FreeImage_GetPitch(imageBitmap32);
    // Preapare room for a raw data copy of the image
    unsigned char *imageIn = (unsigned char *)malloc(height * pitch * sizeof(unsigned char));

    // Initalize the histogram
    histogram H;
    H.R = (unsigned int*)calloc(BINS, sizeof(unsigned int));
    H.G = (unsigned int*)calloc(BINS, sizeof(unsigned int));
    H.B = (unsigned int*)calloc(BINS, sizeof(unsigned int));

    // Extract raw data from the image
    FreeImage_ConvertToRawBits(imageIn, imageBitmap, pitch, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

    // Free source image data
    FreeImage_Unload(imageBitmap32);
    FreeImage_Unload(imageBitmap);

    // Compute and print the histogram
    #ifndef GPU
        double start, end;
        start = omp_get_wtime();

        histogramCPU(imageIn, H, width, height);

        end = omp_get_wtime();
        printf("Velikost slike %dx%d, CPU (sekvencni) cas: %.6fs\n", width, height, end - start);
    #else
        histogramGPU(imageIn, H, width, height);
    #endif
    // printHistogram(H);

    return 0;
}

/*
Velikost slike 640x480, GPU (paralelni) cas: 0.000846s, CPU (sekvencni) cas: 0.000671s, pohitritev: 0.79x
Velikost slike 800x600, GPU (paralelni) cas: 0.000976s, CPU (sekvencni) cas: 0.000972s, pohitritev: 0.996x
Velikost slike 1600x900, GPU (paralelni) cas: 0.001875s, CPU (sekvencni) cas: 0.002969s, pohitritev: 1.58x
Velikost slike 1920x1080, GPU (paralelni) cas: 0.002462s, CPU (sekvencni) cas: 0.004221s, pohitritev: 1.71x
Velikost slike 3840x2160, GPU (paralelni) cas: 0.010403s, CPU (sekvencni) cas: 0.018096s, pohitritev: 1.74x
Velikost slike 15360x8640, GPU (paralelni) cas: 0.191598s, CPU (sekvencni) cas: 0.294050s, pohitritev: 1.53x
*/
