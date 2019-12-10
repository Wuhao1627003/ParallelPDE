#include <iostream>
#include <cmath>
#include <chrono>

using namespace std;

int main()
{

    int hs[] = {100, 10, 1};

    for (int h : hs)
    {
        auto begin = chrono::high_resolution_clock::now();
        double f = 2;
        double fprime = exp(0) + 5 - 3 * f;
        for (int time = 0; time <= 1000000; time += h)
        {
            f += (h / 100.0) * fprime;
            fprime = exp((time / 100.0)) + 5 - 3 * f;
        }
        auto end = chrono::high_resolution_clock::now();
        cout << chrono::duration_cast<chrono::microseconds>(end-begin).count() << endl;
    }
    return 0;
}