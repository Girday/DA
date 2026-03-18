#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

int main() {
    std::vector<int> sorting_vector;
    int input;
    while (std::cin >> input)
        sorting_vector.push_back(input);
    
    int max = *std::max_element(sorting_vector.begin(), sorting_vector.end());
    std::vector<int> frequency_map(max + 1);

    for (int elem : sorting_vector)
        ++frequency_map[elem];
    
    for (int i = 1; i < max + 1; ++i)
        frequency_map[i] += frequency_map[i - 1];
    
    std::vector<int> sorted_vector(sorting_vector.size());
    for (int i = sorting_vector.size() - 1; i >= 0; --i) {
        int elem = sorting_vector[i];
        sorted_vector[--frequency_map[elem]] = elem;
    }
    std::cout << "\n";

    for (int elem : sorted_vector)
        std::cout << elem << " ";
    std::cout << "\n";

    return 0;
}
