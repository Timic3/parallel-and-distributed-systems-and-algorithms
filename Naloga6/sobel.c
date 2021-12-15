#include <stdio.h>
#include <stdlib.h>
#include "FreeImage.h"
#include <math.h>
#include <CL/cl.h>
#include <omp.h>

#define WORKGROUP_SIZE 16
#define MAX_SOURCE_SIZE 16384
#define GPU

// module load CUDA
// gcc sobel.c -O2 -lm -lOpenCL -fopenmp -Wl,-rpath,./ -L./ -l:"libfreeimage.so.3" -o sobel
// srun -n1 -G1 --reservation=fri ./sobel

int getPixel(unsigned char *image, int y, int x, int width, int height) {
    if (x < 0 || x >= width)
        return 0;
    if (y < 0 || y >= height)
        return 0;
    return image[y * width + x];
}

void sobelCPU(unsigned char *image_in, unsigned char *image_out, int width, int height) {
    int i, j;
    int Gx, Gy;
    int tempPixel;

    //za vsak piksel v sliki
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++) {
            Gx = -getPixel(image_in, i - 1, j - 1, width, height) - 2 * getPixel(image_in, i - 1, j, width, height) -
                getPixel(image_in, i - 1, j + 1, width, height) + getPixel(image_in, i + 1, j - 1, width, height) +
                2 * getPixel(image_in, i + 1, j, width, height) + getPixel(image_in, i + 1, j + 1, width, height);
            Gy = -getPixel(image_in, i - 1, j - 1, width, height) - 2 * getPixel(image_in, i, j - 1, width, height) -
                getPixel(image_in, i + 1, j - 1, width, height) + getPixel(image_in, i - 1, j + 1, width, height) +
                2 * getPixel(image_in, i, j + 1, width, height) + getPixel(image_in, i + 1, j + 1, width, height);
            tempPixel = sqrt((float)(Gx * Gx + Gy * Gy));
            if (tempPixel > 255)
                image_out[i * width + j] = 255;
            else
                image_out[i * width + j] = tempPixel;
        }
}

void sobelGPU(unsigned char *image_in, unsigned char *image_out, int width, int height) {
    int imageSize = height * width * sizeof(unsigned char);
    cl_int ret;

    FILE *fp = fopen("sobel.cl", "r");
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
    /*
    size_t local_item_size = WORKGROUP_SIZE;
    size_t num_groups = (((width * height) - 1) / local_item_size + 1);
    size_t global_item_size = num_groups * local_item_size;
    */
    size_t local_item_size[] = { WORKGROUP_SIZE, WORKGROUP_SIZE };
    size_t num_groups[] = { ((width - 1) / local_item_size[0] + 1), ((height - 1) / local_item_size[1] + 1) };
    size_t global_item_size[] = { num_groups[0] * local_item_size[0], num_groups[1] * local_item_size[1] };


    // Alokacija pomnilnika na napravi
    cl_mem image_mem_obj_in = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      imageSize, image_in, &ret);
    cl_mem image_mem_obj_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
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
    printf("%s\n", build_log);
    free(build_log);

    // scepec: priprava objekta
    cl_kernel kernel = clCreateKernel(program, "sobel", &ret);
            // program, ime scepca, napaka

    double start, end;
    start = omp_get_wtime();
    // scepec: argumenti
    ret |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &image_mem_obj_in);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &image_mem_obj_out);
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_int), (void *) &width);
    ret |= clSetKernelArg(kernel, 3, sizeof(cl_int), (void *) &height);
            // scepec, stevilka argumenta, velikost podatkov, kazalec na podatke

    // scepec: zagon
    // ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_item_size, local_item_size, 0, NULL, NULL);
            // vrsta, scepec, dimenzionalnost, mora biti NULL,
            // kazalec na stevilo vseh niti, kazalec na lokalno stevilo niti, 
            // dogodki, ki se morajo zgoditi pred klicem

    // Kopiranje rezultatov
    ret = clEnqueueReadBuffer(command_queue, image_mem_obj_out, CL_TRUE, 0, imageSize, image_out, 0, NULL, NULL);
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
    ret = clReleaseMemObject(image_mem_obj_out);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
}

int main(int argc, char *argv[]) {
    // Load image from file
    FIBITMAP *imageBitmap = FreeImage_Load(FIF_PNG, "input.png", 0);
    // Convert it to an 8-bit grayscale image
    FIBITMAP *imageBitmap8 = FreeImage_ConvertTo8Bits(imageBitmap);

    // Get image dimensions
    int width = FreeImage_GetWidth(imageBitmap8);
    int height = FreeImage_GetHeight(imageBitmap8);
    int pitch = FreeImage_GetPitch(imageBitmap8);

    // Preapare room for a raw data copy of the image
    unsigned char *image_in = (unsigned char *) malloc(height * pitch * sizeof(unsigned char));
    FreeImage_ConvertToRawBits(image_in, imageBitmap8, pitch, 8, 0xFF, 0xFF, 0xFF, TRUE);

    unsigned char *image_out = (unsigned char *) malloc(height * pitch * sizeof(unsigned char));

    // Find edges
    #ifndef GPU
        double start, end;
        start = omp_get_wtime();

        sobelCPU(image_in, image_out, width, height);

        end = omp_get_wtime();
        printf("Velikost slike %dx%d, CPU (sekvencni) cas: %.6fs\n", width, height, end - start);
    #else
        sobelGPU(image_in, image_out, width, height);
    #endif

    // Save output image
    FIBITMAP *dst = FreeImage_ConvertFromRawBits(image_out, width, height, pitch,
        8, 0xFF, 0xFF, 0xFF, TRUE);
    FreeImage_Save(FIF_PNG, dst, "robovi.png", 0);

    return 0;
}

/*
WORKGROUP_SIZE: 16

Velikost slike 640x480, GPU (paralelni) cas: 0.000523s, CPU (sekvencni) cas: 0.006796s, pohitritev: 13x
Velikost slike 800x600, GPU (paralelni) cas: 0.001028s, CPU (sekvencni) cas: 0.010576s, pohitritev: 10x
Velikost slike 1600x900, GPU (paralelni) cas: 0.001687s, CPU (sekvencni) cas: 0.032115s, pohitritev: 19x
Velikost slike 1920x1080, GPU (paralelni) cas: 0.002319s, CPU (sekvencni) cas: 0.045312s, pohitritev: 20x
Velikost slike 3840x2160, GPU (paralelni) cas: 0.005645s, CPU (sekvencni) cas: 0.173020s, pohitritev: 31x
Velikost slike 15360x8640, GPU (paralelni) cas: 0.083240s, CPU (sekvencni) cas: 2.749263s, pohitritev: 33x
*/