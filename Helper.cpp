#include <immintrin.h>
#include <iostream>

using namespace std;

void printTest(int lineNum)
{
	cout << "Reached line " << lineNum << endl;
}

void print256(__m256 floats)
{
	float* temp = (float*)calloc(8, sizeof(float));
	_mm256_stream_ps(temp, floats);
	cout << "8 floats are: " << temp[0] << ", " << temp[1] << ", " << temp[2] << ", " << temp[3] << ", " << temp[4] << ", " << temp[5] << ", " << temp[6] << ", " << temp[7] << endl;
}

void printArray(float* vec, long length)
{
	for (long i = 0; i < length; i++)
	{
		cout << vec[i] << " ";
		if (i % 8 == 7) cout << endl;
	}
	cout << endl;
}
