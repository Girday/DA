#include <iostream>
#include <vector>
#include <cstdint>

void CountingSortForRadix(std::vector<uint64_t>& vec, int shift) {
    const int k = 256;
    std::vector<int> count(k);
    for (const auto& elem : vec)
        ++count[(elem >> shift) & 0xFF];
    
    for (int i = 1; i < k; ++i)
        count[i] += count[i - 1];
    
    std::vector<uint64_t> sorted(vec.size());
    for (int i = vec.size() - 1; i >= 0; --i)
        sorted[--count[(vec[i] >> shift) & 0xFF]] = vec[i];
    
    vec.swap(sorted);
}

void RadixSort(std::vector<uint64_t>& vec) {
    for (int shift = 0; shift < 256; shift += 8)
        CountingSortForRadix(vec, shift);
}

int main() {
    std::vector<uint64_t> vec = {255, 17, 999, 5, 80, 91, 356, 11, 666, 8};
    RadixSort(vec);
    
    for (auto elem : vec)
        std::cout << elem << " ";
    std::cout << "\n";
}