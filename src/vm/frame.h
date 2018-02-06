#ifndef FRAME_H
#define	FRAME_H

#include "lib/typedefs.h"
#include <hash.h>

struct frame_entry
{
    page_entry_ptr_t page;
    thread_ptr_t thread_ptr;
    void_ptr_t frame_ptr;
    uint32_t order;
    struct hash_elem elem;
};


void init_frame_table(void);

unsigned frame_hash ( const_hash_elem_ptr_t ele, void_ptr_t aux);

bool frame_hash_min (  const_hash_elem_ptr_t first, const_hash_elem_ptr_t second, void_ptr_t aux );

frame_entry_ptr_t create_frame(page_entry_ptr_t page);

void destroy_frame(frame_entry_ptr_t frame);

frame_entry_ptr_t get_frame(page_entry_ptr_t page);

void clear_frame(void);

#endif	/* FRAME_H */

