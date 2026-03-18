#include "vector.h"
#include <iostream>

vector CreateEmpty() {
    return std::make_shared<list>(list{nullptr, nullptr});
}

bool IsEmpty(vector vec) {
    return vec->begin == nullptr && vec->end == nullptr;
}

void Insert(vector vec, int pos, int value) {
    if (IsEmpty(vec)) {
        std::shared_ptr<node> new_node = std::make_shared<node>(
            node{nullptr, nullptr, value}
        );

        vec->begin = new_node;
        vec->end = new_node;
    } else {
        int i = 0;
        auto iter = vec->begin;
        while (iter != vec->end) {
            auto left = iter->left;
            auto right = iter->right;
            
            if (i != pos) {
                ++i;
                iter = right;
                continue;
            }

            std::shared_ptr<node> new_node = std::make_shared<node>(
                node{iter, right, value}
            );

            iter->right->left = new_node;
            iter->right = new_node;
            break;
            
        }
    }

    return;
}

void Delete(vector vec, int pos) {
    if (IsEmpty(vec))
        return;
    
    int i = 0;
    auto iter = vec->begin;
    while (iter != vec->end) {
        auto left = iter->left;
        auto right = iter->right;

        if (i != pos) {
            ++i;
            iter = right;
            continue;
        }

        iter->left->right = right;
        iter->right->left = left;
        break;

    }

    return;
}

int main() {
    vector vec = CreateEmpty();
    Insert(vec, 2, 2);

    std::cout << IsEmpty(vec) << "\n";

    return 0;
}
