#include <iostream>
#include <vector>

void BucketSort(std::vector<double>& vec) {
    if (vec.empty())
        return;

    int n = static_cast<int>(vec.size());
    std::vector<std::vector<double>> buckets(n);

    for (double elem : vec)
        buckets[static_cast<int>(n * elem)].push_back(elem);        
    
    std::vector<double> sorted;
    for (auto& bucket : buckets) {
        std::stable_sort(bucket.begin(), bucket.end());
        for (double elem : bucket)
            sorted.push_back(elem);
    }

    vec.swap(sorted);
}

int main() {
    std::vector<double> vec = {0.1, 0.5, 0.3, 0.25, 0.9};
    BucketSort(vec);

    for (double elem : vec)
        std::cout << elem << "\n";

    return 0;
}