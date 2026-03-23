#include <iostream>
#include <vector>

class Treap {
    struct TNode {
        int val;
        TNode* left;
        TNode* right;
        int priority;
        TNode(int val): val(val), priority(::rand()), left(nullptr), right(nullptr){}
    };

public:
    Treap(): root(nullptr){}

    TNode* search(int val) {
        return find(root, val);
    }

    void insert(int val){
        TNode* t1;
        TNode* t2;

        split(root, val, t1, t2);
        t1 = merge(t1, new TNode(val));
        root = merge(t1, t2);
    }

    void remove(int val) {
        TNode* t1;
        TNode* t2;
        TNode* toDelete;

        split(root, val, t1, t2);
        split(t1, val-1, t1, toDelete);
        delete toDelete;

        root = merge(t1, t2);
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
        std::cout << "<" << cur->val << "," << cur->priority << "> ";
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

    TNode* merge(TNode* t1, TNode* t2) {
        if (t1 == nullptr) {
            return t2;
        }

        if (t2 == nullptr) {
            return t1;
        }

        if (t1->priority > t2->priority) {
            t1->right = merge(t1->right, t2);
            return t1;
        }

        t2->left = merge(t1, t2->left);
        return t2;
    }

    std::pair<TNode*, TNode*> split(TNode* t, int key) {
        if (t == nullptr) {
            return {nullptr, nullptr};
        }

        if (t->val <=key) {
            auto p = split(t->right, key);
            t->right = p.first;
            auto t2 = p.second;

            return {t, t2};
        }
    }

    void split(TNode* t, int key, TNode* &t1, TNode* &t2) {
        if (t == nullptr) {
            t1 = t2 = nullptr;
            return;
        }

        if (t->val <= key) {
            split(t->right, key, t->right, t2);
            t1 = t;
            return;
        }

        split(t->left, key, t1, t->left);
        t2 = t;

    }
};

int main() {
    Treap tree = Treap();
    std::vector<int> insert = {3, 14, 15, 9, 2, 6, 35};

    for (auto val : insert) {
        tree.insert(val);
    }
    tree.inorder();

    std::cout << std::endl;

    std::vector<int> toDelete = {15, 3, 12, 1, 6};
    for (auto val : toDelete) {
        tree.remove(val);
    }

    tree.inorder();
}