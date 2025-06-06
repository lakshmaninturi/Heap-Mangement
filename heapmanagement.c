//Heap Management Simulation (Best Fit Method)
//By- Lakshman Uday Kiran Inturi  (BT23CSE071)

#include <stdio.h>
#include <stddef.h>

typedef enum {FALSE,TRUE} Bool;
typedef struct MetaDataBlock{
    size_t size;
    struct MetaDataBlock *next;
} MetaDataBlock;




size_t const sizeOfMetadata= sizeof(MetaDataBlock);
#define MAX_HEAP_SIZE 1048576+sizeof(MetaDataBlock)
//for using examples below, set the max heap size to 100+sizeof(MetaDataBlock)
#define MIN_ALLOC_SIZE sizeOfMetadata+sizeof(int)

char Heap[MAX_HEAP_SIZE];

Bool heapInitialized = FALSE;
MetaDataBlock *freeListHead = (MetaDataBlock *)Heap;
MetaDataBlock *allocatedListHead = NULL;

size_t findNearestMultipleOf4(size_t size);
MetaDataBlock* mergeCurrentAndNext(MetaDataBlock* currentBlock, MetaDataBlock* nextBlock);
MetaDataBlock* mergePrevAndCurrent(MetaDataBlock* prevBlock, MetaDataBlock* currentBlock);

void initializeHeap(){
    if(!heapInitialized){
        freeListHead->size = MAX_HEAP_SIZE - sizeOfMetadata;
        freeListHead->next = NULL;
        heapInitialized = TRUE;
    }
}

void printMetaDataList(MetaDataBlock *head);

void * my_malloc(size_t sizeRequest){
    void * return_ptr=NULL;
    if(!heapInitialized){
        initializeHeap();
    }
    if(sizeRequest==0){
        printf("\nError: Couldn't allocate block of zero size");
    }
    else{
        //go throught the free list and search for empty block
        MetaDataBlock *current = freeListHead;
        MetaDataBlock *prev = NULL;
        Bool sizeFound = FALSE;
        size_t minSize = MAX_HEAP_SIZE;
        MetaDataBlock *minBlock = NULL;
        MetaDataBlock *prevMinBlock = NULL;
        while(current){
            if(current->size >= sizeRequest){
                if(current->size < minSize){
                    minSize = current->size;
                    minBlock = current;
                    prevMinBlock = prev;
                }
            }
            prev = current;
            current = current->next;
        }
        if(minBlock==NULL){
            printf("\nError: Couldn't find a block of size %d", sizeRequest);
        }
        else{
            size_t sizeToAllocate = findNearestMultipleOf4(sizeRequest);
            //now we have to decide to split the block or not
            //min data size will be sie of int
            //MIN_ALLOC_SIZE(8+4=12)
            if(minBlock->size - sizeToAllocate >= MIN_ALLOC_SIZE){
                MetaDataBlock *newBlock = (MetaDataBlock *)((void *)minBlock + sizeOfMetadata+sizeToAllocate);
                newBlock->size = minBlock->size - sizeToAllocate-sizeOfMetadata;
                newBlock->next = minBlock->next;
                minBlock->size = sizeToAllocate;
                minBlock->next = NULL;
                return_ptr = (void *)minBlock + sizeOfMetadata;
                if(prevMinBlock==NULL){
                    freeListHead = newBlock;
                }
                else{
                    prevMinBlock->next = newBlock;
                }
            }
            else{
                if(prevMinBlock==NULL){
                    freeListHead = minBlock->next;
                }
                else{
                    prevMinBlock->next = minBlock->next;
                }
                // minBlock->size = sizeToAllocate;
                minBlock->next = NULL;//this block is allocated so setting its next pointer to null
                return_ptr = (void *)minBlock + sizeOfMetadata;
            }
        }
    }
    return return_ptr;
    
}

