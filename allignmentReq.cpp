#include <iostream>
#include <cstddef>

struct Example{
    char a;
    double b;
};

int main(){

    std::cout << "char: "
              << alignof(char) << '\n';

    std::cout << "int: "
              << alignof(int) << '\n';

    std::cout << "double: "
              << alignof(double) << '\n';

    std::cout << "Example: "
              << alignof(Example) << '\n';
}