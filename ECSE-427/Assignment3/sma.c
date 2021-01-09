/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		2.0
 *  Created:  		06/11/2020 9:30:00 AM - Mohammad Mushfiqur Rahman
 *  Revised:  		20/11/2020 2:34:00 AM - Anthony Porporino
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman & Anthony Porporino
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file
#include <stdlib.h>

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
int FREE_BLOCK_HEADER_SIZE = sizeof(Block);
//	TODO: Add constants here

typedef enum //	Policy type definition
{
	WORST,
	NEXT
} Policy;

char *sma_malloc_error;
void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
void *lastAllocatedSpot = NULL;		  // 	The pointer to the last allocated spot to be used in NextFit search
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
	void *pMemory = NULL;

	// Checks if the free list is empty
	if (freeListHead == NULL)
	{
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	// If free list is not empty
	else
	{
		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);

		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{
			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);
		}
	}

	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}

	totalAllocatedSize += size; 
	lastAllocatedSpot = pMemory;
	return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		//	Adds the block to the free memory list
		add_block_free_list_from_free(ptr);
	}
	totalAllocatedSize -= get_blockSize(ptr + FREE_BLOCK_HEADER_SIZE); 
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
	// easier way to find total free size is when we loop through the list using get_largest_freeBlock
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.
	Block *p = ptr -32;
	void *excessFreeBlock;

	if (p->length == size){
		// if size is same we do not need to do anything

		return ptr;
	}else if (p->length > size){
		// if user wants to shrink size we need to add that excess memeory to the freeSpace

		p->length = size;
		excessFreeBlock = p + size + FREE_BLOCK_HEADER_SIZE;
		((Block *)excessFreeBlock)->length = ((Block *)ptr)->length - size - FREE_BLOCK_HEADER_SIZE;
		((Block *)excessFreeBlock)->tag = 1;
		add_block_free_list_from_free(excessFreeBlock + FREE_BLOCK_HEADER_SIZE);

		return p + FREE_BLOCK_HEADER_SIZE;
	}else {
		// we do not have enough space and need to find more and the free the space it currently has

		// save ptr so we can save the users data
		void *temp = ptr;

		// get more space
		void *newPtr = sma_malloc(size);
		*(int *) newPtr = *(int *)temp;
		sma_free(ptr);
		return newPtr;
	}

}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
	void *newBlock = NULL;
	int excessSize = 0;

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list

	// excess will either 10*size or will be 128KB
	excessSize += 9*size - FREE_BLOCK_HEADER_SIZE;

	// request for 4 times as much of what was asked unless it is over 128KB!
	if (excessSize > 128 * 1024){
		excessSize = 128 * 1024;
		newBlock = sbrk(size + excessSize);
	}else {
		newBlock = sbrk(size*10);
	}

	// we have just asked for more space but it is possible we have leftover space
	// in our free list that just wasnt big enough. We still want to use that space, we
	// do not want to forget about it!
	if (freeListHead != NULL){
		newBlock = freeListHead;
		excessSize += ((Block *)freeListHead)->length;
		freeListHead = ((Block *)freeListHead)->next;
	}
	
	Block m = createNode(size,0,NULL,NULL);
	*(Block *) newBlock = m;

	//	Allocates the Memory Block
	allocate_block(newBlock, size + FREE_BLOCK_HEADER_SIZE, excessSize, 0);

	return newBlock + FREE_BLOCK_HEADER_SIZE;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
	void *pMemory = NULL;

	if (currentPolicy == WORST)
	{
		// Allocates memory using Worst Fit Policy		
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}

	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;
	int largest = 0;
	//	TODO: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to 
	//			get the largest block
	void *cursor = freeListHead;

	// loop through list untill we find the largest available block or NULL
	while (1){
		if (cursor == NULL){
			break;
		}
		
		int newSize = ((Block *)cursor)->length;
		if (newSize >= size){
			if (largest == 0 || newSize > largest){
				largest = newSize;
				worstBlock = cursor;
				blockFound = 1;
			}
		}

		cursor = ((Block *)cursor)->next;
	}

	//	Checks if appropriate block is found.
	if (blockFound) 
	{
		excessSize = ((Block *)worstBlock)->length - size - FREE_BLOCK_HEADER_SIZE;
		//	Allocates the Memory Block
		
		allocate_block(worstBlock, size + FREE_BLOCK_HEADER_SIZE, excessSize, 1);
		return worstBlock + FREE_BLOCK_HEADER_SIZE;
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		worstBlock = (void *)-2;
	}
	
	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	You should use a global pointer to keep track of your last allocated memory address, and 
	//			allocate free blocks that come after that address (i.e. on top of it). Once you reach 
	//			Program Break, you start from the beginning of your heap, as in with the free block with
	//			the smallest address)

	void *startNodeCursor = closestFreeBlock(lastAllocatedSpot);	// will give first node to start searching with
	void *cursor = freeListHead;									// to be used to search through list
	void *startNode  = closestFreeBlock(lastAllocatedSpot);			// will be used to help search through from head to the first node we started with
	void *programBreak = sbrk(0);									// current address of program break

	// loops through free list untill we find first block that satisfies the request
	while(1){
		if (startNodeCursor == NULL || startNodeCursor >= programBreak){
			break;
		}
		int lengthFree = ((Block*)startNodeCursor)->length;

		if (lengthFree >= size){
			nextBlock = startNodeCursor;
			blockFound = 1;
			break;
		}

		startNodeCursor = ((Block *)startNodeCursor)->next;
	}
	// if we havent found a free block we will search search from the head to the node we started with
	if (!blockFound){
		// now will loop from the head to the first node that we checked (cursor2)
		while(1){
			if (cursor == NULL || cursor >= startNode){
				break;
			}
			int lengthFree = ((Block*)cursor)->length;

			if (lengthFree >= size){
				nextBlock = cursor;
				blockFound = 1;
				break;
			}

			cursor = ((Block *)cursor)->next;
		}
	}


	if (blockFound)
	{
		excessSize = ((Block *)nextBlock)->length - size - FREE_BLOCK_HEADER_SIZE;
		//	Allocates the Memory Block
		allocate_block(nextBlock, size + FREE_BLOCK_HEADER_SIZE, excessSize, 1);
		return nextBlock + FREE_BLOCK_HEADER_SIZE;
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock = (void *)-2;
	}

	return nextBlock;
}

