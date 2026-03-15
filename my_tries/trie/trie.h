#include <map>
#include <string>

typedef struct Node Node;
struct Node {
    Node* parent;
    char letter;
    std::map<char, Node*> children;
};

typedef Node* Trie;

Trie CreateTrie();
int AddWord(Trie trie, std::string word);
int DeleteWord(Trie trie, std::string word);
bool IsEmpty(Trie trie);
int Search(Trie trie, std::string word);
void DestroyTrie(Trie trie);
