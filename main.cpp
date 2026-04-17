#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

// === AVL NODE ===
struct Node {
    int key, height;
    Node* left;
    Node* right;

    Node(int k) {
        key = k;
        height = 1;
        left = nullptr;
        right = nullptr;
    }
};

// === HELPERS ===
int getHeight(Node* root) {
    if (root == nullptr) return 0;
    return root->height;
}

int getBalance(Node* root) {
    if (root == nullptr) return 0;
    return getHeight(root->left) - getHeight(root->right);
}

void updateHeight(Node* root) {
    if (root != nullptr) {
        root->height = max(getHeight(root->left), getHeight(root->right)) + 1;
    }
}

Node* minValueNode(Node* root) {
    Node* cur = root;
    while (cur->left != nullptr) cur = cur->left;
    return cur;
}

// === ROTATIONS ===
Node* rotateRight(Node* y) {
    Node* x = y->left;
    Node* t2 = x->right;

    x->right = y;
    y->left = t2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

Node* rotateLeft(Node* x) {
    Node* y = x->right;
    Node* t2 = y->left;

    y->left = x;
    x->right = t2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

// === INSERT ===
Node* insert(Node* root, int key) {
    if (root == nullptr) return new Node(key);

    if (key < root->key) {
        root->left = insert(root->left, key);
    } else if (key > root->key) {
        root->right = insert(root->right, key);
    } else {
        return root;
    }

    updateHeight(root);
    int balance = getBalance(root);

    if (balance > 1 && key < root->left->key) return rotateRight(root);
    if (balance < -1 && key > root->right->key) return rotateLeft(root);

    if (balance > 1 && key > root->left->key) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (balance < -1 && key < root->right->key) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

// === DELETE ===
Node* deleteNode(Node* root, int key) {
    if (root == nullptr) return root;

    if (key < root->key) {
        root->left = deleteNode(root->left, key);
    } else if (key > root->key) {
        root->right = deleteNode(root->right, key);
    } else {
        if (root->left == nullptr || root->right == nullptr) {
            Node* temp;
            if (root->left != nullptr) temp = root->left;
            else temp = root->right;

            if (temp == nullptr) {
                delete root;
                return nullptr;
            } else {
                Node* oldRoot = root;
                root = temp;
                delete oldRoot;
            }
        } else {
            Node* temp = minValueNode(root->right);
            root->key = temp->key;
            root->right = deleteNode(root->right, temp->key);
        }
    }

    if (root == nullptr) return root;

    updateHeight(root);
    int balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0) return rotateRight(root);

    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (balance < -1 && getBalance(root->right) <= 0) return rotateLeft(root);

    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

// === TRAVERSALS ===
void preHelper(Node* root, vector<int>& ans) {
    if (root == nullptr) return;
    ans.push_back(root->key);
    preHelper(root->left, ans);
    preHelper(root->right, ans);
}

void inHelper(Node* root, vector<int>& ans) {
    if (root == nullptr) return;
    inHelper(root->left, ans);
    ans.push_back(root->key);
    inHelper(root->right, ans);
}

void postHelper(Node* root, vector<int>& ans) {
    if (root == nullptr) return;
    postHelper(root->left, ans);
    postHelper(root->right, ans);
    ans.push_back(root->key);
}

vector<int> preOrder(Node* root) {
    vector<int> ans;
    preHelper(root, ans);
    return ans;
}

vector<int> inOrder(Node* root) {
    vector<int> ans;
    inHelper(root, ans);
    return ans;
}

vector<int> postOrder(Node* root) {
    vector<int> ans;
    postHelper(root, ans);
    return ans;
}

// === CLEANUP ===
void freeTree(Node* root) {
    if (root == nullptr) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

// === MAIN ===
int main() {
    string line, s;
    getline(cin, line);
    stringstream ss(line);

    vector<string> tokens;
    while (ss >> s) tokens.push_back(s);

    Node* root = nullptr;
    string mode = tokens.back();

    for (int i = 0; i < (int)tokens.size() - 1; i++) {
        char op = tokens[i][0];
        int x = stoi(tokens[i].substr(1));

        if (op == 'A') root = insert(root, x);
        else if (op == 'D') root = deleteNode(root, x);
    }

    vector<int> ans;
    if (mode == "PRE") ans = preOrder(root);
    else if (mode == "IN") ans = inOrder(root);
    else ans = postOrder(root);

    if (ans.empty()) {
        cout << "EMPTY";
    } else {
        for (int i = 0; i < (int)ans.size(); i++) {
            if (i > 0) cout << " ";
            cout << ans[i];
        }
    }

    freeTree(root);
    return 0;
}