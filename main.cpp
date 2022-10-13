#include "iostream"
#include "vector"
#include "chrono"


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
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::time_point<std::chrono::system_clock> endTime;

    std::vector<long> fib_seq;
    int n_input;

    std::cout << "WELCOME TO THE FIBONACCI C++ CALCULATOR" << std::endl;
    std::cout << "Please enter the number of elements you want: " << std::endl;
    std::cin >> n_input;

    fib_seq = fibonacci(n_input);

    /*startTime = std::chrono::system_clock::now();
    int a = 0;
    for (int i = 0; i < 1000000000; i++) {
        a += 1;
    }
    endTime = std::chrono::system_clock::now();
    std::cout << "DONE: " << a << std::endl;*/

    for(int i = 0; i < n_input + 1; i++) {
        std::cout << "Element number " << "(" << i << "): "  << fib_seq[i] << std::endl;
    }

//    std::cout << "\n\nTIME ELAPSED: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << std::endl;
}