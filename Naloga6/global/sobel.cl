int getPixel(__global unsigned char *image_in, int y, int x, int width, int height) {
    if (x < 0 || x >= width)
        return 255;
    if (y < 0 || y >= height)
        return 255;
    return image_in[y * width + x];
}

__kernel void sobel(__global unsigned char *image_in, __global unsigned char *image_out, const int width, const int height) {
    int Gx, Gy, tempPixel;
    int id = get_global_id(0);
    int i = id / width;
    int j = id % width;

    Gx = -getPixel(image_in, i - 1, j - 1, width, height) - 2 * getPixel(image_in, i - 1, j, width, height) -
        getPixel(image_in, i - 1, j + 1, width, height) + getPixel(image_in, i + 1, j - 1, width, height) +
        2 * getPixel(image_in, i + 1, j, width, height) + getPixel(image_in, i + 1, j + 1, width, height);
    Gy = -getPixel(image_in, i - 1, j - 1, width, height) - 2 * getPixel(image_in, i, j - 1, width, height) -
        getPixel(image_in, i + 1, j - 1, width, height) + getPixel(image_in, i - 1, j + 1, width, height) +
        2 * getPixel(image_in, i, j + 1, width, height) + getPixel(image_in, i + 1, j + 1, width, height);

    tempPixel = sqrt((float)(Gx * Gx + Gy * Gy));
    if (i < height) {
        if (tempPixel > 255)
            image_out[i * width + j] = 255;
        else
            image_out[i * width + j] = tempPixel;
    }
}
