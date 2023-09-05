Sample* process_data(Samples samples)
{
	const auto p = param1();
	const float scale = p / 4095.f;
	const auto offset = (4095 -	p) / 2;

	for (int i = 0; i < SIZE; ++i)
		samples[i] = samples[i] * scale + offset;

    return samples;
}

