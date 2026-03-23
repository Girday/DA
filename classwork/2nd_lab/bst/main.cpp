#include <iostream>
#include <vector>
#include <algorithm>

class BST {
    struct TNode {
        int val;
        TNode* left;
        TNode* right;
        TNode(int val): val(val), left(nullptr), right(nullptr){}
    };

public:
    BST(): root(nullptr){}

    TNode* search(int val) {
        return find(root, val);
    }

    void insert(int val){
        root = insert(root, val);
    }

    void remove(int val) {
        root = remove(root, val);
    }

    void inorder() {
        inorder(root);
    }
private:
    TNode* root;

    TNode* find(TNode* cur, int val) {
        if (cur == nullptr) {
            return nullptr;
        }

        if (cur->val == val) {
            return cur;
        }
        if (val < cur->val) {
            return find(cur->left, val);
        }

        return find(cur->right, val);
    }

    TNode* insert(TNode* cur, int val) {
        if (cur == nullptr) {
            return new TNode(val);
        }

        if (val < cur->val) {
            cur->left = insert(cur->left, val);
        } else {
            cur->right = insert(cur->right, val);
        }

        return cur;
    }

    TNode* remove(TNode* cur, int val) {
        if (cur == nullptr) {
            return nullptr;
        }

        if (val < cur->val) {
            cur->left = remove(cur->left, val);
            return cur;
        }

        if (val > cur->val) {
            cur->right = remove(cur->right, val);
            return cur;
        }

        if (cur->left == nullptr && cur->right == nullptr) {
            delete cur;
            return nullptr;
        }

        if (cur->left == nullptr) {
            auto next = cur->right;
            delete cur;
            return next;
        }

        if (cur->right == nullptr) {
            auto next = cur->left;
            delete cur;
            return next;
        }

        auto next = minRight(cur->right);
        cur->val = next->val;
        cur->right = remove(cur->right, next->val);

        return cur;
    }

    TNode* minRight(TNode* cur) {
        while (cur && cur->left) {
            cur = cur->left;
        }

        return cur;
    }

    void inorder(TNode* cur) {
        if (cur == nullptr) {
            return;
        }

        inorder(cur->left);
        std::cout << cur->val << " ";
        inorder(cur->right);
    }

    TNode* rightRotate(TNode* y) {
        if (y == nullptr) {
            return nullptr;
        }

        auto x = y->left;
        auto beta = x->right;
        y->left = beta;
        x->right = y;
        return x;
    }

    TNode* rotateLeft(TNode* x) {
        if (x == nullptr) {
            return nullptr;
        }

        auto y = x->right;
        auto beta = y->left;
        x->right = beta;
        y->left = x;

        return y;
    }
};

int main() {
    BST tree = BST();
    std::vector<int> insert = {3, 14, 15, 9, 2, 6, 35};

    for (auto val : insert) {
        tree.insert(val);
    }
    tree.inorder();

    std::vector<int> toDelete = {15, 3, 12, 1, 6};
    for (auto val : toDelete) {
        tree.remove(val);
    }

    tree.inorder();
}