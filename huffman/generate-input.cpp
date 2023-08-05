#include <iostream>
#include <fstream>
#include <random>
using namespace std;

int main()
{
    const int n = 1000000000;
    const float mean = 100.0f;
    const float std_dev = 20.0f;
    const string outputPath = "float-tests/1b-samples.in";

    random_device rd;
    mt19937 gen(rd());
    normal_distribution<float> dis(mean, std_dev);

    ofstream out(outputPath, ios::binary | ios::out);
    if (!out)
    {
        cerr << "Error creating the file.\n";
        return 1;
    }

    float random;

    for (int i = 0; i < n; ++i)
    {
        random = dis(gen);
        out.write(reinterpret_cast<const char *>(&random), sizeof(random));
    }

    out.close();

    cout << "File generated.\n";

    return 0;
}
