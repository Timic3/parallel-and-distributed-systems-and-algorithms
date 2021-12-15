#define BINS 256

__kernel void histogram(
                        __global unsigned char *image_in,
                        __global unsigned int *count_r_out,
                        __global unsigned int *count_g_out,
                        __global unsigned int *count_b_out,
                        const int width,
                        const int height) {
    size_t g_id = get_global_id(0);
    size_t l_id = get_local_id(0);

    __local unsigned int l_r[BINS];
    __local unsigned int l_g[BINS];
    __local unsigned int l_b[BINS];
    if (g_id < width * height) {
        l_r[l_id] = 0;
        l_g[l_id] = 0;
        l_b[l_id] = 0;

        barrier(CLK_LOCAL_MEM_FENCE);

        atomic_inc(&l_r[image_in[g_id * 4 + 2]]);
        atomic_inc(&l_g[image_in[g_id * 4 + 1]]);
        atomic_inc(&l_b[image_in[g_id * 4]]);

        barrier(CLK_LOCAL_MEM_FENCE);

        atomic_add(&count_r_out[l_id], l_r[l_id]);
        atomic_add(&count_g_out[l_id], l_g[l_id]);
        atomic_add(&count_b_out[l_id], l_b[l_id]);

        /*
        // Global - this works
        atomic_inc(&count_r_out[image_in[g_id * 4 + 2]]);
        atomic_inc(&count_g_out[image_in[g_id * 4 + 1]]);
        atomic_inc(&count_b_out[image_in[g_id * 4]]);
        */
    }
}
