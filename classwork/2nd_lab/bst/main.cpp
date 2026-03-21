#include <iostream>
#include <vector>

class BST {
    struct TNode {
        int val;
        TNode* left;
        TNode* right;
        TNode(int val) : val(val), left(nullptr), right(nullptr) {}
    };

public:
    BST(): root(nullptr) {}

    TNode* search(int val) {
        return find(root, val);
    }

    void insert(int val) {
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
        if (cur == nullptr)
            return nullptr;
        
        int cur_val = cur->val;

        if (cur_val == val)
            return cur;
        if (cur_val <= val)
            return find(cur->left, val);
    
        return find(cur->right, val);
    }

    TNode* insert(TNode* cur, int val) {
        if (cur == nullptr)
            return new TNode(val);
        
        int cur_val = cur->val;
        if (cur_val <= val)
            cur->left = insert(cur->left, val);
        else if (cur_val > val)
            cur->right = insert(cur->right, val);
        
        return cur;
    }

    TNode* remove(TNode* cur, int val) {
        int cur_val = cur->val;
        auto left = cur->left;
        auto right = cur->right;
            
        if (val <= cur_val)
            return remove(left, val);
        if (val > cur_val)
            return remove(right, val);

        if (!right && !left) {
            delete cur;
            return nullptr;
        }
        if (right) {
            TNode* temp = right;
            delete cur;
            return temp;
        }
        if (left) {
            TNode* temp = left;
            delete cur;
            return temp;
        }
        
        TNode* temp = minRight(right);
        cur->val = temp->val;
        cur->right = remove(cur->right, val);

        return cur;
    }

    TNode* minRight(TNode* cur) {
        while (cur && cur->left)
            cur = cur->left;

        return cur;
    }

    void inorder(TNode* cur) {
        if (cur == nullptr)
            return;

        inorder(cur->left);
        std::cout << cur->val << " ";
        inorder(cur->right);
    }

    TNode* rightRotate(TNode* y) {
        if (y == nullptr)
            return nullptr;
        
        auto x = y->left;
        auto beta = x->right;
        y->left = beta;
        x->right = y;
        
        return x;
    }

    TNode* leftRotate(TNode* x) {
        if (x == nullptr)
            return nullptr;
        
        auto y = x->right;
        auto beta = y->left;
        x->right = beta;
        y->left = x;

        return y;
    }
};

int main() {
    BST tree = BST();

    std::vector<int> to_insert = {3, 14, 15, 9, 2, 6, 5, 35};
    for (auto val : to_insert)
        tree.insert(val);
    tree.inorder();

    std::vector<int> to_delete = {5, 9, 14, 2};
    for (auto val : to_delete)
        tree.remove(val);
    tree.inorder();
}
