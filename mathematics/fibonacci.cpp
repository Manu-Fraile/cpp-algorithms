#include <iostream>
#include <vector>


std::vector<long> fibonacci(int n_elements)
{
    long a, b;
    std::vector<long> fibonacci_arr;
    for(int i = 0; i < n_elements + 1; i++) {

        if (i==0) {
            fibonacci_arr.push_back(0);
            a = 0;
        }

        if (i==1) {
            fibonacci_arr.push_back(1);
            b = 1;
        }

        if (i>1) {
            fibonacci_arr.push_back(b + a);
            a = b;
            b = fibonacci_arr[i];
        }
    }

    return fibonacci_arr;
}

int main() {
    std::vector<long> fib_seq;
    int n_input;

    std::cout << "WELCOME TO THE FIBONACCI C++ CALCULATOR" << std::endl;
    std::cout << "Please enter the number of elements you want: " << std::endl;
    std::cin >> n_input;

    fib_seq = fibonacci(n_input);

    for(int i = 0; i < n_input + 1; i++) {
        std::cout << "Element number " << "(" << i << "): "  << fib_seq[i] << std::endl;
    }
}