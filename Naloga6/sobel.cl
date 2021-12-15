#define WORKGROUP_SIZE 16

__kernel void sobel(__global unsigned char *image_in, __global unsigned char *image_out, const int width, const int height) {
    int Gx, Gy, tempPixel;
    size_t g_x = get_global_id(0);
    size_t g_y = get_global_id(1);

    size_t l_x = get_local_id(0) + 1;
    size_t l_y = get_local_id(1) + 1;

    const size_t g_id = g_y * width + g_x;

    __local unsigned char l_data[(WORKGROUP_SIZE + 2)][(WORKGROUP_SIZE + 2)];

    if (g_x < width && g_y < height) {
        l_data[l_x][l_y] = image_in[g_id];

        // left
        if (l_x == 1)
            l_data[0][l_y] = image_in[g_id - 1];
        // right
        else if (l_x == WORKGROUP_SIZE)
            l_data[WORKGROUP_SIZE + 1][l_y] = image_in[g_id + 1];

        // top
        if (l_y == 1) {
            l_data[l_x][0] = image_in[g_id - width];

            // top-left
            if (l_x == 1)
                l_data[0][0] = image_in[g_id - width - 1];
            // top-right
            else if (l_x == WORKGROUP_SIZE)
                l_data[WORKGROUP_SIZE + 1][0] = image_in[g_id - width + 1];
        }

        // bottom
        if (l_y == WORKGROUP_SIZE) {
            l_data[l_x][WORKGROUP_SIZE + 1] = image_in[g_id + width];

            // bottom-left
            if (l_x == 1)
                l_data[0][WORKGROUP_SIZE + 1] = image_in[g_id + width - 1];
            // bottom-right
            else if (l_x == WORKGROUP_SIZE)
                l_data[WORKGROUP_SIZE + 1][WORKGROUP_SIZE + 1] = image_in[g_id + width + 1];
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (g_x < width && g_y < height) {
        Gx = -l_data[l_x - 1][l_y - 1] - 2 * l_data[l_x - 1][l_y] - l_data[l_x - 1][l_y + 1] +
            l_data[l_x + 1][l_y - 1] + 2 * l_data[l_x + 1][l_y] + l_data[l_x + 1][l_y + 1];
        Gy = -l_data[l_x - 1][l_y - 1] - 2 * l_data[l_x][l_y - 1] - l_data[l_x + 1][l_y - 1] +
            l_data[l_x - 1][l_y + 1] + 2 * l_data[l_x][l_y + 1] + l_data[l_x + 1][l_y + 1];

        tempPixel = sqrt((float)(Gx * Gx + Gy * Gy));
        if (tempPixel > 255)
            image_out[g_id] = 255;
        else
            image_out[g_id] = tempPixel;
    }
}
