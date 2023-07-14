#include <iostream>
#include <unordered_map>
#include <sstream>
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

void *enqueue(MinHeap *heap, Node *node)
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
void dfs(Node *cur, string path, unordered_map<char, string> &code)
{
    if (cur->left)
    {
        dfs(cur->left, path + "0", code);
    }
    if (cur->right)
    {
        dfs(cur->right, path + "1", code);
    }
    if (!cur->left && !cur->right)
    {
        code[cur->value] = path;
    }
}
unordered_map<char, string> getHuffmanCode(unordered_map<char, unsigned> freqMap)
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
    delete heap;
    unordered_map<char, string> code;
    dfs(huffmanTree, "", code);

    return code;
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
    stringstream res;
    for (const char &c : str)
    {
        res << code.at(c);
    }
    return res.str();
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

int main()
{
    string str = "349s5n8vbns43598vs49n8uovn49u8sovn9s8uo4vs89uno8u9onsvenu9ov8raw8uonwrecanuiohaewrwycauicaeyugbdcsafgvdsghsdghvcffghvcfghvdgfdsfcgvdscfghvaecfghvaecfwhgbfcadhbjdchjbdfchbfhgbdsfchjbdhjgbcdhj";
    unordered_map<char, unsigned> freqMap = createFreqMap(str);
    unordered_map<char, string> code = getHuffmanCode(freqMap);
    for (auto i : code)
    {
        cout << i.first << " " << i.second << "\n";
    }
    string encoded = encode(str, code);
    cout << "Encoded string: " << encoded << "\n";
    cout << "Encoded size: " << encoded.length() / 8 << " bytes\n";
    cout << "Decoded string: " << decode(encoded, code) << "\n";
    cout << "Decoded size: " << str.length() << " bytes\n";
    return 0;
}
