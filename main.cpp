// Provides std::size_t, an unsigned integer type used to represent sizes and byte counts safely
#include <cstddef>
#include <iostream>
#include <string>

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
        //strt at first block
        Block* current = first_block;
        while(current != nullptr){ //go through entire linked list
            Block* next = current->next; //save the next block before deleting current block
            delete current; //delete current block metadata
            current = next; //next block
        }
        delete[] memory; // finally release the mem pool
    }

    
    //First fit allocation strategy
    //Searches from the beginning of the block list and uses the first free block large enough to satisfy the requested allocation
    void* allocate(std::size_t size){
        //not allowing zero byte allocations
        if (size == 0){
            return nullptr;
        }

        Block* current = find_first_fit(size);

        if (current == nullptr){
            return nullptr;
        }

        //keeps track of how many bytes passed while searching through the mem blocks
        std::size_t offset = 0;

        while (current != nullptr){
            if (current->is_free && current->size >= size){
                //only create a new block if there is mem left over
                if (current->size > size){
                    Block* new_block = new Block; //create a new block to rep remaining free memory
                    new_block->size = current->size - size; //new block gets what ever mem is left after req allocation
                    new_block->is_free = true; //the rem mem is free
                    new_block->next = current->next; //mew block takes the place of the current old nect block
                    current->next = new_block; //connect the current block to the newly created block
                }
                current->size = size; //current block rep only the amount of mem requested
                current->is_free = false; //mark the current as being used
                return memory + offset; //return pointer to beginning of mem pool
            }
            
            offset += current->size;
            current = current->next; //this block wasnt suitable so we move to the next block

        }

        return nullptr;
    }

    Block* find_first_fit(std::size_t size){
        Block* current = first_block;
        while (current != nullptr){
            if (current->is_free && current->size >= size){
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }

    void deallocate(void* ptr){
        //nothing to free
        if (ptr == nullptr){
            return;
        }
        Block* current = first_block;
        Block* previous = nullptr;
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

                // MERGE WITH NEXT BLOCK
                //check whether next block is free
                if (current->next != nullptr && current->next->is_free){
                    Block* next_block = current->next;
                    current->size += next_block->size; //combine the 2 blocks
                    current->next = next_block->next; //remove the next block from the linked list
                    delete next_block; //delete since we no longer need its metadata
                }

                // MERGE WITH PREVIOUS BLOCK
                if (previous != nullptr && previous->is_free){
                    previous->size += current->size; //combine with the previous block
                    previous->next = current->next; //remove the prev block from the linked list
                    delete current; //delete since we no longer need its metadata
                }

                return;
            }
            //move to the next block
            offset += current->size;
            previous = current;
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

int tests_passed = 0;
int tests_failed = 0;

void test_result(const std::string& test_name, bool passed){
    if (passed){
        std::cout << "[PASS] " << test_name << '\n';
        ++tests_passed;
    }
    else{
        std::cout << "[FAIL] " << test_name << '\n';
        ++tests_failed;
    }
}


void test_basic_allocation(){
    MemoryAllocator allocator(1000);

    void* ptr = allocator.allocate(100);

    test_result(
        "Basic allocation",
        ptr != nullptr
    );
}

void test_multiple_allocations(){
    MemoryAllocator allocator(1000);

    void* ptr1 = allocator.allocate(100);
    void* ptr2 = allocator.allocate(200);

    test_result(
        "Multiple allocations",
        ptr1 != nullptr && ptr2 != nullptr && ptr1 != ptr2
    );
}

void test_oversized_allocation(){
    MemoryAllocator allocator(1000);

    void* ptr = allocator.allocate(1001);

    test_result(
        "Oversized allocation",
        ptr == nullptr
    );
}

void test_zero_allocation(){
    MemoryAllocator allocator(1000);

    void* ptr = allocator.allocate(0);

    test_result(
        "Zero-byte allocation",
        ptr == nullptr
    );
}

void test_deallocation(){
    MemoryAllocator allocator(1000);

    void* ptr = allocator.allocate(100);

    allocator.deallocate(ptr);

    void* new_ptr = allocator.allocate(100);

    test_result(
        "Deallocation",
        new_ptr != nullptr
    );
}

void test_memory_reuse(){
    MemoryAllocator allocator(1000);

    void* ptr = allocator.allocate(100);

    allocator.deallocate(ptr);

    void* new_ptr = allocator.allocate(100);

    test_result(
        "Memory reuse",
        new_ptr == ptr
    );
}

void test_coalescing(){
    MemoryAllocator allocator(1000);

    void* ptr1 = allocator.allocate(300);
    void* ptr2 = allocator.allocate(300);
    void* ptr3 = allocator.allocate(400);

    allocator.deallocate(ptr1);
    allocator.deallocate(ptr3);
    allocator.deallocate(ptr2);

    void* ptr4 = allocator.allocate(1000);

    test_result(
        "Coalescing",
        ptr4 != nullptr
    );
}

void test_double_free(){
    MemoryAllocator allocator(1000);

    void* ptr = allocator.allocate(100);

    allocator.deallocate(ptr);

    // Attempt to free the same block again.
    allocator.deallocate(ptr);

    // The allocator should still be usable.
    void* new_ptr = allocator.allocate(100);

    test_result(
        "Double-free protection",
        new_ptr != nullptr
    );
}

void test_invalid_pointer(){
    MemoryAllocator allocator(1000);

    int value = 42;

    allocator.deallocate(&value);

    // The allocator should still be usable.
    void* ptr = allocator.allocate(100);

    test_result(
        "Invalid pointer protection",
        ptr != nullptr
    );
}

void test_fragmentation(){

    MemoryAllocator allocator(1000);

    //create four blocks.
    void* ptr1 = allocator.allocate(200);
    void* ptr2 = allocator.allocate(200);
    void* ptr3 = allocator.allocate(200);
    void* ptr4 = allocator.allocate(200);

    // Free the first and third blocks.
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr3);

    // Therefire there is now:
    //
    // [200 FREE][200 USED][200 FREE][200 USED][200 FREE]
    //
    // Total free memory = 600 bytes.
    //
    // However, the largest contiguous free block is only 200 bytes.

    void* ptr5 = allocator.allocate(500);

    test_result(
        "Fragmentation handling",
        ptr5 == nullptr
    );
}



int main(){

    std::cout << "========== MEMORY ALLOCATOR TESTS ==========\n\n";

    test_basic_allocation();
    test_multiple_allocations();
    test_oversized_allocation();
    test_zero_allocation();
    test_deallocation();
    test_memory_reuse();
    test_coalescing();
    test_fragmentation();
    test_double_free();
    test_invalid_pointer();

    std::cout << "\n=============================================\n";

    std::cout << "Tests passed: "
              << tests_passed << '\n';

    std::cout << "Tests failed: "
              << tests_failed << '\n';

    return 0;
} 