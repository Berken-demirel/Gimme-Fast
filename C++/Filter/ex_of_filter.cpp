#include <iostream>

using namespace std;

const int length_of_signal = 1000;
const int data_counter_vector_size = 8;

double *ApplyFilt_bandpass_40(double data_to_filter, double data_counter_vector);

int main()
{
	double *data_to_filter = NULL;
	double *filtered_signal = NULL;

	data_to_filter = new double[length_of_signal];

	filtered_signal = ApplyFilt_bandpass_40(data_to_filter, data_counter_vector_size);

}

double *ApplyFilt_bandpass_40(double *data_to_filter, double data_counter_vector)
{
	//    f1 = 0.7;
	//    f2 = 40;
	//    Fs = 330;
	//    Wn=[f1 f2] * 2 / Fs ;
	//    N = 3;
	//    [b, a] = butter(N,Wn,'bandpass');

	int length_of_signal = sizeof(data_to_filter);
	int naxpy;
	int filtered_signal_tmp;
	double *filtered_signal = NULL;
	double *filtered_signal_last = NULL;
	filtered_signal = new double[length_of_signal];
	filtered_signal_last = new double[sizeof(data_counter_vector)];

	static const double dv0[7] = { 0.028311406105033091, -0.0,
	  -0.084934218315099258, -0.0, 0.084934218315099258, -0.0,
	  -0.028311406105033091 };  // coeff of zeros of filter

	double as;

	static const double dv1[7] = { 1.0, -4.5012338735568731, 8.4670715663573777,
	  -8.6010205491081084, 5.0194996412232413, -1.5979691045724465,
	  0.21365288874600138 }; // coeff of poles of Filter

	for (int var = 0; var < length_of_signal; ++var)
	{
		if (length_of_signal - var < 7)
		{
			naxpy = length_of_signal - 1 - var;
		}
		else
		{
			naxpy = 6;

		}

		for (int dummy = 0; dummy <= naxpy; dummy++)
		{
			filtered_signal_tmp = dummy + var;
			filtered_signal[filtered_signal_tmp] += data_to_filter[var] * dv0[dummy];
		}

		if (length_of_signal - 1 - var < 6)
		{
			naxpy = length_of_signal - 2 - var;
		}

		else
		{
			naxpy = 5;

		}

		as = -filtered_signal[var];

		for (int temp = 0; temp <= naxpy; temp++)
		{
			filtered_signal_tmp = (temp + var) + 1;
			filtered_signal[filtered_signal_tmp] += as * dv1[temp + 1];
		}

	}

	// Take only plotting data for filter
	for (int var = 0; var < data_counter_vector_size; ++var)
	{
		filtered_signal_last[var] = filtered_signal[sizeof(filtered_signal) + var - sizeof(data_counter_vector_size)];
	}

	return filtered_signal_last;

}
