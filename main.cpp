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

    void deallocate(void* ptr){
        //nothing to free
        if (ptr == nullptr){
            return;
        }
        Block* current = first_block;
        //keeps track of whjere current block starts relative to the beginning of the pool
        std::size_t offset = 0;

        while (current != nullptr){
            //calculates the address where this blocks usable memory begins
            char* block_address = memory + offset;
            //checks whether the pointer we were given belong to this block
            if (ptr == block_address){
                //make sure we arent freeeing the same block twice
                if (current->is_free){
                    std::cout << "Warning: Block is alreay free \n";
                    return;
                }
                current->is_free = true; //mark the block as avaible
                return;

                //check whether next block is free
                if (current->next != nullptr && current->next->is_free){
                    Block* next_block = current->next;
                    current->size += next_block->size; //combine the 2 blocks
                    current->next = next_block->next; //remove the next block from the linked list
                    delete next_block; //delete since we no longer need its metadata
                }
                return;
            }
            //move to the next block
            offset += current->size;
            current = current->next;
        }
        //the pointer didnt belong to our allocator
        std::cout << "Warning: pointer doesnt belong to allocator";
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

/*
int main(){
    std::cout << std::boolalpha;

    MemoryAllocator allocator(1000); //1000byte mem pool

    std::cout << "Initial state:\n";
    allocator.print_state();

    std::cout << "\nAllocating 100 bytes...\n";
    void* ptr1 = allocator.allocate(100);
    std::cout << "Pointer: " << ptr1 << '\n';
    allocator.print_state();

    std::cout << "\nAllocating 200 bytes...\n";
    void* ptr2 = allocator.allocate(200);
    std::cout << "Pointer: " << ptr2 << '\n';
    allocator.print_state();

    std::cout << "\nChecking pointers...\n";
    if (ptr1 != ptr2) {
        std::cout << "PASS: pointers are different.\n";
    } else {
        std::cout << "FAIL: pointers are the same.\n";
    }

    std::cout << "\nFreeing the first block...\n";
    allocator.deallocate(ptr1);
    allocator.print_state();

    std::cout << "\nAllocating 50 bytes...\n";
    void* ptr3 = allocator.allocate(50);
    std::cout << "Pointer: " << ptr3 << '\n';
    allocator.print_state();

    std::cout << "\nAllocating 600 bytes...\n";
    void* ptr4 = allocator.allocate(600);
    std::cout << "Pointer: " << ptr4 << '\n';
    allocator.print_state();

    std::cout << "\nTrying to allocate 1000 bytes...\n";
    void* ptr5 = allocator.allocate(1000);

    if (ptr5 == nullptr) {
        std::cout << "PASS: allocation failed as expected.\n";
    } else {
        std::cout << "FAIL: allocation should have failed.\n";
    }

    std::cout << "\nCalling deallocate(nullptr)...\n";
    allocator.deallocate(nullptr);
    std::cout << "Done.\n";

    std::cout << "\nFinal state:\n";
    allocator.print_state();

    return 0;
    
}
*/

int main(){
    std::cout << "\n========== COALESCING TEST ==========\n";

    MemoryAllocator test_allocator(1000);

    void* a = test_allocator.allocate(100);
    void* b = test_allocator.allocate(200);
    void* c = test_allocator.allocate(300);

    std::cout << "\nAfter allocating 100, 200, 300:\n";
    test_allocator.print_state();

    test_allocator.deallocate(b);

    std::cout << "\nAfter freeing the 200-byte block:\n";
    test_allocator.print_state();

    test_allocator.deallocate(a);

    std::cout << "\nAfter freeing the 100-byte block:\n";
    test_allocator.print_state();
}
