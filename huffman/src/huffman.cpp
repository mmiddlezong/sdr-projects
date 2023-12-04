#include "huffman.h"

Node *createNode(int value, unsigned freq) {
    Node *node = new Node();
    node->value = value;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

void generateCode(Node *cur, string path, unordered_map<int, string> &code) {
    if (cur->left) {
        generateCode(cur->left, path + '0', code);
    }
    if (cur->right) {
        generateCode(cur->right, path + '1', code);
    }
    if (!cur->left && !cur->right) {
        code[cur->value] = path;
    }
}

unordered_map<int, string> getHuffmanCode(Node *huffmanTree) {
    unordered_map<int, string> code;
    if (!huffmanTree->left && !huffmanTree->right) {
        // Only one unique character
        code[huffmanTree->value] = "0";
        return code;
    }
    generateCode(huffmanTree, "", code);
    return code;
}

bool CompareNode::operator()(const Node *lhs, const Node *rhs) const {
    return lhs->freq > rhs->freq;
}

Node *generateHuffmanTree(const unordered_map<int, unsigned> &freqMap) {
    if (freqMap.empty()) {
        cerr << "freqMap has size 0\n";
        return nullptr;
    }

    priority_queue<Node *, vector<Node *>, CompareNode> heap;

    for (const auto &pair : freqMap) {
        heap.push(createNode(pair.first, pair.second));
    }

    while (heap.size() > 1) {
        Node *first = heap.top();
        heap.pop();
        Node *second = heap.top();
        heap.pop();

        Node *newNode = createNode(0, first->freq + second->freq);
        newNode->left = first;
        newNode->right = second;

        heap.push(newNode);
    }

    return heap.empty() ? nullptr : heap.top();
}

string encode(const vector<int> &vec, const unordered_map<int, string> &code) {
    string res;
    for (const int &i : vec) {
        res += code.at(i);
    }
    return res;
}

vector<int> decode(const string &str, Node *huffmanTree) {
    vector<int> result;

    if (!huffmanTree->left && !huffmanTree->right) {
        // Only one unique character
        for (const char &c : str) {
            result.push_back(huffmanTree->value);
        }
        return result;
    }

    Node *root = huffmanTree;
    Node *node = root;

    for (const char &c : str) {
        if (c == '0') {
            node = node->left;
        } else {
            node = node->right;
        }
        if (!node->left && !node->right) {
            result.push_back(node->value);
            node = root;
        }
    }

    return result;
}

void traverseTree(Node *cur, string &out) {
    if (!cur->left && !cur->right) {
        out.push_back('1');
        int c = cur->value;
        const int numBits = sizeof(c) * 8;
        for (int i = numBits - 1; i >= 0; --i) {
            bool bit = (c >> i) & 1;
            out.push_back(bit ? '1' : '0');
        }
    } else {
        out.push_back('0');
        traverseTree(cur->left, out);
        traverseTree(cur->right, out);
    }
}

string serializeTree(Node *tree) {
    string out;
    traverseTree(tree, out);
    return out;
}

Node *deserialize(const string &serializedTree, int &i) {
    while (i < serializedTree.length()) {
        // Read bit
        bool b = serializedTree.at(i) == '1';

        if (b) {
            int cur = 0;
            int numBits = sizeof(cur) * 8;
            for (int pos = numBits - 1; pos >= 0; pos--) {
                ++i;
                b = serializedTree.at(i) == '1';
                if (b) {
                    cur |= (1 << pos);
                }
            }
            Node *newNode = createNode(cur, 0);
            return newNode;
        } else {
            Node *newNode = createNode(0, 0);
            newNode->left = deserialize(serializedTree, ++i);
            newNode->right = deserialize(serializedTree, ++i);
            return newNode;
        }
    }
    return NULL;
}

Node *deserializeTree(const string &serializedTree) {
    int i = 0;
    return deserialize(serializedTree, i);
}

void writeBitsToFile(ofstream &out, const string &bits) {
    unsigned long long fileSize = (bits.size() + 7) / 8;
    char *bytesToWrite = new char[fileSize];

    char byte = 0;
    int pos = 0;
    size_t byteIndex = 0;
    for (char c : bits) {
        if (c == '1') {
            byte |= (1 << pos);
        }
        if (++pos == 8) {
            bytesToWrite[byteIndex++] = byte;
            byte = 0;
            pos = 0;
        }
    }
    if (pos > 0) { // Handle the last byte
        bytesToWrite[byteIndex] = byte;
    }

    out.write(bytesToWrite, sizeof(byte) * fileSize);
    delete[] bytesToWrite;
}

string readBitsIntoString(ifstream &file, unsigned long long bits) {
    char byte;
    stringstream stream;
    while (bits >= 8) {
        file.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < 8; ++pos) {
            bool b = byte & (1 << pos);
            stream << (b ? '1' : '0');
        }

        bits -= 8;
    }
    if (bits > 0) {
        file.read(&byte, sizeof(byte));

        // Add this to the string
        for (int pos = 0; pos < bits; ++pos) {
            bool b = byte & (1 << pos);
            stream << (b ? '1' : '0');
        }
    }
    return stream.str();
}