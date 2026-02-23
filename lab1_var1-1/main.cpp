#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <array>

using Key = int;
using Value = std::string;
using Object = std::pair<Key, Value>;

const Key KEY_RANGE = 65536;

void CountingSort(std::vector<Object>& objects) {
    if (objects.empty())
        return;
    
    // Частотный словарь ключей
    std::array<Key, KEY_RANGE> counting_array{};

    // Подсчёт частоты ключей
    for (const auto& obj : objects)
        ++counting_array[obj.first];

    // Создание префиксных сумм
    for (int i = 1; i < KEY_RANGE; ++i)
        counting_array[i] += counting_array[i - 1];

    // Отсортированный вектор длиной как начальный
    std::vector<Object> sorted_array(objects.size());
}

int main() {
    std::vector<Object> objects;
    
    Key key;
    Value value;
    while (std::cin >> key >> value)
        objects.push_back({key, value});

    // for (const auto& obj : objects)
    //     std::cout << "Key: " << obj.first << ", Value: " << obj.second << std::endl;

    CountingSort(objects);

    return 0;
}
