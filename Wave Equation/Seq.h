#include "Sources/Background.h"

using namespace std;

class Seq {

private:
	void init(float* f, float* g)
	{
		for (long i = 0; i < DIST_STEPS; i++)
		{
			f[i] = sin(i * XPERIOD);
			g[i] = cos(i * XPERIOD);
		}
	}

	void secondU(const float* origU, float* g)
	{
		for (long i = 0; i < DIST_STEPS; i++)
		{
			g[i] = origU[i] + g[i] * PERIOD;
		}
	}

	void iterate(float* thisU, const float* nextU)
	{
		for (long i = 0; i < DIST_STEPS; i++)
		{
			thisU[i] = FAC1 * nextU[i] + FAC2 * ((i > 0 ? nextU[i - 1] : 0) + (i < DIST_STEPS - 1 ? nextU[i + 1] : 0)) - thisU[i];
		}
	}

public:
	void test()
	{
		float* thisU = (float*)malloc(DIST_STEPS * sizeof(float));
		float* nextU = (float*)malloc(DIST_STEPS * sizeof(float));

		double sumTime = 0;
		for (int n = 0; n < NUM_TESTS; n++) {
			double startTime = CycleTimer::currentSeconds();

			init(thisU, nextU);
			secondU(thisU, nextU);

			for (long t = 0; t < MAX_T; t++)
			{
				iterate(thisU, nextU);
				swap(thisU, nextU);
			}

			double endTime = CycleTimer::currentSeconds();
			sumTime += endTime - startTime;
		}
		printf("%f\n", sumTime / NUM_TESTS);


		//printArray(nextU, DIST_STEPS);

		free(thisU);
		free(nextU);
	}
};