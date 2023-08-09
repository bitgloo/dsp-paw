/**
 * 2_convolve_overlap_save.cpp
 * Written by Clyne Sullivan.
 *
 * This convolution examples takes an overlap-save approach, where samples from the previous run
 * are saved so that the overall operation is not interrupted (i.e. the observed output will
 * transition smoothly between processed "chunks").
 *
 * Note that there are still improvements that can be made to the code; for example, notice every
 * spot where an integer/float conversion is necessary. Operations like these may slow down the
 * computation.
 */

Sample* process_data(Samples samples)
{
    static Samples buffer;

    constexpr unsigned int filter_size = 3;
	float filter[filter_size] = {
        0.3333, 0.3333, 0.3333
    };

    // Keep a buffer of extra samples for overlap-save
    static Sample prev[filter_size];

    for (int n = 0; n < SIZE; n++) {
        buffer[n] = 0;

        for (int k = 0; k < filter_size; k++) {
            int i = n - (filter_size - 1) + k;

            // If i is >= 0, access current sample buffer.
            // If i is < 0, provide the previous samples from the 'prev' buffer
            if (i >= 0)
                buffer[n] += samples[i] * filter[k];
            else
                buffer[n] += prev[filter_size - 1 + i] * filter[k];
        }
    }

    // Save samples for the next convolution run
    for (int i = 0; i < filter_size; i++)
        prev[i] = samples[SIZE - filter_size + i];

    return buffer;
}

