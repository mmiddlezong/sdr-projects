#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <deque>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <limits>

using namespace std;
namespace fs = filesystem;

struct Node
{
    unsigned freq;
    int value;
    Node *left, *right;
};

struct MinHeap
{
    Node **arr; // Array of node pointers
    unsigned capacity;
    unsigned size;
};

MinHeap *createMinHeap(unsigned cap)
{
    MinHeap *heap = new MinHeap();
    heap->capacity = cap;
    heap->size = 0;
    heap->arr = new Node *[cap + 1]; // Keep an empty value at 0
    return heap;
}

void swapNodes(Node **a, Node **b)
{
    Node *t = *a;
    *a = *b;
    *b = t;
}

void enqueue(MinHeap *heap, Node *node)
{
    if (heap->size >= heap->capacity)
    {
        cerr << "Heap reached maximum capacity\n";
        return;
    }
    ++heap->size;
    unsigned i = heap->size;
    heap->arr[i] = node;
    while (i > 1)
    {
        unsigned parent = i / 2;
        if (heap->arr[parent]->freq > heap->arr[i]->freq)
        {
            swapNodes(&heap->arr[parent], &heap->arr[i]);
            i = parent;
        }
        else
        {
            break;
        }
    }
}

Node *dequeue(MinHeap *heap)
{
    if (heap->size <= 0)
    {
        return NULL;
    }
    Node *min = heap->arr[1];
    heap->arr[1] = heap->arr[heap->size];
    --heap->size;
    unsigned i = 1;
    while (2 * i <= heap->size)
    {
        if (2 * i + 1 <= heap->size && heap->arr[2 * i + 1]->freq < heap->arr[i]->freq && heap->arr[2 * i + 1]->freq < heap->arr[2 * i]->freq)
        {
            // Right child is more urgent
            swapNodes(&heap->arr[i], &heap->arr[2 * i + 1]);
            i = 2 * i + 1;
        }
        else if (heap->arr[2 * i]->freq < heap->arr[i]->freq)
        {
            // Left child is more urgent
            swapNodes(&heap->arr[i], &heap->arr[2 * i]);
            i *= 2;
        }
        else
        {
            break;
        }
    }

    return min;
}

