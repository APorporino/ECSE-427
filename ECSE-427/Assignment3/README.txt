# ECSE427 - Assignment 3 
## Simple Memory Allocator - (SMA)

### Message to grader:

All implementations demanded has been compeleted. All tests pass.

The makefile contains a script to generate an executable that can run the tests:
    make test
Alternatively it can be compiled into object code and then used in another way:
    make sma


My implementation included using a struct (Block) to store the length, one tag value and 2 pointers.
I also added 3 functions: 

    1.) createNode(int length, int tag, Block *next, Block *prev)

        This method will create a new block. It is basically a constructor. 
        It then returns this newly created block.

    2.) add_block_free_list_from_free(void *block)

        This function will add a block to the free list if it comes from the sma_free function.
        The regular "add_block_freeList" is left only to be used when we add the block to the end of the list.
        The newly created function will deal with appropriately placing the new free block in the list to
        deal with things like merging and correct placement.

    3.) closestFreeBlock(void *last)

        This function will return the closet free block in the free block list to the pointer passed to the function.
        It is used in next fit algorithm where it passes the global "lastAllocatedSpot" pointer to this function.


Besides those 3 added fucntions the other change I made was how I determined the total free space left. I decided to just loop through
the free block list and keep adding the space to the global total. This does not increase the time since I looped through it anyway to find the largest block.

I did add 2 lines to sma_malloc and 1 line to sma_free relating to keeping track of total allocated memory and last allocated spot. I did this because I 
found those to be to easiest places to keep track of those required variables. I did NOT however change anything else about how it runs or the implementation.


General stuff: In my opinion, this assignment was extremly difficult and I spent many many many hours working on it and mostly debugging. It taught me so much
about memeory addresses and how the heap and stack work when a program runs. It also gave me confidence in my ability to code in C, and forced me to learn 
how to use the GDB debugger which was a crucial part to my success.

Thanks for reading!

