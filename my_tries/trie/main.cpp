#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "trie.h"

void PrintTrie(Trie trie, const std::string& prefix = "", bool is_last = true) {
    if (!trie)
        return;

    if (trie->parent == nullptr)
        std::cout << "% (root)\n";
    else
        std::cout << prefix << (is_last ? "`-- " : "|-- ")
                  << trie->letter << (trie->letter == '$' ? " (end)" : "") << "\n";

    std::vector<std::pair<char, Trie>> children(trie->children.begin(), trie->children.end());
    std::sort(children.begin(), children.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

    const std::string next_prefix =
        (trie->parent == nullptr) ? "" : prefix + (is_last ? "    " : "|   ");

    for (std::size_t i = 0; i < children.size(); ++i)
        PrintTrie(children[i].second, next_prefix, i + 1 == children.size());
}

int main() {
    Trie trie = CreateTrie();

    std::vector<std::string> words = {"word", "work", "worker", "cat", "car"};
    for (const auto& word : words)
        AddWord(trie, word);

    std::cout << "After insertion:\n";
    PrintTrie(trie);

    std::cout << "\nSearch(\"work\"): " << (Search(trie, "work") ? 1 : 0) << "\n";
    std::cout << "Search(\"cow\"): " << (Search(trie, "cow") ? 1 : 0) << "\n";

    DeleteWord(trie, "work");
    std::cout << "\nAfter deleting \"work\":\n";
    PrintTrie(trie);

    DestroyTrie(trie);
    return 0;
}
