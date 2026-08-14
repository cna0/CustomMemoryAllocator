#include <iostream>\

struct Block{
    std::size_t size; //stores how big the block is
    bool is_free; //checks whether block is currently available
    Block* next; //pointer to another block
};


int main(){
    Block block;
    block.size = 100;
    block.is_free = true;
    block.next = nullptr;

    std::cout << "Size: " << block.size << '\n';
    std::cout << "Free: " << block.is_free << '\n';
    std::cout << "Next: " << block.next << '\n';
}