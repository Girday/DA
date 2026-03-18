#pragma once

#include <unordered_map>
#include <string>

typedef struct Node Node;
struct Node {
    Node* parent;
    char letter;
    std::unordered_map<char, Node*> children;
};

typedef Node* Trie;

Trie CreateTrie();
int AddWord(Trie trie, std::string word);
int DeleteWord(Trie trie, std::string word);
bool IsEmpty(Trie trie);
Trie Search(Trie trie, std::string word);
void DestroyTrie(Trie trie);