/**
 * 
 * 	This method will find the close tfreeBlock in the free block list to the last allocated memory 
 * 	spot given to the user.
 * 
 * @params:
 * 	 void *last : pointer to the last allocated memory location
 * 
 * @return:
 * 	 pointer to the next (higher) free block in the free block list.
 * 
 */
void *closestFreeBlock(void *last){
	if (last == NULL){
		return freeListHead;
	}

	void *cursor = freeListHead;

	while(1){
		if (cursor == NULL){
			break;
		}

		if (cursor >= last){
			return cursor;
		}

		cursor = ((Block *)cursor)->next;
	}
	return freeListHead;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation

	//	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
	//	Hint: Might want to have a minimum size greater than the Head/Tail sizes
	addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE;

	//	If excess free size is big enough
	if (addFreeBlock)
	{
		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
		Block b = createNode(excessSize,1,NULL,NULL);
		//excessFreeBlock = &b;
		excessFreeBlock = newBlock + size;
		*(Block *)excessFreeBlock = b;
		// excessFreeBlock += sizeof(b);
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock);
		}
		else
		{
			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);
		}
	}
	//	Otherwise add the excess memory to the new block
	else
	{
		//	TODO: Add excessSize to size and assign it to the new Block
		((Block *)newBlock)->length += excessSize;
		totalAllocatedSize += excessSize; 

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
	// store the next and previous of oldBlock in temporary pointers
	Block *temp1 = ((Block *)oldBlock)->previous;
	Block *temp2 = ((Block *)oldBlock)->next;

	if (oldBlock == freeListHead){
		freeListHead = newBlock;
		((Block *)freeListHead)->next = temp2;
		((Block *)freeListHead)->previous = temp1;
	} else {
		if (temp1 != NULL){
			temp1->next = newBlock;
		}
		if (temp2 != NULL){
			temp2->previous = newBlock;
		}
		((Block *)newBlock)->next = temp2;
		((Block *)newBlock)->previous = temp1;
	}
	
	//	TODO: Replace the old block with the new block
	int size = ((Block *)oldBlock)->length -= ((Block *)newBlock)->length;
	Block giveToUser = createNode(size, 0, NULL, NULL);
	*(Block *)oldBlock = giveToUser;

}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block)
{
	//	TODO: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).


	// this function is the simplified verison of adding to the free list that will only add to 
	// the list to replace the head, tail or add at the end

	if (freeListHead == NULL){
		freeListHead = block;
	}else if (freeListTail == NULL){
		if ((freeListHead + ((Block *)freeListHead)->length) == block){
			int s = ((Block *)block)->length;
			((Block *)freeListHead)->length += ((Block *)block)->length;
		}else {
			((Block *)freeListHead)->next = freeListTail;
			freeListTail = block;
			((Block *)freeListTail)->previous = freeListHead;
		}
	}else {
		// must check if the last node is contiguous with this one first
			Block temp = *(Block *)freeListTail;
			((Block *)freeListTail)->next = block;
			freeListTail = block;
			((Block *)freeListTail)->previous = &temp;

			// check if top block is > 128KB and if so deallocate untill there is only 128KB
			if (((Block *)freeListTail)->length > 128 * 1024){
				int excess = ((Block *)freeListTail)->length - (128 * 1024);
				((Block *)freeListTail)->length = 128 * 1024;
				sbrk(-excess);
				totalAllocatedSize -= excess;
			}
	}
}

