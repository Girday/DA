#include <iostream>
#include <vector>

void merge(std::vector<int> &array, int l, int mid, int r) {
    std::vector<int> res(r - l);
    int i = l, j = mid, key = 0;

    while (i < mid && j < r) {
        if (array[i] <= array[j])
            res[key++] = array[i++];
        else
            res[key++] = array[j++];
    }

    while (i < mid)
        res[key++] = array[i++];

    while (j < r)
        res[key++] = array[j++];

    for (int k = 0; k < r - l; ++k)
        array[l + k] = res[k];
}

void mergeSortRecursively(std::vector<int> &array, int l, int r) {
    if (r - l <= 1)
        return;
    
    int mid = l + (r - l) / 2;
    mergeSortRecursively(array, l, mid);
    mergeSortRecursively(array, mid, r);
    merge(array, l, mid, r);
}

void mergeSort(std::vector<int> &array) {
    mergeSortRecursively(array, 0, array.size());
}


int main() {
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    std::vector<int> array = {5, 1, 2, 9, 0, 3};
    mergeSort(array);

    for (int i = 0; i < array.size(); ++i)
        std::cout << array[i] << "\n";
}
