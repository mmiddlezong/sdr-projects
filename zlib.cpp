#include <iostream>
#include <fstream>
#include <vector>
#include <zlib.h>
#include <chrono>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

string transformString(const string& inputString) {
    string outputString = inputString;
    size_t pos = outputString.find("data");
    if (pos != string::npos) {
        outputString.replace(pos, 4, "output-zlib");
    }
    outputString += ".gz";
    return outputString;
}

int main()
{
    // Path to the input
    string dataPath = "data";

    // Keep track of totals
    double totalTime = 0;
    double totalOriginalSize = 0;
    double totalCompressedSize = 0;

    for (const auto &entry : fs::directory_iterator(dataPath))
    {
        if (fs::is_regular_file(entry))
        {
            string inputFilePath = entry.path();
            string outputFilePath = transformString(inputFilePath);

            // Open the input file in binary mode and move the file pointer to the end
            ifstream inputFile(inputFilePath, ios::binary | ios::ate);
            streamsize fileSize = inputFile.tellg();
            inputFile.seekg(0, ios::beg);

            // Read the file into a buffer
            vector<char> buffer(fileSize);
            if (!inputFile.read(buffer.data(), fileSize))
            {
                cerr << "Error reading the file." << endl;
                return 1;
            }

            // Allocate memory for compressed data
            uLong maxCompressedSize = compressBound(fileSize);
            vector<char> compressedData(maxCompressedSize);

            // Start timer
            auto startTime = chrono::high_resolution_clock::now();

            // Compress the data
            int result = compress2(reinterpret_cast<Bytef*>(compressedData.data()), &maxCompressedSize, 
                                   reinterpret_cast<const Bytef*>(buffer.data()), fileSize, Z_BEST_SPEED);
            if (result != Z_OK)
            {
                cerr << "Error compressing the data: " << result << endl;
                return 1;
            }

            // End timer
            auto endTime = chrono::high_resolution_clock::now();

            // Write the compressed data to the output file
            ofstream outputFile(outputFilePath, ios::binary);
            outputFile.write(compressedData.data(), maxCompressedSize);

            // Calculate
            double compressionRatio = static_cast<double>(fileSize) / static_cast<double>(maxCompressedSize);

            double elapsedTime = chrono::duration<double>(endTime - startTime).count();
            double throughput = (static_cast<double>(fileSize) / (1024 * 1024)) / elapsedTime;

            // Update totals
            totalTime += elapsedTime;
            totalOriginalSize += static_cast<double>(fileSize);
            totalCompressedSize += static_cast<double>(maxCompressedSize);

            cout << "File at " << inputFilePath << " successfully compressed." << endl;
            cout << "Compression ratio: " << compressionRatio << endl;
            cout << "Throughput: " << throughput << " MB/s" << endl;
        }
    }
    
    cout << "-------" << endl;

    cout << "Total compression time: " << totalTime << "s" << endl;
    cout << "Original size: " << totalOriginalSize / (1024 * 1024) << " MB" << endl;
    cout << "Compressed size: " << totalCompressedSize / (1024 * 1024) << " MB" << endl;
    cout << "Overall compression ratio: " << totalOriginalSize / totalCompressedSize << endl;
    cout << "Overall throughput: " << totalOriginalSize / (1024 * 1024) / totalTime << " MB/s" << endl;

    return 0;
}
