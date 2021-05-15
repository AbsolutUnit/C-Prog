/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"

int began = 1;
int prolg_size = 32;

void init_free_lists(){
    for(int i = 0; i < NUM_FREE_LISTS; i++){
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];         // Initialize the next for sentinel
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];         // Initialize prev for sentinel
    }
}

int get_index(int size1){
    int m = 32;
    if (size1 == 32){
        return 0;
    } else if (size1 <= 2 * m){
        return 1;
    } else if (size1 <= 4 * m){
        return 2;
    } else if (size1 <= 8 * m){
        return 3;
    } else if (size1 <= 16 * m){
        return 4;
    } else if (size1 <= 32 * m){
        return 5;
    } else {
        return 6;
    }
}

void init_heap(){
    init_free_lists();                                  // Initialize wilderness block
    sf_mem_grow();                              // Initialize the heap by making new page
    sf_block *prolg = (sf_block *)(sf_mem_start() + 8);           // Set prologue block
    prolg->header = 32;
    prolg->header = prolg->header | THIS_BLOCK_ALLOCATED;
    sf_header *epil = (sf_header *)(sf_mem_end() - 8);         // Set epilogue header
    *epil = 0;
    sf_block *wild = (sf_block *)((void *)(prolg) + prolg_size);        // Construct wilderness block
    wild->header = PAGE_SZ - 48;
    wild->header = wild->header | PREV_BLOCK_ALLOCATED;
    int head = wild->header;
    sf_footer *foot = (sf_footer *)((void *)(wild) + PAGE_SZ - 48 - 8);
    *foot = head;
    sf_free_list_heads[7].body.links.next = wild;
    sf_free_list_heads[7].body.links.prev = wild;
    wild->body.links.next = &sf_free_list_heads[7];
    wild->body.links.prev = &sf_free_list_heads[7];      // Link sentinel of wilderness to wild block
}