Node *createNode(int value, unsigned freq)
{
    Node *node = new Node();
    node->value = value;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

void generateCode(Node *cur, string path, unordered_map<int, string> &code)
{
    if (cur->left)
    {
        generateCode(cur->left, path + '0', code);
    }
    if (cur->right)
    {
        generateCode(cur->right, path + '1', code);
    }
    if (!cur->left && !cur->right)
    {
        code[cur->value] = path;
    }
}

unordered_map<int, string> getHuffmanCode(Node *huffmanTree)
{
    unordered_map<int, string> code;
    if (!huffmanTree->left && !huffmanTree->right)
    {
        // Only one unique character
        code[huffmanTree->value] = "0";
        return code;
    }
    generateCode(huffmanTree, "", code);
    return code;
}

Node *generateHuffmanTree(const unordered_map<int, unsigned> &freqMap)
{
    if (freqMap.size() == 0)
    {
        cerr << "freqMap has size 0\n";
        return NULL;
    }

    MinHeap *heap = createMinHeap(freqMap.size());

    for (auto i : freqMap)
    {
        enqueue(heap, createNode(i.first, i.second));
    }

    for (int i = 0; i < freqMap.size() - 1; i++)
    {
        Node *first = dequeue(heap);
        Node *second = dequeue(heap);
        Node *newNode = createNode(0, first->freq + second->freq);
        newNode->left = (first->freq < second->freq) ? first : second;
        newNode->right = (first->freq < second->freq) ? second : first;

        enqueue(heap, newNode);
    }

    Node *huffmanTree = heap->arr[1];
    return huffmanTree;
}

unordered_map<int, unsigned> createFreqMap(const vector<int> &vec)
{
    unordered_map<int, unsigned> freqMap;
    for (int i : vec)
    {
        freqMap[i]++;
    }
    return freqMap;
}

string encode(const vector<int> vec, const unordered_map<int, string> &code)
{
    string res;
    for (const int &i : vec)
    {
        res += code.at(i);
    }
    return res;
}

unordered_map<string, int> createReverseLookup(const unordered_map<int, string> &code)
{
    unordered_map<string, int> reverseCode;

    for (const auto &pair : code)
    {
        reverseCode[pair.second] = pair.first;
    }

    return reverseCode;
}

vector<int> decode(const string &str, Node *huffmanTree)
{
    vector<int> result;

    if (!huffmanTree->left && !huffmanTree->right)
    {
        // Only one unique character
        for (const char &c : str)
        {
            result.push_back(huffmanTree->value);
        }
        return result;
    }

    Node *root = huffmanTree;
    Node *node = root;

    for (const char &c : str)
    {
        if (c == '0')
        {
            node = node->left;
        }
        else
        {
            node = node->right;
        }
        if (!node->left && !node->right)
        {
            result.push_back(node->value);
            node = root;
        }
    }

    return result;
}

void traverseTree(Node *cur, string &out)
{
    if (!cur->left && !cur->right)
    {
        out.push_back('1');
        int c = cur->value;
        const int numBits = sizeof(c) * 8;
        for (int i = numBits - 1; i >= 0; --i)
        {
            bool bit = (c >> i) & 1;
            out.push_back(bit ? '1' : '0');
        }
    }
    else
    {
        out.push_back('0');
        traverseTree(cur->left, out);
        traverseTree(cur->right, out);
    }
}

string serializeTree(Node *tree)
{
    string out;
    traverseTree(tree, out);
    return out;
}

Node *deserialize(const string &serializedTree, int &i)
{
    while (i < serializedTree.length())
    {
        // Read bit
        bool b = serializedTree.at(i) == '1';

        if (b)
        {
            int cur = 0;
            int numBits = sizeof(cur) * 8;
            for (int pos = numBits - 1; pos >= 0; pos--)
            {
                ++i;
                b = serializedTree.at(i) == '1';
                if (b)
                {
                    cur |= (1 << pos);
                }
            }
            Node *newNode = createNode(cur, 0);
            return newNode;
        }
        else
        {
            Node *newNode = createNode(0, 0);
            newNode->left = deserialize(serializedTree, ++i);
            newNode->right = deserialize(serializedTree, ++i);
            return newNode;
        }
    }
    return NULL;
}

Node *deserializeTree(const string &serializedTree)
{
    int i = 0;
    return deserialize(serializedTree, i);
}

void writeBitsToFile(ofstream &out, const string &bits)
{
    char byte = 0;
    int pos = 0;
    for (const char &c : bits)
    {
        bool b = c == '1';
        if (b)
        {
            byte |= (1 << pos);
        }
        pos++;
        if (pos == 8)
        {
            out.write(&byte, sizeof(byte));
            byte = 0;
            pos = 0;
        }
    }
    if (pos > 0)
    {
        out.write(&byte, sizeof(byte));
    }
}

string readBitsIntoString(ifstream &file, unsigned long long bits)
{
    char byte;
    stringstream stream;
    while (bits >= 8)
    {
        file.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < 8; ++pos)
        {
            bool b = byte & (1 << pos);
            stream << (b ? '1' : '0');
        }

        bits -= 8;
    }
    if (bits > 0)
    {
        file.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < bits; ++pos)
        {
            bool b = byte & (1 << pos);
            stream << (b ? '1' : '0');
        }
    }
    return stream.str();
}

