#include "trie.h"

Trie CreateTrie() {
    return new Node{nullptr, '%', {}};
}

int AddWord(Trie trie, std::string word) { // Функция добавления слова в trie
    auto& children = trie->children; // Берём словарик детей

    if (word.size() == 0) { // Если вставляемое слово закончилось, ищем sentinel
        if (auto iter = children.find('$'); iter != children.end())
            return 0; // Если нашли, то слово уже есть
        else { // Если нет, то создаём sentinel
            children['$'] = new Node{trie, '$', {}};
            return 1; // Вставка закончена
        }
    }

    auto letter = word[0]; // Берём первую букву слова ищем её в словаре детей
    if (auto iter = children.find(letter); iter != children.end())
        return AddWord(children[letter], word.substr(1)); // Если нашли, запускаем AddWord для остальной части слова
    else {
        children[letter] = new Node{trie, letter, {}}; // Если нет, то добавляем новую ноду в словарь
        return AddWord(children[letter], word.substr(1)); // И запускаем AddWord для остльной части слова
    }
}

int DeleteWord(Trie trie, std::string word) {

}

bool IsEmpty(Trie trie) {
    return trie->children.empty();
}

Trie Search(Trie trie, std::string word) {
    if (IsEmpty(trie)) // Если trie пустой, то там нет слов
        return nullptr;
        
    auto& children = trie->children; // Берём словарик детей

    if (word.size() == 0) { // Если искомое слово закончилось, проверяем, есть ли в конце sentinel ($)
        if (auto iter = children.find('$'); iter != children.end())
            return iter->second;  // Если есть, то слово нашлось
        else
            return nullptr;  // Если нет, то слово не нашлось
    }

    auto letter = word[0]; // Берём первую букву искомого слова
    if (auto iter = children.find(letter); iter != children.end()) // Ищем эту букву в словарике детей
        return Search(iter->second, word.substr(1)); // Если такая нашлась, то запускаем поиск подслова после этой буквы
    else
        return nullptr; // Если нет, то слово не нашлось
}

void DestroyTrie(Trie trie) {

}
