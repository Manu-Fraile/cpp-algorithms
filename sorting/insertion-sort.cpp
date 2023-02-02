#include <iostream>
#include <vector>

std::vector<int> insertionSort(std::vector<int> input, int n){
    int i, key, j;

    for (i = 1; i < n; i++) {
        key = input[i];
        j = i - 1;
        while ((j >= 0) && (input[j] >= key)) {
            input[j+1] = input[j];
            j = j - 1;
        }
        input[j+1] = key;
    }

    return input;
}

int main() {
    std::vector<int> input = {5, 2, 4, 6, 1, 3};
    int n = static_cast<int>(input.size());

    std::vector<int> input_sorted = insertionSort(input, n);

    std::cout << "THE ORIGIN ARRAY: [ ";
    for (int i : input) {
        std::cout << i << " ";
    }
    std::cout << "]";

    std::cout << "\nTHE SORTED ARRAY: [ ";
    for (int i : input_sorted) {
        std::cout << i << " ";
    }
    std::cout << "]";
}
