#include "page.h"
#include "threads/malloc.h"
#include "frame.h"
#include "threads/synch.h"
#include <string.h>
#include "userprog/pagedir.h"
#include "threads/vaddr.h"


void init_sup_page_t(hash_ptr_t page_table)
{
    hash_init(page_table, page_hash, page_hash_min, NULL);
}

//Destroy's the page table
void destroy_page_t(hash_ptr_t table)
{
    hash_destroy(table, delete_page_entry);
}

// Computes and returns the hash value for hash element ele, given auxiliary data aux
unsigned page_hash(const_hash_elem_ptr_t ele, void_ptr_t aux UNUSED)
{
    page_entry_ptr_t page = hash_entry(ele, struct page_entry, elem);
    return hash_int((int) page->user_page);
}

//Compares the value of two hash elements and returns true if first is lesser than second, or false if first is greater than or equal to second.
bool page_hash_min(const_hash_elem_ptr_t first, const_hash_elem_ptr_t second,void_ptr_t aux UNUSED)
{
    page_entry_ptr_t page_first = hash_entry(first, struct page_entry, elem),
            page_second = hash_entry(second, struct page_entry, elem);

    return page_first->user_page < page_second->user_page;
}

// Delete and free the page is the page entry!=NULL
void delete_page_entry(hash_elem_ptr_t hash_ele, void_ptr_t aux UNUSED)
{
    page_entry_ptr_t p_entry = NULL;
    p_entry = hash_entry(hash_ele, struct page_entry, elem);

    if (p_entry != NULL)
    {
        free(p_entry);
    }
}

// Creates an entry in supplement page table
page_entry_ptr_t create_page_entry(file_ptr_t file, off_t ofs, uint8_ptr_t upage, uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
    page_entry_ptr_t spte = (page_entry_ptr_t) malloc(sizeof (struct page_entry));
    thread_ptr_t cur = thread_current();

    if (spte != NULL) // alloc is successful
    {
        spte->file = file;
        spte->ofs = ofs;
        spte->read_bytes = read_bytes;
        spte->zero_bytes = zero_bytes;
        spte->user_page = upage;
        spte->writable = writable;

        if (hash_insert(&cur->sup_page_table, &spte->elem) == NULL) //Instert the enty to the supplement page table
        {
            return spte;
        }
    }

    free(spte);// free the resources if the page is present
    return NULL;
}

//Get the page from suplementary page table
page_entry_ptr_t get_page_entry(uint8_ptr_t upage)
{
    struct page_entry temp_page_entry;
    hash_elem_ptr_t hash_ele = NULL;
    page_entry_ptr_t page_entry = NULL;
    thread_ptr_t cur = thread_current();

    temp_page_entry.user_page = upage;
    hash_ele = hash_find(&cur->sup_page_table, &temp_page_entry.elem);

    if (hash_ele != NULL)
    {
        page_entry = hash_entry(hash_ele, struct page_entry, elem);
    }

    return page_entry;
}


bool load_page_file(page_entry_ptr_t p)
{
    frame_entry_ptr_t new_frame = NULL;
    uint32_t read_bytes = 0;

    lock_acquire(&page_lock_file);

    if (p != NULL)
    {
        new_frame = create_frame(p);

        if (new_frame != NULL)
        {
            if (p->read_bytes != 0)// If p->read_bytes is !=0 then read bytes from file
            {
                read_bytes = file_read_at(p->file, new_frame->frame_ptr, p->read_bytes, p->ofs);
                if (read_bytes != p->read_bytes)//if the bytes read is not equal frree the frame
                {
                    destroy_frame(new_frame);
                    lock_release(&page_lock_file);
                    return false;
                }
            }

            memset(new_frame->frame_ptr + p->read_bytes, 0, p->zero_bytes);

            if (!install_page(p->user_page, new_frame->frame_ptr, p->writable))
            {
                destroy_frame(new_frame);
                lock_release(&page_lock_file);
                return false;
            }

        }

    }

    lock_release(&page_lock_file);
    return true;
}

//Delete the page table entry
void destroy_page_entry(page_entry_ptr_t p)
{
    thread_ptr_t cur = thread_current();
    hash_delete(&cur->sup_page_table, &p->elem);
}

//Used to exapnd the stack
void_ptr_t build_stack(void_ptr_t uaddr)
{
    page_entry_ptr_t page = NULL;
    frame_entry_ptr_t frame = NULL;
	if(CALC_SIZE(uaddr) > MAX_STACK_SIZE) 
	//if ( (size_t) (PHYS_BASE - pg_round_down(uaddr)) > MAX_STACK_SIZE)
    	{
	     return NULL;
    	}
	page = create_page_entry(NULL, 0, uaddr, 0, 0, true);
        if (page != NULL)//check if page not equal to null
        {
            frame = create_frame(page);
            if (frame != NULL)//check if frame is not equal to nell
            {
                if (install_page(page->user_page, frame->frame_ptr, page->writable))
                {
                    return frame->frame_ptr;
                }
            }
        }
    

    return NULL;
}

