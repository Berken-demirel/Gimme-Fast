#include <iostream>


using namespace std;

int main()
{
	int data_bits[7] = { 1,0,1,1,1,1,1 };

	int redundant_bits[4];

	const int R1_bits[5] = { 0, 1, 3, 4, 6 };
	const int R2_bits[5] = { 0, 2, 3, 5, 6 };
	const int R4_bits[5] = { 1, 2, 3 };
	const int R8_bits[5] = { 4, 5, 6 };

	int count_R1_bit = 0;
	int count_R2_bit = 0;
	int count_R4_bit = 0;
	int count_R8_bit = 0;


	for (const int &each_R1_index : R1_bits)
	{
		if (data_bits[each_R1_index] == 1)
			count_R1_bit++;
	}

	for (const int &each_R2_index : R2_bits)
	{
		if (data_bits[each_R2_index] == 1)
			count_R1_bit++;
	}

	for (const int &each_R4_index : R4_bits)
	{
		if (data_bits[each_R4_index] == 1)
			count_R1_bit++;
	}

	for (const int &each_R8_index : R8_bits)
	{
		if (data_bits[each_R8_index] == 1)
			count_R1_bit++;
	}

	redundant_bits[0] = (count_R1_bit % 2 == 0) ? 0 : 1;

	redundant_bits[1] = (count_R2_bit % 2 == 0) ? 0 : 1;

	redundant_bits[2] = (count_R4_bit % 2 == 0) ? 0 : 1;

	redundant_bits[3] = (count_R8_bit % 2 == 0) ? 0 : 1;

	int package[11];

	package[0] = redundant_bits[0];

	package[1] = redundant_bits[1];

	package[2] = data_bits[0];

	package[3] = redundant_bits[2];

	package[4] = data_bits[1];

	package[5] = data_bits[2];

	package[6] = data_bits[3];

	package[7] = redundant_bits[3];

	package[8] = data_bits[4];

	package[9] = data_bits[5];

	package[10] = data_bits[6];


	system("Pause");
}