void compressFile(const string &inputPath, const string &outputPath, const float &maxError)
{
    ifstream file(inputPath);
    vector<float> inputFloats;
    if (!file)
    {
        cerr << "Failed to open the file.\n";
        return;
    }

    float curFloat;
    while (file.read(reinterpret_cast<char *>(&curFloat), sizeof(curFloat)))
    {
        inputFloats.push_back(curFloat);
    }

    const long long n = inputFloats.size();
    if (n < 2)
    {
        cerr << "File contains fewer than two data points.\n";
        return;
    }

    vector<float> extrapolateErrors(n - 2); // Size n-2
    vector<float> lossyData(n);             // Size n
    lossyData[0] = inputFloats[0];
    lossyData[1] = inputFloats[1];
    // Extrapolation step
    for (size_t i = 2; i < inputFloats.size(); ++i)
    {
        const float extrapolatedFloat = 2 * lossyData[i - 1] - lossyData[i - 2];
        const float err = inputFloats[i] - extrapolatedFloat;
        extrapolateErrors[i - 2] = err;

        // Figure out what err would quantize to
        float quantizedErr = round(err / (2 * maxError)) * 2 * maxError;
        lossyData[i] = extrapolatedFloat + quantizedErr;
    }

    vector<int> inputInts; // Size n-2
    // Split into buckets of size 2 * maxError, where bucket 0 is centered at 0
    for (const auto &err : extrapolateErrors)
    {
        int bucket = round(err / (2 * maxError));
        inputInts.push_back(bucket);
    }

    unordered_map<int, unsigned> freqMap = createFreqMap(inputInts);
    Node *tree = generateHuffmanTree(freqMap);
    unordered_map<int, string> code = getHuffmanCode(tree);

    string encoded = encode(inputInts, code);
    string buffer = serializeTree(tree);
    unsigned bufferSize = buffer.length(); // Get number of bits needed to store the tree
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

void decompressFile(const string &inputPath, const string &outputPath)
{
    auto startTotal = chrono::high_resolution_clock::now();
    ifstream compressed(inputPath, ios::binary);

    if (!compressed.is_open())
    {
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
    if (!decodedFile)
    {
        cerr << "Error creating the file.\n";
        return;
    }

    // Reconstruction step
    for (size_t i = 0; i < decodedInts.size(); ++i)
    {
        // Convert the bucket number back to float
        const float decodedErr = decodedInts[i] * 2 * maxError;

        // Extrapolate the new data point and adjust for error
        const float extrapolatedFloat = 2 * reconstructedData[i + 1] - reconstructedData[i];
        const float reconstructedFloat = extrapolatedFloat + decodedErr;
        reconstructedData.push_back(reconstructedFloat);
    }

    // Write reconstructed data to file
    for (const auto &f : reconstructedData)
    {
        decodedFile.write(reinterpret_cast<const char *>(&f), sizeof(f));
    }
    decodedFile.close();
    auto endTotal = chrono::high_resolution_clock::now();
    auto durationTotal = chrono::duration_cast<chrono::microseconds>(endTotal - startTotal);
    // cout << "Decoding time: " << durationTotal.count() << " microseconds.\n";
}

size_t getFileSize(const string &filePath)
{
    ifstream in(filePath, ios::ate | ios::binary);
    if (!in.is_open())
    {
        cerr << "File could not be opened.\n";
        return 0;
    }
    return in.tellg();
}

void outputErrors(const string &filePath1, const string &filePath2, const string &outputPath)
{
    ifstream file1(filePath1, ios::binary);
    ifstream file2(filePath2, ios::binary);

    if (!file1.is_open() || !file2.is_open())
    {
        throw runtime_error("Files could not be opened.");
    }

    float v1, v2;
    ofstream out(outputPath);
    if (!out)
    {
        throw runtime_error("Invalid output path.");
    }

    while (file1.read(reinterpret_cast<char *>(&v1), sizeof(v1)))
    {
        file2.read(reinterpret_cast<char *>(&v2), sizeof(v2));
        float curError = abs(v2 - v1);
        out << curError << "\n";
    }

    file1.close();
    file2.close();
    out.close();
}

pair<float, float> getMaxAndAvgError(const string &filePath1, const string &filePath2)
{
    ifstream file1(filePath1, ios::binary);
    ifstream file2(filePath2, ios::binary);

    if (!file1.is_open() || !file2.is_open())
    {
        throw runtime_error("Files could not be opened.");
    }

    float v1, v2;
    float maxError = 0.0f;
    int count = 0;
    float sumError = 0.0f;
    while (file1.read(reinterpret_cast<char *>(&v1), sizeof(v1)))
    {
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

int main()
{
    float maxError = 0.05f;
    vector<string> testCases;
    const string testDir = "continuous-tests";
    for (const auto &entry : fs::directory_iterator(testDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".in")
        {
            testCases.push_back(entry.path().string());
        }
    }

    for (string filePath : testCases)
    {
        string compressedPath = filePath + "-compressed.bin";
        string outputPath = filePath + "-decompressed.bin";

        auto c0 = chrono::high_resolution_clock::now();
        compressFile(filePath, compressedPath, maxError);
        auto c1 = chrono::high_resolution_clock::now();
        decompressFile(compressedPath, outputPath);
        auto c2 = chrono::high_resolution_clock::now();

        // Get time in ms
        auto compressionTime = chrono::duration_cast<chrono::microseconds>(c1 - c0).count() / 1000.0f;
        auto decompressionTime = chrono::duration_cast<chrono::microseconds>(c2 - c1).count() / 1000.0f;

        size_t originalSize = getFileSize(filePath);
        size_t compressedSize = getFileSize(compressedPath);

        // outputErrors(filePath, outputPath, filePath + "-errors.txt");

        pair<float, float> maxAndAvgError = getMaxAndAvgError(filePath, outputPath);
        float curMaxError = maxAndAvgError.first;
        float curAvgError = maxAndAvgError.second;

        cout << "Compressed " << filePath << ":\n";
        cout << "- Max error: " << curMaxError << "\n";
        cout << "- Avg error: " << curAvgError << "\n";
        cout << "- Original file size: " << originalSize << " B\n";
        cout << "- Compressed file size: " << compressedSize << " B\n";
        cout << "- Compression..." << "\n";
        cout << "-   ratio: " << (float) originalSize / compressedSize << "\n";
        cout << "-   time: " << compressionTime << " ms\n";
        cout << "-   throughput: " << (float) originalSize / compressionTime << " KB/s\n";
        cout << "- Decompression..." << "\n";
        cout << "-   time: " << decompressionTime << " ms\n";
        cout << "-   throughput: " << (float) originalSize / decompressionTime << " KB/s\n";
    }

    return 0;
}