void *split_block(int i, size_t siz){
    if (i == 7){
        int prev_flag = 0;                                         // Flag to track if prev_block allocated
        int len = sf_free_list_heads[i].body.links.next->header;    // Variable to track curr leng
        if (len & PREV_BLOCK_ALLOCATED){
            len -= PREV_BLOCK_ALLOCATED;
            prev_flag = 1;                                          // Setting prev flag
        }
        void *current = (void *)sf_free_list_heads[i].body.links.next;      // Mem address of current
        sf_block *new_block = (sf_block *)(current + siz);        // Create new block at offset
        sf_block *curr_block = (sf_block *)(current);         // Recreate current block
        curr_block->header = siz;                                           // Set new current header
        curr_block->header = curr_block->header | THIS_BLOCK_ALLOCATED;     // Set Alloc Bit
        if (prev_flag){
            curr_block->header = curr_block->header | PREV_BLOCK_ALLOCATED;     // Set Prev Bit if needed
        }
        new_block->header = len - siz;                                  // Set new block header to diff
        sf_header *coal = (sf_header *)((void *)(new_block) + (len - siz));
        int extend = 0;
        if (!(*coal & THIS_BLOCK_ALLOCATED)){
            extend = *coal;
            if (extend & PREV_BLOCK_ALLOCATED){
                extend -= PREV_BLOCK_ALLOCATED;
            }
        } else {
            if (*coal & PREV_BLOCK_ALLOCATED){
                *coal -= PREV_BLOCK_ALLOCATED;
            }
        }
        int tot = new_block->header + extend;
        new_block->header = new_block->header | PREV_BLOCK_ALLOCATED;   // Set prev bit
        int head = new_block->header;                                   // Copy header for footer
        sf_footer *foot = (sf_footer *)((void *)(new_block) + tot - 8);            // Find footer mem location
        *foot = head;                                               // Set footer to header temp
        sf_free_list_heads[i].body.links.next = sf_free_list_heads[i].body.links.prev = new_block;
        new_block->body.links.next = new_block->body.links.prev = &sf_free_list_heads[i];
        return ((void *)(curr_block) + 8);
    } else {
        int prev_flag = 0;                                         // Flag to track if prev_block allocated
        int len = sf_free_list_heads[i].body.links.next->header;    // Variable to track curr leng
        if (len & PREV_BLOCK_ALLOCATED){
            len -= PREV_BLOCK_ALLOCATED;
            prev_flag = 1;                                          // Setting prev flag
        }
        void *current = (void *)sf_free_list_heads[i].body.links.next;      // Mem address of current
        sf_block *new_block = (sf_block *)(void *)(current + siz);        // Create new block at offset
        sf_block *curr_block = (sf_block *)(void *)(current);         // Recreate current block
        curr_block->header = siz;                                           // Set new current header
        curr_block->header = curr_block->header | THIS_BLOCK_ALLOCATED;     // Set Alloc Bit
        if (prev_flag){
            curr_block->header = curr_block->header | PREV_BLOCK_ALLOCATED;     // Set Prev Bit if needed
        }
        new_block->header = len - siz;                                  // Set new block header to diff
        sf_header *coal = (sf_header *)((void *)(new_block) + (len - siz));
        int extend = 0;
        if (!(*coal & THIS_BLOCK_ALLOCATED)){
            extend = *coal;
            if (extend & PREV_BLOCK_ALLOCATED){
                extend -= PREV_BLOCK_ALLOCATED;
            }
        } else {
            if (*coal & PREV_BLOCK_ALLOCATED){
                *coal -= PREV_BLOCK_ALLOCATED;
            }
        }
        int ind_flag = 0;
        if (extend > 0 && (void *)(coal) == sf_free_list_heads[7].body.links.next){
            ind_flag = 1;
        }
        new_block->header = new_block->header | PREV_BLOCK_ALLOCATED;   // Set prev bit
        int head = new_block->header + extend;                                   // Copy header for footer
        sf_footer *foot = (void *)(new_block + (len - siz + extend) - 8);            // Find footer mem location
        *foot = head;                                               // Set footer to header temp
        int index = get_index((int)(len - siz + extend));
        if (ind_flag){
            index = 7;
            sf_free_list_heads[7].body.links.next = sf_free_list_heads[7].body.links.prev = &sf_free_list_heads[7];
        }
        new_block->body.links.next = sf_free_list_heads[index].body.links.next;     // Set new block prev to current prev
        new_block->body.links.prev = &sf_free_list_heads[index];                    // Set new block next to sentinel
        sf_free_list_heads[index].body.links.next->body.links.prev = new_block;     // Set previous prev's next to new_block
        sf_free_list_heads[index].body.links.next = new_block;                      // Set sentinel's prev to new_block
        return ((void *)(curr_block) + 8);
    }
}

void *split_block_alloc(void *johnny, int size){
    sf_block *john = (sf_block *)(johnny - 8);
    int prev_flag = 0;
    int size1 = john->header;
    if (size1 & PREV_BLOCK_ALLOCATED){
        prev_flag = 1;
        size1 -= PREV_BLOCK_ALLOCATED;
    }
    if (size1 & THIS_BLOCK_ALLOCATED){
        size1 -= THIS_BLOCK_ALLOCATED;
    }
    sf_block *new_split = (sf_block *)(john);
    new_split->header = size;
    new_split->header += THIS_BLOCK_ALLOCATED;
    if (prev_flag){
        new_split->header += PREV_BLOCK_ALLOCATED;
    }
    sf_block *new_block = (sf_block *)((void*)(john) + size);
    new_block->header = size1 - size;
    sf_header *coal = (sf_header *)((void *)(new_block) + (size1 - size));
    int extend = 0;
    if (!(*coal & THIS_BLOCK_ALLOCATED)){
        extend = *coal;
        if (extend & PREV_BLOCK_ALLOCATED){
            extend -= PREV_BLOCK_ALLOCATED;
        }
    } else {
        if (*coal & PREV_BLOCK_ALLOCATED){
            *coal -= PREV_BLOCK_ALLOCATED;
        }
    }
    int ind_flag = 0;
    if (extend > 0 && (void *)(coal) == sf_free_list_heads[7].body.links.next){
        ind_flag = 1;
    }
    int offset = size1 - size + extend;
    new_block->header += extend;
    new_block->header = new_block->header | PREV_BLOCK_ALLOCATED;
    sf_footer *foot = (sf_footer *)((void *)(new_block) + offset - 8);
    *foot = new_block->header;
    int index = get_index((int)(size1 - size + extend));
    if (ind_flag){
        index = 7;
        sf_free_list_heads[7].body.links.next = sf_free_list_heads[7].body.links.prev = &sf_free_list_heads[7];
    }
    new_block->body.links.next = sf_free_list_heads[index].body.links.next;     // Set new block prev to current prev
    new_block->body.links.prev = &sf_free_list_heads[index];                    // Set new block next to sentinel
    sf_free_list_heads[index].body.links.next->body.links.prev = new_block;     // Set previous prev's next to new_block
    sf_free_list_heads[index].body.links.next = new_block;                      // Set sentinel's prev to new_block
    return ((void *)(new_split) + 8);
}