void my_free(void * freeDataPtr){
    if(freeDataPtr==NULL){
        printf("\nError: Cannot free Null pointer\n");
    }
    else if(freeDataPtr<(void*)Heap || freeDataPtr>(void*)Heap+MAX_HEAP_SIZE){
        printf("\nError: Pointer outside Heap bounds(Failed to free up Memory)");
    }
    else{
        MetaDataBlock* currentBlockToFree=(MetaDataBlock*)(freeDataPtr-sizeOfMetadata);
        printf("\nFree MetaData block ptr to free up: %p",currentBlockToFree);
        printf("\nsize of data to free up: %d",currentBlockToFree->size);

        //finding previous free block and next free block to merge
        MetaDataBlock* prevFreeBlock=NULL;
        MetaDataBlock* nextFreeBlock=freeListHead;

        while(nextFreeBlock!=NULL  && currentBlockToFree>nextFreeBlock){
            prevFreeBlock=nextFreeBlock;
            nextFreeBlock=nextFreeBlock->next;
        }
        //now we have the two free blocks to left and right of the current block to be freed
        if(freeListHead==NULL || (prevFreeBlock==NULL && nextFreeBlock==NULL)){
        //1 freeList head is null or prev and next both are null -> free lsit is empty, so we make the current block to be free block
        printf("\nType 1 freeing");
            freeListHead=currentBlockToFree;
            currentBlockToFree->next=NULL;
        }
        else if(prevFreeBlock==NULL && nextFreeBlock!=NULL){
            printf("\nType 2 freeing");
        //2 prev is null and next is not
        // this means insert at start , in this two cases
        // case 1: to merge -> (void *)currentBlock + metadata size + currentBlock->size == nextFreeBlock now we have to merge 
        //case 2: else dont merge just add it to start of free list and update free list head
            if((void*) currentBlockToFree+ sizeOfMetadata+currentBlockToFree->size==nextFreeBlock){
                printf("\ncurrent and next block Merging required\n");
                MetaDataBlock* mergedBlockPtr= mergeCurrentAndNext(currentBlockToFree,nextFreeBlock);
                freeListHead=mergedBlockPtr;
            }
            else{
                printf("\nNo merging required\n");//just add it to the beggining of free list and update its next ptr
                currentBlockToFree->next=nextFreeBlock;
                freeListHead=currentBlockToFree;
            }
        }
        else if(prevFreeBlock!=NULL && nextFreeBlock==NULL){
            printf("\nType 3 freeing");
        //3 prev is not null and next is null >> insert at end
        //case 1: to merge-> (void*)prevBlock +metadata+ prevBlock-> size== currentBlock, then merge
        // case 2: else dont merge and just add it to the free list at the end
            if((void*)prevFreeBlock+sizeOfMetadata+prevFreeBlock->size==currentBlockToFree){
                printf("\nprevious and current block Merging required");
                MetaDataBlock* mergedBlockPtr= mergePrevAndCurrent(prevFreeBlock,currentBlockToFree);
            }
            else{
                printf("\nNo merging required");
                currentBlockToFree->next=NULL;
                prevFreeBlock->next=currentBlockToFree;
            }
        }
        else if(prevFreeBlock!=NULL && nextFreeBlock!=NULL){
            printf("\nType 4 freeing");
            //check if left merge is possible then merge left and current block
            MetaDataBlock* newLeftBlock=NULL;
            if((void*)prevFreeBlock+sizeOfMetadata+prevFreeBlock->size==currentBlockToFree){
                printf("\nprevious and current block Merging required");
                newLeftBlock= mergePrevAndCurrent(prevFreeBlock,currentBlockToFree);
            }
            else{
                printf("\nNo merging required");//just join prev and current block
                prevFreeBlock->next=currentBlockToFree;
                newLeftBlock=currentBlockToFree;
            }
            //check if right merge is possible then merge newLeft and right block
            if((void*)newLeftBlock+sizeOfMetadata+newLeftBlock->size==nextFreeBlock){
                printf("\nnew left and next block Merging required");
                MetaDataBlock* mergedBlockPtr= mergeCurrentAndNext(newLeftBlock,nextFreeBlock);
            }
            else{
                printf("\nNo merging required\n");
                newLeftBlock->next=nextFreeBlock;
            }
        }
        printf("\nFreed up Block\n");
    }
}

MetaDataBlock* mergeCurrentAndNext(MetaDataBlock* currentBlock, MetaDataBlock* nextBlock){
    MetaDataBlock* ret_ptr=currentBlock;
    currentBlock->next=nextBlock->next;
    nextBlock->next=NULL;
    // now merge
    currentBlock->size=currentBlock->size+sizeOfMetadata+nextBlock->size;
    return ret_ptr;
}

MetaDataBlock* mergePrevAndCurrent(MetaDataBlock* prevBlock, MetaDataBlock* currentBlock){
    MetaDataBlock* ret_ptr=prevBlock;
    //ensuring prevBlock next pointer is preserved we shouldnt do prevBlock->next=currentBlock->next
    currentBlock->next=NULL;
    //now merge
    prevBlock->size=prevBlock->size+sizeOfMetadata+currentBlock->size;
    return ret_ptr;
}

