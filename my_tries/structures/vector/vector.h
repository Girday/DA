#pragma once

#include <memory>

typedef struct node node;
struct node {
    std::shared_ptr<node> left;
    std::shared_ptr<node> right;
    int value;
};

typedef struct list list;
struct list {
    std::shared_ptr<node> begin;
    std::shared_ptr<node> end;
};

typedef std::shared_ptr<list> vector;

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
