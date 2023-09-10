#include <iostream>
#include <fstream>
#include <random>
#include <cmath>

using namespace std;

int main()
{
    const bool print = false;
    const int n = 10000000;
    const float period = 100.0f;
    const float amplitude = 10.0f;
    const float verticalShift = 10.0f;
    const float noiseStdDev = 0.2f;
    const string outputPath = "continuous-tests/sine-10m.in";

    random_device rd;
    mt19937 gen(rd());
    normal_distribution<float> dis(0, noiseStdDev);

    ofstream out(outputPath, ios::binary | ios::out);
    if (!out)
    {
        cerr << "Error creating the file.\n";
        return 1;
    }

    float random;

    if (print) cout << "Data being generated: ";
    for (int i = 0; i < n; ++i)
    {
        random = dis(gen) + sin(i * 2 * 3.1415f / period) * amplitude + verticalShift;
        if (print) cout << random << " ";
        out.write(reinterpret_cast<const char *>(&random), sizeof(random));
    }

    out.close();

    cout << "\nFile generated.\n";

    return 0;
}
