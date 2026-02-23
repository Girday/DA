#include <iostream>
#include <vector>
#include <utility>
#include <string>

using Key = int;
using Value = std::string;
using Object = std::pair<Key, Value>;

const Key MAX_KEY = 65535;

void CountingSort(std::vector<Object>& objects) {
    if (objects.empty())
        return;
}

int main() {
    std::vector<Object> objects;
    
    Key key;
    Value value;
    while (std::cin >> key >> value)
        objects.push_back({key, value});

    for (const auto& obj : objects)
        std::cout << "Key: " << obj.first << ", Value: " << obj.second << std::endl;

    return 0;
}
