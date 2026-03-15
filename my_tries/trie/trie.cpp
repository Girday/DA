#include "trie.h"

Trie CreateTrie() {
    return new Node{nullptr, '%', {}};
}

int AddWord(Trie trie, std::string word) {
    
}

int DeleteWord(Trie trie, std::string word) {

}

bool IsEmpty(Trie trie) {
    return trie->children.empty();
}

int Search(Trie trie, std::string word) {
    if (IsEmpty(trie)) // Если trie пустой, то там нет слов
        return 0;
        
    auto& children = trie->children; // Берём словарик детей

    if (word.size() == 0) { // Если искомое слово закончилось, проверяем, есть ли в конце sentinel ($)
        if (auto iter = children.find('$'); iter != children.end())
            return 1; // Если есть, то слово нашлось
        else
            return 0; // Если нет, то слово не нашлось
    }

    auto letter = word[0]; // Берём первую букву искомого слова
    if (auto iter = children.find(letter); iter != children.end()) // Ищем эту букву в словарике детей
        Search(iter->second, word.substr(1)); // Если такая нашлась, то запускаем поиск подслова после этой буквы
    else
        return 0; // Если нет, то слово не нашлось
}

void DestroyTrie(Trie trie) {

}
