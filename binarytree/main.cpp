#include <iostream>
using namespace std;

class BinaryTree
{
private:
    struct Node
    {
        int data;
        Node *left;
        Node *right;
    };

    Node *root;

    Node *getNewNode(int data)
    {
        Node *node = new Node();
        node->data = data;
        node->left = node->right = nullptr;
        return node;
    }

    Node *insertNode(Node *current, int data)
    {
        if (current == nullptr)
        {
            current = getNewNode(data);
        }
        else if (data <= current->data)
        {
            current->left = insertNode(current->left, data);
        }
        else
        {
            current->right = insertNode(current->right, data);
        }
        return current;
    }

    bool containsNode(Node *current, int data)
    {
        if (current == nullptr)
            return false;
        if (current->data == data)
            return true;
        if (data <= current->data)
            return containsNode(current->left, data);
        return containsNode(current->right, data);
    }

    int findLowest(Node *current)
    {
        if (current == nullptr)
            return -1;
        if (current->left == nullptr)
            return current->data;
        return findLowest(current->left);
    }

public:
    BinaryTree() : root(nullptr) {}

    void insert(int data)
    {
        root = insertNode(root, data);
    }

    bool contains(int data)
    {
        return containsNode(root, data);
    }

    int lowest()
    {
        return findLowest(root);
    }
};

int main()
{
    BinaryTree tree;

    tree.insert(15);
    tree.insert(20);
    tree.insert(10);
    tree.insert(8);
    tree.insert(12);

    cout << "Lowest: " << tree.lowest() << "\n";

    int n;
    cout << "Input a number:\n";
    cin >> n;
    if (tree.contains(n))
        cout << "Number was found in the tree\n";
    else
        cout << "Number was not found in the tree\n";
}
