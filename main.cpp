#include <iostream>\

struct Block{
    std::size_t size; //stores how big the block is
    bool is_free; //checks whether block is currently available
    Block* next; //pointer to another block
};


int main(){
    Block first;
    Block second;

    first.size = 100;
    first.is_free = true;
    first.next = &second;


    second.size = 250;
    second.is_free = true;
    second.next = nullptr;

    std::cout << "Size: " << first.size << '\n';
    std::cout << "Free: " << first.is_free << '\n';
    std::cout << "Next: " << first.next << '\n';

    std::cout << "Size: " << second.size << '\n';
    std::cout << "Free: " << second.is_free << '\n';
    std::cout << "Next: " << second.next << '\n';

}