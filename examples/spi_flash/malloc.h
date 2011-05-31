#define HEAPSTART    0x00300000  // put this above FLASH_BUFFER 
#define HEAPSIZE     0x00C00000  // 12MB for now - leave 1MB for JPEG buffer


char *malloc(unsigned int);
void free(char *);
void init_heap();



typedef int ALIGN;
union header {              // free block header
    struct {
        union header *ptr;  // next free block
        unsigned int size;  // size of this free block
    } s;
    ALIGN x;                // forces alignment of blocks
};

typedef union header HEADER;
extern char *heap_start, *heap_ptr, *heap_end;
extern HEADER base;         // pointer to beginning of free block list
extern HEADER *allocp;      // last allocated block
HEADER *morecore(unsigned int);