/**
 * This function will deal with the hard task of adding a newly free'd block back 
 * into the free list.
 * 
 * This function is seperated from "add_block_freeList" since that wll only ever add excess block 
 * to the end of the list. In this fucntion however, we may create holes in our memory and will have
 * to deal with making sure they are being kept track of correctly
 * 
 * @params: 
 * 	- void *block : A pointer to the block of memory that wants to be freed. 
 * 					RECALL - the block of information (Block struct)
 * 					that has the length is stored <FREE_BLOCK_HEADER_SIZE> amount of space before the pointer
 */
void add_block_free_list_from_free(void *block)
{
	void *freeBlock = block - FREE_BLOCK_HEADER_SIZE;
	void *cursor = freeListHead;

	// loop through list to add new block into right spot. We might have to merge free blocks...
	while(1){
		if (cursor == NULL){
			// add it to end of list since it is not bigger than anything else
			add_block_freeList(block);
			break;
		}
		if (freeBlock < cursor){
			// check if the address of block is smaller than any existing free block 
			// to place it in the correct order

			void *next = ((Block*)cursor)->next;
			void *previous = ((Block*)cursor)->previous;

			// will check if the block chunks are contiguous and if so merge them
			if (freeBlock + (((Block *)freeBlock)->length) == cursor){
				Block mergedBlock = createNode(((Block *)freeBlock)->length + ((Block *)cursor)->length, 1, NULL, NULL);

				// must check that our merge did not go over 128KB limit
				// if it did, decrease its size and decrement the program break
				if (mergedBlock.length > 1024 * 128){
					int excess = mergedBlock.length - 1024*128;
					mergedBlock.length = 1024 * 128 - FREE_BLOCK_HEADER_SIZE;
					sbrk(-excess);
					totalAllocatedSize -= excess;
				}

				mergedBlock.next = next;
				mergedBlock.previous = previous;
				// will make freeBlock point to this merged block. Keep it mind it still hasn't been added to the list
				*(Block *)freeBlock = mergedBlock; 

				// Must add it to the list... so we make the next element in list point back to it
				if (next != NULL){
					((Block *)next)->previous = freeBlock;
				}
			}else {	// not contiguous so just create an intermediate node in list
				((Block *)freeBlock)->next = cursor;		
				((Block *)freeBlock)->previous = previous;
				((Block *)freeBlock)->tag = 1;
				((Block *)cursor)->previous = freeBlock;
			}
			
			// make previous element point to newly created freeBlock
			if (previous != NULL){
				((Block *)previous)->next = freeBlock;
			} else {
				// if the cursor does not have any previous node, it means we are at the head
				freeListHead = freeBlock;
			}
			break;
		}
		cursor = ((Block *)cursor)->next;
	}
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
	//	TODO: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.
	Block *temp1 = ((Block *)block)->previous;
	Block *temp2 = ((Block *)block)->next;

	if (temp1 != NULL){
		(((Block *)block)->previous)->next = temp2;
	}
	if (temp2 != NULL){
		(((Block *)block)->next)->previous = temp1;
	}
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
	//	Returns the deferenced size. 
	// Must amke sure to always pass the correct address to the block and not the free space
	return ((Block *)ptr)->length;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
	int largestBlockSize = 0;
	totalFreeSize = 0;

	//	TODO: Iterate through the Free Block List to find the largest free block and return its size
	void *cursor = freeListHead;

	while(1){
		if (cursor == NULL){
			break;
		}

		if (((Block *)cursor)->length > largestBlockSize)
			largestBlockSize = ((Block *)cursor)->length;
		
		totalFreeSize += ((Block *)cursor)->length;

		cursor = ((Block *)cursor)->next;
	}

	return largestBlockSize;
}

/**
 * This function acts as a constructor for Block struct 
 */
Block createNode(int length, int tag, Block *next, Block *prev){
	Block newBlock;
	newBlock.length = length;
	newBlock.tag = tag;
	newBlock.next = next;
	newBlock.previous = prev;

	return newBlock;
}