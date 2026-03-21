#include <iostream>
#include <vector>
#include <cstdint>

void countingSortForRadix(std::vector<uint64_t> &nums, int shift) {
    int k = 256;
    std::vector<int> cnt(k, 0);
    for (const auto &elem: nums)
        ++cnt[(elem >> shift) & 0xFF];
    
    for (int i = 1; i < k; ++i)
        cnt[i] += cnt[i - 1];
    
    std::vector<uint64_t> res(nums.size());
    for (int i = nums.size() - 1; i >= 0; --i)
        res[--cnt[(nums[i] >> shift) & 0xFF]] = nums[i];
    
    for (int i = 0; i < res.size(); ++i)
        nums[i] = res[i];
}

void radixSort(std::vector<uint64_t> &nums) {
    for (int shift = 0; shift < 256; shift += 8)
        countingSortForRadix(nums, shift);
}

int main() {
    std::vector<uint64_t> array = {255, 17, 999, 5, 80, 91, 356, 11, 666, 8};
    radixSort(array);

    for (int i = 0; i < array.size(); ++i)
        std::cout << array[i] << " ";
    std::cout << "\n";
}
