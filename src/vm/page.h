#ifndef PAGE_H
#define	PAGE_H

#include <hash.h>
#include "lib/typedefs.h"

#define CALC_SIZE(uaddr) ((uint32_t)(((uint8_ptr_t)PHYS_BASE) - (uint8_ptr_t)uaddr))

#define MAX_STACK_SIZE 1048576


//Structure for page entry
struct page_entry
{
    uint8_ptr_t user_page; // Virtual address of a physical memory
    
    file_ptr_t file; // The file which the page is read from
    off_t ofs; // offset    
    uint32_t read_bytes; // read bytes which are valid
    uint32_t zero_bytes; // bytes that are not valid and xzero
    bool writable; // to check if the page is writable or not. true - if it is writable. false - if it is not writable  
    struct hash_elem elem;
};

void init_sup_page_t(hash_ptr_t page_table);

void destroy_page_t(hash_ptr_t table);

unsigned page_hash ( const_hash_elem_ptr_t ele, void_ptr_t aux);

bool page_hash_min (  const_hash_elem_ptr_t first, const_hash_elem_ptr_t second, void_ptr_t aux );

void delete_page_entry(hash_elem_ptr_t hash_elem, void_ptr_t aux UNUSED);

void destroy_page_entry(page_entry_ptr_t page);

bool load_page_file(page_entry_ptr_t page_entry);

void_ptr_t build_stack(void_ptr_t uaddr);

page_entry_ptr_t create_page_entry(file_ptr_t file, off_t ofs, uint8_ptr_t upage, uint32_t read_bytes, uint32_t zero_bytes, bool writable);

page_entry_ptr_t get_page_entry(uint8_ptr_t upage);
#endif	/* PAGE_H */

