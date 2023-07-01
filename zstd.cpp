#include <iostream>
#include <fstream>
#include <vector>
#include <zstd.h>
#include <chrono>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

string transformString(const string& inputString) {
    string outputString = inputString;
    size_t pos = outputString.find("data");
    if (pos != string::npos) {
        outputString.replace(pos, 4, "output");
    }
    outputString += ".zst";
    return outputString;
}

int main()
{

    // Path to the input and output files
    string dataPath = "data";
    string outputPath = "output";

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
            size_t const maxCompressedSize = ZSTD_compressBound(fileSize);
            vector<char> compressedData(maxCompressedSize);

            // Start timer
            auto startTime = chrono::high_resolution_clock::now();

            // Compress the data
            size_t const compressedSize = ZSTD_compress(compressedData.data(), maxCompressedSize, buffer.data(), fileSize, 3);
            if (ZSTD_isError(compressedSize))
            {
                cerr << "Error compressing the data: " << ZSTD_getErrorName(compressedSize) << endl;
                return 1;
            }

            // End timer
            auto endTime = chrono::high_resolution_clock::now();

            // Write the compressed data to the output file
            ofstream outputFile(outputFilePath, ios::binary);
            outputFile.write(compressedData.data(), compressedSize);

            // Calculate
            double compressionRatio = static_cast<double>(fileSize) / static_cast<double>(compressedSize);

            double elapsedTime = chrono::duration<double>(endTime - startTime).count();
            double throughput = (static_cast<double>(fileSize) / (1024 * 1024)) / elapsedTime;

            // Update totals
            totalTime += elapsedTime;
            totalOriginalSize += static_cast<double>(fileSize);
            totalCompressedSize += static_cast<double>(compressedSize);

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
