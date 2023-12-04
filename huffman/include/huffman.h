#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

// Structure for the Huffman tree nodes
struct Node {
    unsigned freq;
    int value;
    Node *left, *right;
};

// Function prototypes
Node *createNode(int value, unsigned freq);
void generateCode(Node *cur, string path, unordered_map<int, string> &code);
unordered_map<int, string> getHuffmanCode(Node *huffmanTree);
Node *generateHuffmanTree(const unordered_map<int, unsigned> &freqMap);
string encode(const vector<int> &vec, const unordered_map<int, string> &code);
vector<int> decode(const string &str, Node *huffmanTree);
void traverseTree(Node *cur, string &out);
string serializeTree(Node *tree);
Node *deserialize(const string &serializedTree, int &i);
Node *deserializeTree(const string &serializedTree);
void writeBitsToFile(ofstream &out, const string &bits);
string readBitsIntoString(ifstream &file, unsigned long long bits);

// Structure for comparing nodes in the priority queue
struct CompareNode {
    bool operator()(const Node *lhs, const Node *rhs) const;
};

#endif // HUFFMAN_H
