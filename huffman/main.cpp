#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <deque>
using namespace std;

struct Node
{
    unsigned freq;
    char value;
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

Node *createNode(char value, unsigned freq)
{
    Node *node = new Node();
    node->value = value;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}
void generateCode(Node *cur, string path, unordered_map<char, string> &code)
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

unordered_map<char, string> getHuffmanCode(Node *huffmanTree)
{
    unordered_map<char, string> code;
    generateCode(huffmanTree, "", code);
    return code;
}
Node *generateHuffmanTree(const unordered_map<char, unsigned> &freqMap)
{
    MinHeap *heap = createMinHeap(freqMap.size());
    for (auto i : freqMap)
    {
        enqueue(heap, createNode(i.first, i.second));
    }
    for (int i = 0; i < freqMap.size() - 1; i++)
    {
        Node *first = dequeue(heap);
        Node *second = dequeue(heap);
        Node *newNode = createNode('$', first->freq + second->freq);
        newNode->left = (first->freq < second->freq) ? first : second;
        newNode->right = (first->freq < second->freq) ? second : first;

        enqueue(heap, newNode);
    }

    Node *huffmanTree = heap->arr[1];
    return huffmanTree;
}

unordered_map<char, unsigned> createFreqMap(const string &str)
{
    unordered_map<char, unsigned> freqMap;
    for (char c : str)
    {
        freqMap[c]++;
    }
    return freqMap;
}

string encode(const string &str, const unordered_map<char, string> &code)
{
    string res;
    for (const char &c : str)
    {
        res += code.at(c);
    }
    return res;
}

unordered_map<string, char> createReverseLookup(const unordered_map<char, string> &code)
{
    unordered_map<string, char> reverseCode;

    for (const auto &pair : code)
    {
        reverseCode[pair.second] = pair.first;
    }

    return reverseCode;
}

string decode(const string &str, const unordered_map<char, string> &code)
{
    unordered_map<string, char> reverseCode = createReverseLookup(code);
    stringstream result;
    string buffer;

    for (const char &c : str)
    {
        buffer += c;
        auto it = reverseCode.find(buffer);
        if (it != reverseCode.end())
        {
            result << it->second;
            buffer.clear();
        }
    }

    return result.str();
}

void traverseTree(Node *cur, string &out)
{
    if (!cur->left && !cur->right)
    {
        out += '1';
        char c = cur->value;
        for (int i = 7; i >= 0; --i)
        {
            bool bit = (c >> i) & 1;
            out += bit ? '1' : '0';
        }
    }
    else
    {
        out += '0';
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
            char byte = 0;
            for (int pos = 7; pos >= 0; pos--)
            {
                ++i;
                b = serializedTree.at(i) == '1';
                if (b)
                {
                    byte |= (1 << pos);
                }
            }
            Node *newNode = createNode(byte, 0);
            return newNode;
        }
        else
        {
            Node *newNode = createNode('$', 0);
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
    string inputString;
    if (file.is_open())
    {
        stringstream buffer;
        buffer << file.rdbuf();
        inputString = buffer.str();
        file.close();
    }
    else
    {
        cout << "Failed to open the file.\n";
        return;
    }

    unordered_map<char, unsigned> freqMap = createFreqMap(inputString);
    Node *tree = generateHuffmanTree(freqMap);
    unordered_map<char, string> code = getHuffmanCode(tree);

    string encoded = encode(inputString, code);
    string buffer = serializeTree(tree);
    unsigned int bufferSize = buffer.length(); // Get number of bits needed to store the tree
    unsigned int encodedSize = encoded.length();

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
    cout << "Compression successful.\n";
    cout << "Original size: " << inputString.length() << " bytes\n";
    cout << "Buffer size: " << (bufferSize + 7) / 8 << " bytes\n";
    cout << "Encoded size: " << (encoded.size() + 7) / 8 << " bytes\n";
}

void decompressFile(const string &inputPath, const string &outputPath)
{
    ifstream compressed(inputPath, ios::binary);

    unsigned int bufferSize;
    unsigned int encodedSize;

    compressed.read(reinterpret_cast<char *>(&bufferSize), sizeof(bufferSize));
    compressed.read(reinterpret_cast<char *>(&encodedSize), sizeof(encodedSize));

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

    Node *deserializedTree = deserializeTree(serializedTree);
    unordered_map<char, string> deserializedCode = getHuffmanCode(deserializedTree);
    string decodedString = decode(encodedData, deserializedCode);
    ofstream decodedFile(outputPath);
    decodedFile << decodedString;
    decodedFile.close();

    cout << "Decompression successful.\n";
}

int main()
{
    compressFile("input.txt", "compressed.bin");
    decompressFile("compressed.bin", "decoded.txt");

    return 0;
}
