__kernel void mandelbrot(int width, int height, __global unsigned char *image) {
    int max_iteration = 800; // Max stevilo iteracij
    unsigned char max = 255; // Max vrednost barvnega kanala

    int id = get_global_id(0);
    int i = id / width;
    int j = id % width;

    // Za vsak piksel v sliki
    float x0 = (float) j / width * (float) 3.5 - (float) 2.5; // Zacetna vrednost
    float y0 = (float) i / height * (float) 2.0 - (float) 1.0;
    float x = 0;
    float y = 0;
    int iter = 0;
    // Ponavljamo, dokler ne izpolnemo enega izmed pogojev
    while ((x * x + y * y <= 4) && (iter < max_iteration)) {
        float xtemp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtemp;
        iter++;
    }
    // Izracunamo barvo (magic: http://linas.org/art-gallery/escape/smooth.html)
    int color = 1.0 + iter - log(log(sqrt(x*x + y * y))) / log(2.0);
    color = (8 * max * color) / max_iteration;
    if (color > max)
        color = max;
    // Zapisemo barvo RGBA (v resnici little endian BGRA)
    int index = 4 * i * width + 4 * j;
    image[index + 0] = color; // Blue
    image[index + 1] = color; // Green
    image[index + 2] = 0; // Red
    image[index + 3] = 255; // Alpha
}
