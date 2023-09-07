#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>

class MyException : public std::exception {
public:
    const char* what() const noexcept override {
        return "Error extracting max value in input vector.";
    }
};

std::vector<int> countingSort(std::vector<int> A, int n, int k){
    std::vector<int> B(n); // Output array
    std::vector<int> C(k + 1, 0); // Counting array

    // Step 1: Initialize the counting array C
    for (int j = 0; j < n; ++j) {
        C[A[j]]++;
    }

    // Step 2: Modify C to contain the actual position of each element in B
    for (int i = 1; i <= k; ++i) {
        C[i] += C[i - 1];
    }

    // Step 3: Build the sorted array B
    for (int j = n - 1; j >= 0; --j) {
        B[C[A[j]] - 1] = A[j];
        C[A[j]]--;
    }

    return B;
}

int returnMaxElement(const std::vector<int>& vectorObj){
    try{
        auto it = std::max_element(vectorObj.begin(), vectorObj.end());
        if(it != vectorObj.end()){
            return *it;
        }
        else {
            throw MyException();
        }
    }
    catch(...) {
        std::cerr << "ERROR EXTRACTING MAX VALUE" << std::endl;
    }
}

int main() {
    std::vector<int> input = {6, 0, 2, 0, 1, 3, 4, 6, 1, 3, 2, 0, 1, 1, 1, 8, 5, 3, 4, 6, 4, 7, 2, 7, 5, 1, 3, 4, 0, 5};
    int n = static_cast<int>(input.size());
    int k = returnMaxElement(input);

    std::vector<int> input_sorted = countingSort(input, n, k);

    std::cout << "THE ORIGINAL ARRAY: [ ";
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