int wild_check(size_t j){
    if (j > sf_free_list_heads[7].body.links.next->header){             // Check if alloc size is greater than current wild
        return 1;                                                       // If so, return True
    } else {
        return 0;                                                       // Else return False
    }
}

void *gen_mem(){
    if (sf_mem_grow() == NULL){                                         // Call mem_grow, if Null check
        sf_errno = ENOMEM;                                                  // Set ENOMEM
        return NULL;                                                    // Return Null
    }
    int prev_flag = 0;
    void *wild = sf_free_list_heads[7].body.links.next;                 // Get mem address of wild
    int siz = sf_free_list_heads[7].body.links.next->header;
    if (siz & PREV_BLOCK_ALLOCATED){                                    // Flag management for header
        siz -= PREV_BLOCK_ALLOCATED;
        prev_flag = 1;
    }
    sf_block *new_wild = wild;                                          // Construct new wild block
    new_wild->header = siz + PAGE_SZ;                                   // Set new size
    if (prev_flag){
        new_wild->header += PREV_BLOCK_ALLOCATED;                       // and prev alloc if necessary
    }
    int head = new_wild->header;
    sf_footer *foot = (sf_footer *)(void*)(sf_mem_end() - 16);          // Set footer as well
    *foot = head;
    sf_footer *epilg = (sf_footer *)(void *)(sf_mem_end() - 8);         // And epilogue
    *epilg = 0;
    sf_free_list_heads[7].body.links.next = new_wild;
    sf_free_list_heads[7].body.links.prev = new_wild;
    new_wild->body.links.next = &sf_free_list_heads[7];                 // Reset sentinel node of wild index
    new_wild->body.links.prev = &sf_free_list_heads[7];
    return new_wild;
}

