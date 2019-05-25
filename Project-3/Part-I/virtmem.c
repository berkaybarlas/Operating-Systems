/**
 * virtmem.c 
 * Written by Michael Ballantyne 
 * Modified by Didem Unat
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TLB_SIZE 16
#define PAGES 256
#define PAGE_FRAMES 64
#define PAGE_MASK 255

#define PAGE_SIZE 256
#define OFFSET_BITS 8
#define OFFSET_MASK 255

#define MEMORY_SIZE PAGE_FRAMES * PAGE_SIZE
#define BACKING_SIZE PAGES * PAGE_SIZE

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

struct tlbentry {
  unsigned char logical;
  unsigned char physical;
};

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];
// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;

// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
int pagetable[PAGES];

signed char main_memory[MEMORY_SIZE];

// Pointer to memory mapped backing file
signed char *backing;

// Algorithms to return page frame to write on with different strategies
// FIFO algorithm for page replacement algorithm that uses modulo
int fifoPageSelect(unsigned char *free_page){
	int selectedPage = *free_page;
	*free_page = (*free_page + 1) % PAGE_FRAMES;
	return selectedPage;
}
// LRU  algorithm for page replacement algorithm that uses counting table
int lruPageSelect(int* pageRefTbl, int logical_page, int pageFault) {
  
  // If the pageFault number is less than page frames there is no need for page replacement
  // which means there are still space for new pages without replacement
  if(pageFault <= PAGE_FRAMES) {
    return pageFault - 1;
  }

  int i;
  int minIndex = 0;
  int minValue = 0;
  for(i = 0; i < PAGES; i++) {
    if(pagetable[i] != -1) {
      if(minValue == 0 || minValue > pageRefTbl[i]) {
        minIndex = i;
        minValue = pageRefTbl[i];
        
      }
    }
  }
return pagetable[minIndex]; 
}

// Copies page from backing to memory and updates the page table 
void putPageInMemory(int logical_page, int physical_page){
	 memcpy(main_memory + physical_page * PAGE_SIZE, backing + logical_page * PAGE_SIZE, PAGE_SIZE);
	 for(int i = 0; i < PAGES; i++){
	 	if(pagetable[i] == physical_page){
	 		pagetable[i] = -1;
	 	}
	 }
	 pagetable[logical_page] = physical_page;
}

int max(int a, int b)
{
  if (a > b)
    return a;
  return b;
}

/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(unsigned char logical_page) {
  int i;
  for (i = max((tlbindex - TLB_SIZE), 0); i < tlbindex; i++) {
    struct tlbentry *entry = &tlb[i % TLB_SIZE];
    
    if (entry->logical == logical_page) {
      return entry->physical;
    }
  }
  
  return -1;
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(unsigned char logical, unsigned char physical) {
  struct tlbentry *entry = &tlb[tlbindex % TLB_SIZE];
  tlbindex++;
  entry->logical = logical;
  entry->physical = physical;
}

// Take command line arguments 
// The only existing flag is p
void cmdline(int argc, char **argv, int *p) {
  int opt = 0;
  opterr = 0;
  while (optind < argc) {
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            *p = atoi(optarg);
            printf("p:%d\n",*p);
            return;
        default: /* 'Error' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
        
    }
    optind++;
  }
}

int main(int argc, char **argv)
{
  if (argc < 3) {
    fprintf(stderr, "Usage ./virtmem backingstore input %d\n", argc);
    exit(1);
  }
  int p = 0;
  cmdline(argc, argv, &p);
  const char *backing_filename = argv[1]; 
  int backing_fd = open(backing_filename, O_RDONLY);
  backing = mmap(0, BACKING_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0); 
  
  const char *input_filename = argv[2];
  FILE *input_fp = fopen(input_filename, "r");
  
  // Fill page table entries with -1 for initially empty table.
  int i;
  for (i = 0; i < PAGES; i++) {
    pagetable[i] = -1;
  }
  
  // Character buffer for reading lines of input file.
  char buffer[BUFFER_SIZE];
  
  // Data we need to keep track of to compute stats at end.
  int total_addresses = 0;
  int tlb_hits = 0;
  int page_faults = 0;
  
  // Number of the next unallocated physical page in main memory
  unsigned char free_page = 0;
  
  // Table containing last reference times of each page, initially all zero
  int pageRefTbl[PAGES] = {0};
  
  while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL) {
    total_addresses++;
    int logical_address = atoi(buffer);
    
    int offset = logical_address & OFFSET_MASK;
    int logical_page = (logical_address >> OFFSET_BITS) & PAGE_MASK;
    
    // Use total addresses as a counting number in page referance table
    if(p == 1) { 
      pageRefTbl[logical_page] = total_addresses;
    }

    int physical_page = search_tlb(logical_page);
    
    if (physical_page != -1) {
      // TLB hit
      tlb_hits++;
      
    } else {
      // TLB miss

      physical_page = pagetable[logical_page];
      
      if (physical_page == -1) {
        // Page fault
        // Increase unique total Address
        page_faults++;
        if(p == 0){
        	physical_page = fifoPageSelect(&free_page);
        } else if (p == 1) {
          physical_page = lruPageSelect(pageRefTbl, logical_page, page_faults);
        }
        // Copy page from backing file into physical memory
        putPageInMemory(logical_page, physical_page);
      }
      
      add_to_tlb(logical_page, physical_page);
    }
    int physical_address = (physical_page << OFFSET_BITS) | offset;
    signed char value = main_memory[physical_page * PAGE_SIZE + offset];
    
    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, value);
  }
  
  printf("Number of Translated Addresses = %d\n", total_addresses);
  printf("Page Faults = %d\n", page_faults);
  printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
  printf("TLB Hits = %d\n", tlb_hits);
  printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));
  
  return 0;
}
