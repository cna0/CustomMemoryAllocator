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
        Block* current = first_block;

        //keeps track of how many bytes passed while searching through the mem blocks
        std::size_t offset = 0;

        while (current != nullptr){
            if (current->is_free && current->size >= size){
                Block* new_block = new Block; //create a new block to rep remaining free memory
                new_block->size = current->size - size; //new block gets what ever mem is left after req allocation
                new_block->is_free = true; //the rem mem is free
                new_block->next = current->next; //mew block takes the place of the current old nect block
                current->next = new_block; //connect the current block to the newly created block
                current->size = size; //current block rep only the amount of mem requested
                current->is_free = false; //mark the current as being used
                return memory + offset; //return pointer to beginning of mem pool
            }
            
            offset += current->size;
            current = current->next; //this block wasnt suitable so we move to the next block

        }

        return nullptr;
    }

    //prints current state of all blocks
    void print_state(){
        Block* current = first_block;
        int block_number = 0;
        while (current != nullptr){
            std::cout << "Block " << block_number << '\n';
            std::cout << "   Size: " << current->size << " bytes\n";
            std::cout << "   Free: " << std::boolalpha << current->is_free << '\n';
            std::cout << '\n';

            current = current->next;
            ++block_number;
        }

    }


};

int main(){
    MemoryAllocator allocator(1000); //1000byte mem pool

    std::cout << "initial state: \n";
    allocator.print_state();

    void* ptr1 = allocator.allocate(100);

    std::cout << "\nAfter allocating 100 bytes: \n";
    allocator.print_state();

    void* ptr2 = allocator.allocate(250);

    std::cout << "After allocating 250 bytes: \n";
    allocator.print_state();

    std::cout << "\nPointer: \n";
    std::cout << "ptr1: " << ptr1 << '\n';
    std::cout << "ptr2: " << ptr2 << '\n';

    //testing by explicitly converting the pointers to char*
    auto* p1 = static_cast<char*>(ptr1);
    auto* p2 = static_cast<char*>(ptr2);

    std::cout << "Distance: " << p2 - p1 << " bytes\n";
    
    
}