void *norm_alloc(int i){
    sf_block *curr = sf_free_list_heads[i].body.links.next;             // Non split allocation, grab block
    curr->header = curr->header | THIS_BLOCK_ALLOCATED;                 // Set the block to allocated
    sf_free_list_heads[i].body.links.next->body.links.next->body.links.prev = &sf_free_list_heads[i];
    sf_free_list_heads[i].body.links.next = sf_free_list_heads[i].body.links.next->body.links.next;
    if (sf_free_list_heads[i].body.links.prev == curr){                 // Reset sentinel links and check if it's only val
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
    return ((void *)(curr) + 8);                                                // Return address of block
}

void *sf_malloc(size_t size) {
    if (size == 0){                                                 // If invalid malloc call
        return NULL;
    }
    void *answer = NULL;
    if (began){                                                     // Init heap if not init
        init_heap();
        began = 0;
    }
    int val = size + 8;                                                 // Grab malloc val
    if (val < 32){
        val = 32;
    }
    if (val % 16 != 0){                                             // Round to next 32 if not a multiple already
        val = (val + (16 - val % 16));
    }
    int ind = get_index(val);
    for(int i = ind; i < NUM_FREE_LISTS; i++){                      // Loop through free lists
        if (sf_free_list_heads[i].body.links.next != &sf_free_list_heads[i]){       // Check if not empty
            if (i == 7){                                            // Check if dealing with wilderness
                if (wild_check(val)){                               // If value greater than wild size
                    while (wild_check(val)){                        // Begin while iteration
                        void *conc = gen_mem();
                        if (conc == NULL){                          // If sf_mem_grow returns NULL
                            return NULL;
                        }
                    }
                    if (sf_free_list_heads[i].body.links.next->header - val >= 32){     // If new wild is too big
                        answer = split_block(i, val);               // Split that thing
                        break;
                    } else {
                        answer = norm_alloc(i);                     // Or don't
                        break;
                    }
                } else {
                    if (sf_free_list_heads[i].body.links.next->header - val >= 32) {    // If wild is too big
                        answer = split_block(i, val);               // Split that thing
                        break;
                    } else {
                        answer = norm_alloc(i);                     // Or just don't
                        break;
                    }
                }
            } else {
                if (sf_free_list_heads[i].body.links.next->header - val >= 32) {    // repeat code
                    answer = split_block(i, val);
                } else {
                    answer = norm_alloc(i);
                }
            }
        }
    }
    return answer;              // Finally return the pointer to the new alloc
}

int next_free(sf_header *duh){
    if (*duh & THIS_BLOCK_ALLOCATED){
        return 0;
    }
    return 1;
}

void pre_check(void *pp){
    if (pp == NULL){
        abort();
    }
    sf_block *head = (sf_block *)((void *)(pp) - 8);
    int size = head->header;
    if (size & THIS_BLOCK_ALLOCATED){
        size -= THIS_BLOCK_ALLOCATED;
    }
    if (size & PREV_BLOCK_ALLOCATED){
        size -= PREV_BLOCK_ALLOCATED;
    }
    if (((unsigned long)pp & 15) != 0){
        abort();
    }
    if (size < 32 || size % 16 != 0){
        abort();
    }
    if (!(head->header & THIS_BLOCK_ALLOCATED)){
        abort();
    }
    if (((void *)(head) + size) > sf_mem_end() || ((void *)(head) + size + 8) > sf_mem_end()){
        abort();
    }
    if (head->header & PREV_BLOCK_ALLOCATED){
        sf_footer *prev = (sf_footer *)((void *)(head) - 8);
        int val = *prev;
        if (val & THIS_BLOCK_ALLOCATED){
            val -= THIS_BLOCK_ALLOCATED;
        }
        if (val & PREV_BLOCK_ALLOCATED){
            val -= PREV_BLOCK_ALLOCATED;
        }
        sf_header *prev_header = (sf_header *)((void *)(head) - val);
        if (!(*prev_header & THIS_BLOCK_ALLOCATED)){
            abort();
        }
    } else {
        sf_footer *prev = (sf_footer *)((void *)(head) - 8);
        int val = *prev;
        if (val & THIS_BLOCK_ALLOCATED){
            val -= THIS_BLOCK_ALLOCATED;
        }
        if (val & PREV_BLOCK_ALLOCATED){
            val -= PREV_BLOCK_ALLOCATED;
        }
        sf_header *prev_header = (sf_header *)((void *)(prev) - val);
        if (*prev_header & THIS_BLOCK_ALLOCATED){
            abort();
        }
    }
}

void free_list_free(sf_block *ptr){
    ptr->body.links.prev->body.links.next = ptr->body.links.next;
    ptr->body.links.next->body.links.prev = ptr->body.links.prev;
}

void sf_free(void *pp) {
    pre_check(pp);
    sf_block *curr = (sf_block *)(pp - 8);

    int prev_flag = 1;

    curr->header -= THIS_BLOCK_ALLOCATED;                   // subtract the alloc bit from the header
    int siz = curr->header;                             // Set size variable for the current size of the block
    void *bruh = (void *)(curr);
    if (siz & PREV_BLOCK_ALLOCATED){                    // Check for previous allocation
        siz -= PREV_BLOCK_ALLOCATED;
        prev_flag = 0;
    }

    int next_flag = -1;
    if (((void *)(bruh) + siz) == sf_mem_end() - 8) {
        next_flag = 0;
    } else {
        sf_block *next = (sf_block *)(bruh + siz);        // Grab memory address of the next block
        int val_holder = next->header;
        if (val_holder & THIS_BLOCK_ALLOCATED){
            next_flag = 0;
        } else {
            next_flag = 1;
        }
    }
    if (next_flag && prev_flag) {
        int wild_flag = 0;
        sf_header *next = (sf_header *)(bruh + siz);        // Grab memory address of the next block
        int siz1 = *((sf_footer *)((void *)(curr) - 8));  // Get size and parse from prev footer
        int siz2 = *(next);                             // Get size and parse from next header
        int prev_prev_flag = 0;
        if (siz1 & PREV_BLOCK_ALLOCATED){               // Subtract if PREV ALLOC
            siz1 -= PREV_BLOCK_ALLOCATED;
            prev_prev_flag = 1;
        }
        if (siz2 & PREV_BLOCK_ALLOCATED){               // Subtract if PREV ALLOC
            siz2 -= PREV_BLOCK_ALLOCATED;
        }
        sf_block *prev = (sf_block *)(((void *)curr) - siz1);                   // Construct previous block
        free_list_free(prev);
        sf_block *nextb = (sf_block *)(void *)(next);
        if (nextb == sf_free_list_heads[7].body.links.next){
            wild_flag = 1;
        }
        free_list_free(nextb);
        prev->header = siz1 + siz2 + siz;
        int tot = prev->header;
        if (prev_prev_flag){
            prev->header += PREV_BLOCK_ALLOCATED;
        }
        sf_footer *foot = ((void *)(prev) + tot - 8);
        *foot = prev->header;
        if (wild_flag){
            sf_free_list_heads[7].body.links.next = prev;
            sf_free_list_heads[7].body.links.prev = prev;
            prev->body.links.next = &sf_free_list_heads[7];
            prev->body.links.prev = &sf_free_list_heads[7];
        } else {
            int ind = get_index(siz + siz1 + siz2);
            prev->body.links.next = sf_free_list_heads[ind].body.links.next;
            sf_free_list_heads[ind].body.links.next->body.links.prev = prev;
            sf_free_list_heads[ind].body.links.next = prev;
            prev->body.links.prev = &sf_free_list_heads[ind];
        }
        void *next_head = (void *)(prev);
        sf_header *next_header = (sf_header *)(next_head + siz + siz1 + siz2);
        if (*next_header & PREV_BLOCK_ALLOCATED){
            *next_header -= PREV_BLOCK_ALLOCATED;
        }
    } else if (next_flag) {
        int wild_flag = 0;
        sf_header *next = (sf_header *)(bruh + siz);        // Grab memory address of the next block
        int siz2 = *(next);                             // Get size and parse from next header
        if (siz2 & PREV_BLOCK_ALLOCATED){               // Subtract if PREV ALLOC
            siz2 -= PREV_BLOCK_ALLOCATED;
        }
        sf_block *nextb = (sf_block *)(void *)(next);
        if (nextb == sf_free_list_heads[7].body.links.next){
            wild_flag = 1;
        }
        free_list_free(nextb);
        curr->header = siz + siz2;
        int tot = curr->header;
        curr->header = curr->header | PREV_BLOCK_ALLOCATED;
        int val = curr->header;
        sf_footer *foot = (sf_footer *)((void *)(curr) + tot - 8);
        *foot = val;
        if (wild_flag){
            sf_free_list_heads[7].body.links.next = curr;
            sf_free_list_heads[7].body.links.prev = curr;
            curr->body.links.next = &sf_free_list_heads[7];
            curr->body.links.prev = &sf_free_list_heads[7];
        } else {
            int ind = get_index(siz + siz2);
            curr->body.links.next = sf_free_list_heads[ind].body.links.next;
            sf_free_list_heads[ind].body.links.next->body.links.prev = curr;
            sf_free_list_heads[ind].body.links.next = curr;
            curr->body.links.prev = &sf_free_list_heads[ind];
        }
        void *next_head = (void *)(curr);
        sf_header *next_header = (sf_header *)(next_head + siz + siz2);
        if (*next_header & PREV_BLOCK_ALLOCATED){
            *next_header -= PREV_BLOCK_ALLOCATED;
        }
    } else if (prev_flag) {
        int wild_flag = 0;
        int prev_prev_flag = 0;
        sf_footer *sizl = (sf_footer *)((void *)(curr) - 8);  // Get size and parse from prev footer
        int siz1 = *sizl;
        if (siz1 & PREV_BLOCK_ALLOCATED){               // Subtract if PREV ALLOC
            siz1 -= PREV_BLOCK_ALLOCATED;
            prev_prev_flag = 1;
        }
        sf_block *prev = (sf_block *)((void *)(curr) - siz1);                   // Construct previous bloc
        free_list_free(prev);
        prev->header = siz + siz1;
        int tot = prev->header;
        if (prev_prev_flag){
            prev->header = prev->header | PREV_BLOCK_ALLOCATED;
        }
        sf_footer *foot = ((void *)(prev) + tot - 8);
        *foot = prev->header;
        if (((void *)(prev) + siz + siz1) == (void *)(sf_mem_end() - 8)){
            wild_flag = 1;
        }
        if (wild_flag){
            sf_free_list_heads[7].body.links.next = prev;
            sf_free_list_heads[7].body.links.prev = prev;
            prev->body.links.next = &sf_free_list_heads[7];
            prev->body.links.prev = &sf_free_list_heads[7];
        } else {
            int ind = get_index(siz + siz1);
            prev->body.links.next = sf_free_list_heads[ind].body.links.next;
            sf_free_list_heads[ind].body.links.next->body.links.prev = prev;
            sf_free_list_heads[ind].body.links.next = prev;
            prev->body.links.prev = &sf_free_list_heads[ind];
        }
        void *next_head = (void *)(prev);
        sf_header *next_header = (sf_header *)(next_head + siz + siz1);
        if (*next_header & PREV_BLOCK_ALLOCATED){
            *next_header -= PREV_BLOCK_ALLOCATED;
        }
    } else {
        curr->header = 0;
        int wild_flag = 0;
        if (((void *)(curr) + siz) == (void *)(sf_mem_end() - 8)){
            wild_flag = 1;
        }
        curr->header = siz;
        curr->header = curr->header | PREV_BLOCK_ALLOCATED;
        sf_footer *foot = (sf_footer *)((void *)(curr) + siz - 8);
        *foot = curr->header;
        if (wild_flag){
            sf_free_list_heads[7].body.links.next = curr;
            sf_free_list_heads[7].body.links.prev = curr;
            curr->body.links.next = &sf_free_list_heads[7];
            curr->body.links.prev = &sf_free_list_heads[7];
        } else {
            int ind = get_index(siz);
            curr->body.links.next = sf_free_list_heads[ind].body.links.next;
            sf_free_list_heads[ind].body.links.next->body.links.prev = curr;
            sf_free_list_heads[ind].body.links.next = curr;
            curr->body.links.prev = &sf_free_list_heads[ind];
        }
        void *next_head = (void *)(curr);
        sf_header *next_header = (sf_header *)(next_head + siz);
        if (*next_header & PREV_BLOCK_ALLOCATED){
            *next_header -= PREV_BLOCK_ALLOCATED;
        }
    }
}

void *sf_realloc(void *pp, size_t rsize) {
    pre_check(pp);
    if (rsize == 0){
        free(pp);
    }
    sf_block *curr = (sf_block *)(pp - 8);
    int size = curr->header;
    if (size & PREV_BLOCK_ALLOCATED){
        size -= PREV_BLOCK_ALLOCATED;
    }
    if (size & THIS_BLOCK_ALLOCATED){
        size -= THIS_BLOCK_ALLOCATED;
    }
    if (size == rsize){
        return pp;
    } else if (size > rsize){
        rsize = rsize + 8;                                                 // Grab malloc val
        if (rsize < 32){
            rsize = 32;
        }
        if (rsize % 16 != 0) {                                             // Round to next 32 if not a multiple already
            rsize = (rsize + (16 - rsize % 16));
        }
        if ((size - rsize) < 32){
            return pp;
        } else {
            void *block = split_block_alloc(pp, rsize);
            return block;
        }
    } else if (size < rsize){
        void *new_block = sf_malloc(rsize);
        memcpy(new_block, (void *)(curr + 8), size - 8);
        sf_free((void *)(curr) + 8);
        return new_block;
    }
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    if (align < 32 || !((align & (align - 1)) == 0)) {
        sf_errno = EINVAL;
        return NULL;
    } else if (size == 0){
        return NULL;
    }
    int alloc_size = size + align + 32;
    sf_block *our_block = sf_malloc(alloc_size);
    if ((((unsigned long)our_block + 8) & (align - 1)) == 0){
        return our_block;
    }
    return NULL;
}
