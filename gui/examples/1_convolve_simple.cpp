/**
 * 1_convolve_simple.cpp
 * Written by Clyne Sullivan.
 *
 * Computes a convolution in the simplest way possible. While the code is brief, it lacks many
 * possible optimizations. The convolution's result will not fill the output buffer either, as the
 * transient response is not calculated.
 */

Sample* process_data(Samples samples)
{
    // Define our output buffer.
    static Samples buffer;

    // Define our filter
    constexpr unsigned int filter_size = 3;
	float filter[filter_size] = {
        0.3333, 0.3333, 0.3333
    };

    // Begin convolving:
    // SIZE is the size of the sample buffer.
    for (int n = 0; n < SIZE - (filter_size - 1); n++) {
        buffer[n] = 0;
        for (int k = 0; k < filter_size; k++)
            buffer[n] += samples[n + k] * filter[k];
    }

    return buffer;
}
