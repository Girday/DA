#include <iostream>
#include <vector>

void countingSort(std::vector<int> &array) {
    int k = array[0];

    for (int i = 1; i < array.size(); ++i)
        k = std::max(k, array[i]);
    
    std::vector<int> count(k + 1, 0);
    for (int i = 0; i < array.size(); ++i)
        ++count[array[i]];
    
    for (int i = 1; i < count.size(); ++i)
        count[i] += count[i - 1];

    std::vector<int> output(array.size());
    for (int i = array.size() - 1; i >= 0; i--)
        output[--count[array[i]]] = array[i];

    for (int i = 0; i < array.size(); ++i)
        array[i] = output[i];
}

int main() {
    std::vector<int> array = {2, 4, 7, 5, 8, 9, 3, 1, 6, 0};
    countingSort(array);

    for (int i = 0; i < array.size(); ++i)
        std::cout << array[i] << " ";
    
    std::cout << "\n";
}
