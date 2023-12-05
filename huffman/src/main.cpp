#include <algorithm>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "extrapolate.h"
#include "huffman.h"

using namespace std;
namespace fs = filesystem;

void readFloats(const string &inputPath, vector<float> &inputFloats) {
    ifstream file(inputPath, ios::binary | ios::ate);
    if (!file) {
        cerr << "Failed to open the file.\n";
        return;
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t numFloats = size / sizeof(float);
    inputFloats.reserve(numFloats);

    std::vector<float> buffer(numFloats);
    if (file.read(reinterpret_cast<char *>(buffer.data()),
                  numFloats * sizeof(float))) {
        inputFloats.insert(inputFloats.end(), buffer.begin(), buffer.end());
    } else {
        cerr << "Error reading the file.\n";
    }
}

enum ErrorMode {
    absolute,
    relative
};
float getAbsAverage(const std::vector<float> &vec) {
    if (vec.empty()) {
        return 0.0f; // Handle empty vector case
    }

    float sum = 0.0f;
    for (float num : vec) {
        sum += abs(num);
    }

    return sum / vec.size();
}

void compressFile(const string &inputPath, const string &outputPath,
                  const float &error, const ErrorMode &errorMode,
                  const ExtrapolationMethod &extrapolationMethod, const bool debugMode) {
    vector<float> inputFloats;
    readFloats(inputPath, inputFloats);

    const long long n = inputFloats.size();
    if (n < 2) {
        cerr << "File contains fewer than two data points.\n";
        return;
    }

    float minFloat = std::numeric_limits<float>::max();
    float maxFloat = std::numeric_limits<float>::lowest();

    for (float f : inputFloats) {
        if (f < minFloat)
            minFloat = f;
        if (f > maxFloat)
            maxFloat = f;
    }

    float range = maxFloat - minFloat;

    float maxError;
    // Calculate absolute error
    if (errorMode == absolute) {
        maxError = error;
    } else {
        maxError = range * error;
    }

    if (abs(maxError) < 1.0E-15F) {
        cout << "WARNING! Max error has extremely small magnitude: " << maxError
             << "\n";
    }

    vector<float> extrapolateErrors(n - 2); // Size n-2
    vector<float> lossyData(n);             // Size n
    lossyData[0] = inputFloats[0];
    lossyData[1] = inputFloats[1];

    ofstream extrapErrorsFile;
    if (debugMode) {
        extrapErrorsFile.open(outputPath + "-extrap-errors.txt");
    }

    // Extrapolation step
    for (size_t i = 2; i < inputFloats.size(); ++i) {
        const float extrapolatedFloat =
            extrapolateNext(lossyData, i, extrapolationMethod);
        const float err = inputFloats[i] - extrapolatedFloat;
        extrapolateErrors[i - 2] = err;

        if (debugMode && extrapErrorsFile.is_open()) {
            extrapErrorsFile << err << "\n";
        }

        // Figure out what err would quantize to
        float quantizedErr = round(err / (2 * maxError)) * 2 * maxError;
        lossyData[i] = extrapolatedFloat + quantizedErr;
    }

    if (debugMode && extrapErrorsFile.is_open()) {
        extrapErrorsFile.close();
    }

    vector<int> inputInts; // Size n-2
    unordered_map<int, unsigned> freqMap;
    ofstream quantizationLevelsFile;
    if (debugMode) {
        quantizationLevelsFile.open(outputPath + "-quantization-levels.txt");
    }
    // Split into buckets of size 2 * maxError, where bucket 0 is centered at 0
    for (const auto &err : extrapolateErrors) {
        int bucket = round(err / (2 * maxError));
        inputInts.push_back(bucket);
        freqMap[bucket]++;

        if (debugMode && quantizationLevelsFile.is_open()) {
            quantizationLevelsFile << bucket << "\n";
        }
    }
    if (debugMode && quantizationLevelsFile.is_open()) {
        quantizationLevelsFile.close();
    }

    Node *tree = generateHuffmanTree(freqMap);
    unordered_map<int, string> code = getHuffmanCode(tree);

    string encoded = encode(inputInts, code);
    string buffer = serializeTree(tree);
    unsigned bufferSize =
        buffer.length(); // Get number of bits needed to store the tree
    unsigned long long encodedSize = encoded.length();

    // Write the compressed file
    ofstream out(outputPath, ios::binary | ios::out);

    // 4 + 4 bytes to store first two data points
    out.write(reinterpret_cast<char *>(&inputFloats[0]), sizeof(maxError));
    out.write(reinterpret_cast<char *>(&inputFloats[1]), sizeof(maxError));

    // 4 bytes to store maxError
    out.write(reinterpret_cast<const char *>(&maxError), sizeof(maxError));

    // 4 + 8 bytes to store bufferSize and encodedSize
    out.write(reinterpret_cast<char *>(&bufferSize), sizeof(bufferSize));
    out.write(reinterpret_cast<char *>(&encodedSize), sizeof(encodedSize));

    writeBitsToFile(out, buffer);
    writeBitsToFile(out, encoded);

    out.close();
}

void decompressFile(const string &inputPath, const string &outputPath,
                    const ExtrapolationMethod &extrapolationMethod) {
    auto startTotal = chrono::high_resolution_clock::now();
    ifstream compressed(inputPath, ios::binary);

    if (!compressed.is_open()) {
        cerr << "File could not be opened.\n";
        return;
    }

    float maxError;
    float x0, x1;
    vector<float> reconstructedData;
    unsigned bufferSize;
    unsigned long long encodedSize;

    compressed.read(reinterpret_cast<char *>(&x0), sizeof(x0));
    compressed.read(reinterpret_cast<char *>(&x1), sizeof(x1));
    reconstructedData.push_back(x0);
    reconstructedData.push_back(x1);

    compressed.read(reinterpret_cast<char *>(&maxError), sizeof(maxError));

    compressed.read(reinterpret_cast<char *>(&bufferSize), sizeof(bufferSize));
    compressed.read(reinterpret_cast<char *>(&encodedSize), sizeof(encodedSize));

    string serializedTree = readBitsIntoString(compressed, bufferSize);
    string encodedData = readBitsIntoString(compressed, encodedSize);

    compressed.close();

    Node *deserializedTree = deserializeTree(serializedTree);
    vector<int> decodedInts = decode(encodedData, deserializedTree);
    ofstream decodedFile(outputPath, ios::binary | ios::out);
    if (!decodedFile) {
        cerr << "Error creating the file.\n";
        return;
    }

    // Reconstruction step
    for (size_t i = 0; i < decodedInts.size(); ++i) {
        // Convert the bucket number back to float
        const float decodedErr = decodedInts[i] * 2 * maxError;

        // Extrapolate the new data point and adjust for error
        const float extrapolatedFloat =
            extrapolateNext(reconstructedData, i + 2, extrapolationMethod);
        const float reconstructedFloat = extrapolatedFloat + decodedErr;
        reconstructedData.push_back(reconstructedFloat);
    }

    // Write reconstructed data to file
    for (const auto &f : reconstructedData) {
        decodedFile.write(reinterpret_cast<const char *>(&f), sizeof(f));
    }
    decodedFile.close();
    auto endTotal = chrono::high_resolution_clock::now();
    auto durationTotal =
        chrono::duration_cast<chrono::microseconds>(endTotal - startTotal);
    // cout << "Decoding time: " << durationTotal.count() << " microseconds.\n";
}

size_t getFileSize(const string &filePath) {
    ifstream in(filePath, ios::ate | ios::binary);
    if (!in.is_open()) {
        cerr << "File could not be opened.\n";
        return 0;
    }
    return in.tellg();
}

void outputErrors(const string &filePath1, const string &filePath2,
                  const string &outputPath) {
    ifstream file1(filePath1, ios::binary);
    ifstream file2(filePath2, ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        throw runtime_error("Files could not be opened.");
    }

    float v1, v2;
    ofstream out(outputPath);
    if (!out) {
        throw runtime_error("Invalid output path.");
    }

    while (file1.read(reinterpret_cast<char *>(&v1), sizeof(v1))) {
        file2.read(reinterpret_cast<char *>(&v2), sizeof(v2));
        float curError = abs(v2 - v1);
        out << curError << "\n";
    }

    file1.close();
    file2.close();
    out.close();
}

pair<float, float> getMaxAndAvgError(const string &filePath1,
                                     const string &filePath2) {
    ifstream file1(filePath1, ios::binary);
    ifstream file2(filePath2, ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        throw runtime_error("Files could not be opened.");
    }

    float v1, v2;
    float maxError = 0.0f;
    int count = 0;
    float sumError = 0.0f;
    while (file1.read(reinterpret_cast<char *>(&v1), sizeof(v1))) {
        file2.read(reinterpret_cast<char *>(&v2), sizeof(v2));
        float curError = abs(v2 - v1);
        maxError = max(maxError, curError);

        sumError += curError;
        count++;
    }

    float avgError = sumError / count;

    file1.close();
    file2.close();

    return make_pair(maxError, avgError);
}

string getCurrentTimeFormatted() {
    time_t t = time(nullptr);
    tm *localTime = localtime(&t);

    char buffer[20];

    // Use strftime to format the time
    strftime(buffer, sizeof(buffer), "%Y-%m-%d.%H:%M:%S", localTime);

    return string(buffer);
}

void compressDataset(const fs::path &datasetDirectory, float maxError,
                     const ErrorMode &errorMode, const string &errorModeName,
                     const ExtrapolationMethod &extrapolationMethod,
                     const string &methodName, const bool debugMode) {
    vector<string> testCases;

    fs::path outputDir = "out" / datasetDirectory;
    fs::create_directories(outputDir);

    for (const auto &entry : fs::directory_iterator(datasetDirectory)) {
        if (entry.is_regular_file()) {
            testCases.push_back(entry.path().filename().string());
        }
    }

    // Declare totals
    size_t datasetSize = 0;
    size_t compressedDatasetSize = 0;
    float totalCompressionTime = 0.0f;

    // Dataset compression log
    string compressionLogName = "compression-" + methodName + "-" +
                                getCurrentTimeFormatted() +
                                ".log"; // Based on current time
    ofstream compressionLog(outputDir / compressionLogName);
    if (!compressionLog.is_open()) {
        throw runtime_error("File could not be opened");
    }

    for (string filename : testCases) {
        fs::path inputPath = datasetDirectory / filename;
        fs::path compressedPath = outputDir / (filename + "-compressed.bin");
        fs::path outputPath = outputDir / (filename + "-decompressed.bin");

        auto c0 = chrono::high_resolution_clock::now();
        compressFile(inputPath, compressedPath, maxError, errorMode,
                     extrapolationMethod, debugMode);
        auto c1 = chrono::high_resolution_clock::now();
        decompressFile(compressedPath, outputPath, extrapolationMethod);
        auto c2 = chrono::high_resolution_clock::now();

        // Get time in ms
        auto compressionTime =
            chrono::duration_cast<chrono::microseconds>(c1 - c0).count() / 1000.0f;
        auto decompressionTime =
            chrono::duration_cast<chrono::microseconds>(c2 - c1).count() / 1000.0f;

        size_t originalSize = getFileSize(inputPath);
        size_t compressedSize = getFileSize(compressedPath);

        fs::path errorsPath = outputDir / (filename + "-errors.txt");
        // outputErrors(inputPath, outputPath, errorsPath);

        pair<float, float> maxAndAvgError =
            getMaxAndAvgError(inputPath, outputPath);
        float curMaxError = maxAndAvgError.first;
        float curAvgError = maxAndAvgError.second;

        cout << "Compressed " << filename << "...\n";

        compressionLog << "Compressed " << filename << ":\n";
        compressionLog << "- Max error: " << curMaxError << "\n";
        compressionLog << "- Avg error: " << curAvgError << "\n";
        compressionLog << "- Original file size: " << originalSize << " B\n";
        compressionLog << "- Compressed file size: " << compressedSize << " B\n";
        compressionLog << "- Compression..."
                       << "\n";
        compressionLog << "-   ratio: " << (float)originalSize / compressedSize
                       << "\n";
        compressionLog << "-   time: " << compressionTime << " ms\n";
        compressionLog << "-   throughput: "
                       << (float)originalSize / compressionTime << " KB/s\n";
        compressionLog << "- Decompression..."
                       << "\n";
        compressionLog << "-   time: " << decompressionTime << " ms\n";
        compressionLog << "-   throughput: "
                       << (float)originalSize / decompressionTime << " KB/s\n";

        // Update running totals for the entire dataset
        datasetSize += originalSize;
        compressedDatasetSize += compressedSize;
        totalCompressionTime += compressionTime;
    }

    cout << "FINISHED! Summary:\n";

    cout << "- Dataset size: " << datasetSize << " B\n";
    cout << "- Compressed dataset size: " << compressedDatasetSize << " B\n";
    cout << "- Compression time: " << totalCompressionTime << " ms\n";
    cout << "METRICS:\n";
    cout << "- Extrapolation method: " << methodName << "\n";
    cout << "- Max error: " << maxError << " " << errorModeName << "\n";
    cout << "- Overall compression ratio: "
         << (float)datasetSize / compressedDatasetSize << "\n";
    cout << "- Overall throughput: " << (float)datasetSize / totalCompressionTime
         << " KB/s\n";

    compressionLog << "FINISHED! Summary:\n";

    compressionLog << "- Dataset size: " << datasetSize << " B\n";
    compressionLog << "- Compressed dataset size: " << compressedDatasetSize
                   << " B\n";
    compressionLog << "- Compression time: " << totalCompressionTime << " ms\n";
    compressionLog << "METRICS:\n";
    compressionLog << "- Extrapolation method: " << methodName << "\n";
    compressionLog << "- Max error: " << maxError << " " << errorModeName << "\n";
    compressionLog << "- Overall compression ratio: "
                   << (float)datasetSize / compressedDatasetSize << "\n";
    compressionLog << "- Overall throughput: "
                   << (float)datasetSize / totalCompressionTime << " KB/s\n";
    if (debugMode) {
        compressionLog << "WARNING: DEBUG MODE WAS ON\n";
    }

    compressionLog.close();
}

int main() {

    static unordered_map<string, ErrorMode> const errorModeNames = {
        {"absolute", absolute}, {"relative", relative}};
    static unordered_map<string, ExtrapolationMethod> const methodNames = {
        {"linear", linear},
        {"piecewise", piecewise},
        {"none", none},
        {"quadratic", quadratic}};

    vector<fs::path> datasets = {"real-datasets/CESM-ATM", "real-datasets/EXAALT",
                                 "real-datasets/ISABEL"};
    vector<float> errors = {1E-2, 1E-3, 1E-4, 1E-5, 1E-6};
    vector<string> methods = {"none"};

    for (const fs::path &dataset : datasets) {
        for (const float &error : errors) {
            for (const string &method : methods) {
                // compressDataset(dataset, error, relative, "relative",
                // methodNames.at(method), method);
            }
        }
    }

    // return 0;

    // Take in inputs
    fs::path testDir;
    cout << "Enter dataset directory to test: ";
    cin >> testDir;

    string inputErrorMode;
    cout << "Enter error mode (absolute, relative): ";
    cin >> inputErrorMode;

    // Parse the error mode

    ErrorMode errorMode;
    auto it = errorModeNames.find(inputErrorMode);
    if (it != errorModeNames.end()) {
        errorMode = it->second;
    } else {
        throw runtime_error("Invalid error mode");
    }

    float maxError;
    if (errorMode == absolute) {
        cout << "Enter max absolute error: ";
    } else if (errorMode == relative) {
        cout << "Enter max relative error: ";
    }
    cin >> maxError;

    string inputMethod;
    cout << "Enter extrapolation method (linear, piecewise, none, quadratic): ";
    cin >> inputMethod;

    // Parse the extrapolation method

    ExtrapolationMethod extrapolationMethod;
    auto it_ = methodNames.find(inputMethod);
    if (it_ != methodNames.end()) {
        extrapolationMethod = it_->second;
    } else {
        throw runtime_error("Invalid extrapolation method");
    }

    // Verify if user wants to continue
    string debugModeInput;
    cout << "Debug mode (y/n)? ";
    cin >> debugModeInput;
    bool debugMode = false;
    if (debugModeInput == "y") {
        debugMode = true;
    }

    // Verify if user wants to continue
    string inputContinue;
    cout << "Continue (y/n)? ";
    cin >> inputContinue;
    if (inputContinue != "y") {
        return 0;
    }

    compressDataset(testDir, maxError, errorMode, inputErrorMode,
                    extrapolationMethod, inputMethod, debugMode);

    return 0;
}
