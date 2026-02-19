#include <iostream>
#include <vector>
#include <utility>
#include <string>

using Key = int;
using Value = std::string;
using Object = std::pair<Key, Value>;

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
