#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <deque>
#include <filesystem>
#include <chrono>
namespace fs = std::filesystem;
using namespace std;

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
    heap->arr = new Node *[cap];
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
        throw "Heap reached maximum capacity";
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
    if (!huffmanTree->left && !huffmanTree->right) {
        // Only one unique character
        code[huffmanTree->value] = "0";
        return code;
    }
    generateCode(huffmanTree, "", code);
    return code;
}
Node *generateHuffmanTree(const unordered_map<int, unsigned> &freqMap)
{
    if (freqMap.size() == 0) {
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

vector<int> decode(const string &str, const unordered_map<int, string> &code)
{
    unordered_map<string, int> reverseCode = createReverseLookup(code);
    vector<int> result;
    string buffer;

    for (const char &c : str)
    {
        buffer.push_back(c);
        auto it = reverseCode.find(buffer);
        if (it != reverseCode.end())
        {
            result.push_back(it->second);
            buffer.clear();
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

void compressFile(const string &inputPath, const string &outputPath)
{
    ifstream file(inputPath);
    vector<int> inputInts;
    if (!file)
    {
        cerr << "Failed to open the file.\n";
        return;
    }

    int curInt;
    while (file.read(reinterpret_cast<char *>(&curInt), sizeof(curInt)))
    {
        inputInts.push_back(curInt);
    }

    unordered_map<int, unsigned> freqMap = createFreqMap(inputInts);
    Node *tree = generateHuffmanTree(freqMap);
    unordered_map<int, string> code = getHuffmanCode(tree);

    string encoded = encode(inputInts, code);
    string buffer = serializeTree(tree);
    unsigned bufferSize = buffer.length(); // Get number of bits needed to store the tree
    unsigned long long encodedSize = encoded.length();

    // Write the compressed file
    ofstream out(outputPath, ios::binary);

    out.write(reinterpret_cast<char *>(&bufferSize), sizeof(bufferSize));
    out.write(reinterpret_cast<char *>(&encodedSize), sizeof(encodedSize));

    char byte = 0;
    int pos = 0;
    for (const char &c : buffer)
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

    byte = 0;
    pos = 0;
    for (const char &c : encoded)
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
    out.close();
    // cout << "Compression successful.\n";
    // cout << "Original size: " << inputInts.size() * 4 << " bytes\n";
    // cout << "Buffer size: " << (bufferSize + 7) / 8 << " bytes\n";
    // cout << "Encoded size: " << (encoded.size() + 7) / 8 << " bytes\n";
}

void decompressFile(const string &inputPath, const string &outputPath)
{
    auto startTotal = chrono::high_resolution_clock::now();

    auto start = chrono::high_resolution_clock::now();

    ifstream compressed(inputPath, ios::binary);

    unsigned bufferSize;
    unsigned long long encodedSize;

    compressed.read(reinterpret_cast<char *>(&bufferSize), sizeof(bufferSize));
    compressed.read(reinterpret_cast<char *>(&encodedSize), sizeof(encodedSize));

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Reading buffer and encoded size time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    char byte;
    stringstream bufferStream, encodedStream;
    while (bufferSize >= 8)
    {
        compressed.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < 8; ++pos)
        {
            bool b = byte & (1 << pos);
            bufferStream << (b ? '1' : '0');
        }

        bufferSize -= 8;
    }

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Reading buffer stream time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    if (bufferSize > 0)
    {
        compressed.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < bufferSize; ++pos)
        {
            bool b = byte & (1 << pos);
            bufferStream << (b ? '1' : '0');
        }
    }
    string serializedTree = bufferStream.str();

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Reading remaining buffer stream time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    // Read the encoded string
    while (encodedSize >= 8)
    {
        compressed.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < 8; ++pos)
        {
            bool b = byte & (1 << pos);
            encodedStream << (b ? '1' : '0');
        }

        encodedSize -= 8;
    }

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Reading encoded string time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    if (encodedSize > 0)
    {
        compressed.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < encodedSize; ++pos)
        {
            bool b = byte & (1 << pos);
            encodedStream << (b ? '1' : '0');
        }
    }
    string encodedData = encodedStream.str();
    compressed.close();

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Reading remaining encoded string and closing file time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    Node *deserializedTree = deserializeTree(serializedTree);

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Deserializing tree time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    unordered_map<int, string> deserializedCode = getHuffmanCode(deserializedTree);

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Getting Huffman code time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    vector<int> decodedInts = decode(encodedData, deserializedCode);

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Decoding time: " << duration.count() << " microseconds.\n";

    start = chrono::high_resolution_clock::now();

    ofstream decodedFile(outputPath);
    if (!decodedFile)
    {
        cerr << "Error creating the file.\n";
        return;
    }
    for (int i : decodedInts)
    {
        decodedFile.write(reinterpret_cast<char *>(&i), sizeof(i));
    }
    decodedFile.close();

    end = chrono::high_resolution_clock::now();
    duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Writing to file time: " << duration.count() << " microseconds.\n";

    auto endTotal = chrono::high_resolution_clock::now();
    auto durationTotal = chrono::duration_cast<chrono::microseconds>(endTotal - startTotal);
    cout << "Total execution time: " << durationTotal.count() << " microseconds.\n";
}

bool filesAreSame(const string &a, const string &b)
{
    ifstream file1(a, ios::ate | ios::binary);
    ifstream file2(b, ios::ate | ios::binary);
    const streampos fileSize = file1.tellg();

    if (fileSize != file2.tellg())
    {
        return false;
    }

    file1.seekg(0);
    file2.seekg(0);

    istreambuf_iterator<char> begin1(file1);
    istreambuf_iterator<char> begin2(file2);

    return equal(begin1, istreambuf_iterator<char>(), begin2);
}
size_t getFileSize(const string &filePath)
{
    ifstream in(filePath, ios::ate | ios::binary);
    return in.tellg();
}

int main()
{
    vector<string> testCases;
    const string testDir = "tests";
    for (const auto& entry : fs::directory_iterator(testDir))
    {
        if (entry.path().extension() == ".in")
        {
            testCases.push_back(entry.path().string());
        }
    }

    for (const string &filePath : testCases)
    {
        string compressedPath = filePath + "-compressed.bin";
        string outputPath = filePath + "-decompressed.bin";

        compressFile(filePath, compressedPath);
        decompressFile(compressedPath, outputPath);

        size_t originalSize = getFileSize(filePath);
        size_t compressedSize = getFileSize(compressedPath);

        if (filesAreSame(filePath, outputPath))
        {
            cout << "SUCCESS! Compression was lossless: " << filePath << ".\n";
        }
        else
        {
            cout << "FAIL! Compression was not lossless: " << filePath << ".\n";
        }

        cout << "- Original file size: " << originalSize << " bytes.\n";
        cout << "- Compressed file size: " << compressedSize << " bytes.\n";
    }

    return 0;
}