void printMetaDataList(MetaDataBlock *head){
    int freeBlockcount=0;
    MetaDataBlock *current = head;
    if(current==NULL){
        printf("\nNo free blocks in the list");
    }
    while(current){
        ++freeBlockcount;
        printf("\nAddress of Free block: %p", current);
        printf("\nSize of Free block: %d", current->size);
        current = current->next;
    }
    printf("\nNumber of free blocks: %d",freeBlockcount);
}

size_t findNearestMultipleOf4(size_t size){
    size_t ret_val=size;
    if(size%4!=0){
        ret_val = size + (4 - size%4);
    }
    return ret_val;
}

void Example1(){
    printf("\n\tExample 1");
    //allocating 50 bytes 
    int * firstAllocatedBlock=(int*) my_malloc(50);//as 50  bytes takes 52 bytes to store, padding acc to multiple of 4
    printf("\nAfter first allocation of 50 bytes");
    printMetaDataList(freeListHead);
    //then 40 bytes to make heap full
    int * secondAllocatedBlock=(int*) my_malloc(40);
    printf("\n\nAfter second allocation of 40 bytes");
    printMetaDataList(freeListHead);
    // and then releasing the 40 bytes and check status of heap
    //freeing blocks
    my_free(secondAllocatedBlock);
    secondAllocatedBlock=NULL;
    printf("\n\nAfter freeing second block of 40 bytes");
    printMetaDataList(freeListHead);

    my_free(firstAllocatedBlock);
    firstAllocatedBlock=NULL;
    printf("\n\nAfter freeing first block of 50 bytes");
    printMetaDataList(freeListHead);
}

void Example2(){
    printf("\n\tExample 2");
    //allocating 50 bytes 
    int * firstAllocatedBlock=(int*) my_malloc(50);//as 50  bytes takes 52 bytes to store, padding acc to multiple of 4
    printf("\nAfter first allocation of 50 bytes");
    printMetaDataList(freeListHead);
    //then 40 bytes to make heap full
    int * secondAllocatedBlock=(int*) my_malloc(40);
    printf("\n\nAfter second allocation of 40 bytes");
    printMetaDataList(freeListHead);
    // and then releasing the 50 bytes and check status of heap
    //freeing blocks
    my_free(firstAllocatedBlock);
    firstAllocatedBlock=NULL;
    printf("\n\nAfter freeing first block of 50 bytes");
    printMetaDataList(freeListHead);

    my_free(secondAllocatedBlock);
    secondAllocatedBlock=NULL;
    printf("\n\nAfter freeing second block of 50 bytes");
    printMetaDataList(freeListHead);
}

void Example3(){
    int* block1=(int*) my_malloc(12);
    int* block2=(int*) my_malloc(12);
    int* block3=(int*) my_malloc(12);
    int* block4=(int*) my_malloc(12);
    int* block5=(int*) my_malloc(12);
    printf("\n\n After initial 5 allocations");
    printMetaDataList(freeListHead);
    printf("\n\n Freeing up 1st and 5th blocks");
    my_free(block1);
    my_free(block5);
    printMetaDataList(freeListHead);
    //now freeing the third block
    printf("\n\n Freeing up 3rd block");
    my_free(block3);
    printMetaDataList(freeListHead);

    //freeing 2nd block
    printf("\n\n Freeing up 2nd block");
    my_free(block2);
    printMetaDataList(freeListHead);

    printf("\n\n Freeing up 4th block");
    my_free(block4);
    printMetaDataList(freeListHead);
}

void Example4(){
    int* block1=(int*) my_malloc(40);
    printMetaDataList(freeListHead);
    int* block2=(int*) my_malloc(20);
    printMetaDataList(freeListHead);
    int* block3=(int*) my_malloc(20);
    printMetaDataList(freeListHead);

    printf("\n\n After initial 3 allocations");
    printMetaDataList(freeListHead);
    printf("\n\n Freeing up 1st block");
    my_free(block1);
    printMetaDataList(freeListHead);
    printf("\n\n Freeing up 3rd block");
    my_free(block3);
    printMetaDataList(freeListHead);
    printf("\n\n Freeing up 2nd block");
    my_free(block2);
    printMetaDataList(freeListHead);

}

int main() {  
    initializeHeap();
    printf("\nInitial state of heap\n\n");
    printMetaDataList(freeListHead);
   // The below examples are to test the heap Management for max heap size of 100 bytes, so set the max heap size to 100 bytes, before using the examples
     int* a=(int*) my_malloc(sizeof(int));
    printMetaDataList(freeListHead);
    Example1();
     Example2();
     Example3();
     Example4();

    return 0;
}