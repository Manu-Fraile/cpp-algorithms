#include <iostream>
#include <vector>

std::vector<long> collatz(long input){
    long new_element;
    std::vector<long> collatz_vector = {input};
    bool check = false;

    do {
        if (collatz_vector.back() % 2 == 0) {
            new_element = collatz_vector.back() / 2;
            collatz_vector.push_back(new_element);
        }
        else {
            new_element = collatz_vector.back()*3 + 1;
            collatz_vector.push_back(new_element);
        }

        if (collatz_vector.size() > 6) {
            check = (collatz_vector.end()[-1] == 1) && (collatz_vector.end()[-4] == 1);
        }
    } while(!check);

    return collatz_vector;
}

int main() {
    long input = 25598741357789546;
    std::vector<long> output = collatz(input);

    std::cout << "The Collatz distribution: ";
    for (long i : output) {
        std::cout << i << " ";
    }
}
