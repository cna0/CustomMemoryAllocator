// Provides std::size_t, an unsigned integer type used to represent sizes and byte counts safely
#include <cstddef>
#include <iostream>

// Represents a chunk of memory within our memory pool
struct Block{
    std::size_t size; //stores how big the block is
    bool is_free; //checks whether block is currently available
    Block* next; //pointer to another block
};

class MemoryAllocator{
private:
    //pointer to memory pool
    char* memory;
    //pointer to first block
    Block* first_block;

public:
    //sets up the initial memory pool and metadata block
    MemoryAllocator(std::size_t size){
        memory = new char[size]; //allocates the raw byte array to be used as the memory pool

        first_block = new Block;
        first_block->size = size;
        first_block->is_free = true;
        first_block->next = nullptr;
    }
    //deconstructor to clean up dynamically allocated memory to prevent leaks
    ~MemoryAllocator(){
        delete[] memory;
        delete first_block;
    }

    
    void* allocate(std::size_t size){
        if (first_block->is_free && first_block->size){
            first_block->is_free = false;
            return memory;
        }
        return nullptr;
    }

    void print_state(){
        //prints the current state of allocators first block
        std::cout << "Memory size: " << first_block->size << '\n';
        std::cout << "Is free: " << first_block->is_free << '\n';
        std::cout << "Next block: " << first_block->next << '\n';
    }

};

int main(){
    MemoryAllocator allocator(1000);
    allocator.print_state();

    void* ptr1 = allocator.allocate(100);
    void* ptr2 = allocator.allocate(10); //shouldnt work because the block is already marked as used
    allocator.print_state();
    std::cout << ptr2 << '\n'; //shld return 0

}