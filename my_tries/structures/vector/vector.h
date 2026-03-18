#pragma once

#include <memory>

typedef struct node node;
struct node {
    std::unique_ptr<node> left;
    std::unique_ptr<node> right;
    int value;
};

typedef std::unique_ptr<node> vector;

vector CreateEmpty();
// void PushBack(vector vec, int value);
// void PushFront(vector vec, int value);
// int PopBack(vector vec);
// int PopFront(vector vec);
void Insert(vector vec, int pos);
void Delete(vector vec, int pos);
// int GetIndex(vector vec, int value);
// int Search(vector vec, int index);
// void DestroyVector(vector vec);
