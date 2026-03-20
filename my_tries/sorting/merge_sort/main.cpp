#include <iostream>
#include <vector>

void Merge(std::vector<int>& vec, int l, int mid, int r) {
    std::vector<int> res(r - l);
    int i = l, j = mid, key = 0;

    while (i < mid && j < r) {
        if (vec[i] <= vec[j])
            res[key++] = vec[i++];
        else
            res[key++] = vec[j++];
    }
    
    while (i < mid)
        res[key++] = vec[i++];
    while (j < r)
        res[key++] = vec[j++];
    
    for (int k = 0; k < r - l; ++k)
        vec[l + k] = res[k];
}

void MergeSortRecursively(std::vector<int>& vec, int l, int r) {
    if (r - l <= 1)
        return;

    int mid = l + (r - l) / 2;
    MergeSortRecursively(vec, l, mid);
    MergeSortRecursively(vec, mid, r);
    Merge(vec, l, mid, r);
}

void MergeSort(std::vector<int>& vec) {
    MergeSortRecursively(vec, 0, vec.size());
}

int main() {
    std::vector<int> vec = {5, 1, 2, 9, 0, 3};
    MergeSort(vec);

    for (int elem : vec)
        std::cout << elem << "\n";

    return 0;
}
