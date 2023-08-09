/**
 * 6_iir_test.cpp
 * Written by Clyne Sullivan.
 *
 * Implements a simple infinite impulse response (IIR) filter using an alpha
 * parameter.
 * To build upon this example, try setting `alpha` with a parameter knob:
 * alpha = param1() / 4095.0
 */

Sample* process_data(Samples samples)
{
	constexpr float alpha = 0.7;

	static Sample prev = 2048;

	samples[0] = (1 - alpha) * samples[0] + alpha * prev;
	for (unsigned int i = 1; i < SIZE; i++)
		samples[i] = (1 - alpha) * samples[i] + alpha * samples[i - 1];
	prev = samples[SIZE - 1];

	return samples;
}
