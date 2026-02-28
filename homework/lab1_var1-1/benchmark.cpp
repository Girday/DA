#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using Key = int;
using Value = std::string;
using Object = std::pair<Key, Value>;

const Key KEY_RANGE = 65536;

void CountingSort(std::vector<Object>& objects) {
    if (objects.empty())
        return;
    
    std::array<Key, KEY_RANGE> counting_array{};

    for (const auto& obj : objects)
        ++counting_array[obj.first];

    for (int i = 1; i < KEY_RANGE; ++i)
        counting_array[i] += counting_array[i - 1];

    std::vector<Object> sorted_array(objects.size());

    for (int i = objects.size(); i > 0; --i) {
        const auto& obj = objects[i - 1];
        int position = --counting_array[obj.first];
        sorted_array[position] = obj;
    }

    objects.swap(sorted_array);
}


using duration_t_main = std::chrono::milliseconds;
const std::string DURATION_PREFIX_main = "ms";

using duration_t_help = std::chrono::microseconds;
const std::string DURATION_PREFIX_help = "us";

int main() {
    std::vector<Object> objects;

    Key key;
    Value value;
    while (std::cin >> key >> value)
        objects.emplace_back(key, value);

    auto objects_counting = objects;
    auto objects_stl = objects;

    std::cout << "Count of lines is " << objects.size() << "\n";

    std::chrono::time_point<std::chrono::system_clock> start_ts = std::chrono::system_clock::now();
    CountingSort(objects);
    auto end_ts = std::chrono::system_clock::now();
    uint64_t counting_sort_ts_main = std::chrono::duration_cast<duration_t_main>(end_ts - start_ts).count();
    uint64_t counting_sort_ts_help = std::chrono::duration_cast<duration_t_help>(end_ts - start_ts).count();

    start_ts = std::chrono::system_clock::now();
    std::stable_sort(std::begin(objects_stl), std::end(objects_stl));
    end_ts = std::chrono::system_clock::now();

    uint64_t stl_sort_ts_main = std::chrono::duration_cast<duration_t_main>(end_ts - start_ts).count();
    uint64_t stl_sort_ts_help = std::chrono::duration_cast<duration_t_help>(end_ts - start_ts).count();
    std::cout << "Counting sort time: " << counting_sort_ts_main << DURATION_PREFIX_main << " (" << counting_sort_ts_help << DURATION_PREFIX_help << ")" << "\n";
    std::cout << "STL stable sort time: " << stl_sort_ts_main << DURATION_PREFIX_main << " (" << stl_sort_ts_help << DURATION_PREFIX_help << ")" << "\n";
}
