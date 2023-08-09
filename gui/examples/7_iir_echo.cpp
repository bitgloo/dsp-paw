/**
 * 7_iir_echo.cpp
 * Written by Clyne Sullivan.
 *
 * This filter produces an echo of the given input. There are two parameters:
 * alpha controls the feedback gain, and D controls the echo/delay length.
 */

Sample* process_data(Samples samples)
{
	constexpr float alpha = 0.75;
	constexpr unsigned int D = 100;

	static Samples output;
	static Sample prev[D]; // prev[0] = output[0 - D]

	// Do calculations with previous output
	for (unsigned int i = 0; i < D; i++)
		output[i] = samples[i] + alpha * (prev[i] - 2048);

	// Do calculations with current samples
	for (unsigned int i = D; i < SIZE; i++)
		output[i] = samples[i] + alpha * (output[i - D] - 2048);

	// Save outputs for next computation
	for (unsigned int i = 0; i < D; i++)
		prev[i] = output[SIZE - (D - i)];

	return output;
}
