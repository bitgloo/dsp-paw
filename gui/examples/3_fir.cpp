/**
 * 3_fir.cpp
 * Written by Clyne Sullivan.
 *
 * The below code was written for applying FIR filters. While this is still essentially an overlap-
 * save convolution, other optimizations have been made to allow for larger filters to be applied
 * within the available execution time. Samples are also normalized so that they center around zero.
 */

Sample* process_data(Samples samples)
{
    static Samples buffer;

	// Define the filter:
	constexpr unsigned int filter_size = 3;
	static float filter[filter_size] = {
        // Put filter values here (note: precision will be truncated for 'float' size).
        0.3333, 0.3333, 0.3333
	};

    // Do an overlap-save convolution
    static Sample prev[filter_size];

    for (int n = 0; n < SIZE; n++) {
        // Using a float variable for accumulation allows for better code optimization
        float v = 0;

        for (int k = 0; k < filter_size; k++) {
            int i = n - (filter_size - 1) + k;

            auto s = i >= 0 ? samples[i] : prev[filter_size - 1 + i];
			// Sample values are 0 to 4095. Below, the original sample is normalized to a -1.0 to
            // 1.0 range for calculation.
            v += (s / 2048.f - 1) * filter[k];
        }

        // Return value to sample range of 0-4095.
        buffer[n] = (v + 1) * 2048.f;
    }

    // Save samples for next convolution
    for (int i = 0; i < filter_size; i++)
        prev[i] = samples[SIZE - filter_size + i];

    return buffer;
}

