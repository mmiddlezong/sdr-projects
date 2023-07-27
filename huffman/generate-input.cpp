#include <iostream>
#include <fstream>
#include <random>
using namespace std;
int main()
{
    const int n = 10000;
    const int min = 1;
    const int max = 1000;
    const string outputPath = "input.in";

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution dis(min, max);

    ofstream out(outputPath, ios::binary | ios::out);
    if (!out)
    {
        cerr << "Error creating the file.\n";
        return 1;
    }

    srand(static_cast<unsigned>(time(0)));

    int random;

    for (int i = 0; i < n; ++i)
    {
        random = dis(gen);
        out.write(reinterpret_cast<const char *>(&random), sizeof(random));
    }

    cout << "File generated.\n";

    return 0;
}