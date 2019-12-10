#include <immintrin.h>
#include <stdio.h>

using namespace std;

void printTest(int lineNum)
{
	printf("Reached line %d\n", lineNum);
}

void print256(__m256 floats)
{
	float* temp = (float*)calloc(8, sizeof(float));
	_mm256_stream_ps(temp, floats);
	printf("8 floats are: %f %f %f %f %f %f %f %f\n", temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
}

void printArray(float* vec, long length)
{
	for (long i = 0; i < length; i++)
	{
		printf("%f ", vec[i]);
		if (i % 8 == 7) printf("\n");
	}
	printf("\n");
}