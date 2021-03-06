#include "frame.h"
#include "filesys/file.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "page.h"

#include "lib/typedefs.h"

//Hashing for frame table
static struct hash frame_table;
static struct lock lock_frame;

void init_frame_table(void)
{
    hash_init(&frame_table, frame_hash, frame_hash_min,NULL);    
    lock_init(&lock_frame);
}

// Computes and returns the hash value for hash element ele, given auxiliary data aux
unsigned frame_hash (const_hash_elem_ptr_t ele, void_ptr_t aux UNUSED)
{
   frame_entry_ptr_t frame = hash_entry(ele,struct frame_entry,elem);
   return hash_int((int)frame->order);   
}

//Compares the value of two hash elements and returns true if first is lesser than second, or false if first is greater than or equal to second.
bool frame_hash_min (const_hash_elem_ptr_t first, const_hash_elem_ptr_t second, void_ptr_t aux UNUSED)
{
    frame_entry_ptr_t frame_first = hash_entry(first,struct frame_entry,elem),
                      frame_second = hash_entry(second,struct frame_entry,elem);

    return frame_first->order < frame_second->order;
}

//Creates a frame and returns the frame address
frame_entry_ptr_t create_frame(page_entry_ptr_t page)
{
    frame_entry_ptr_t frame_entry = NULL;    
    void_ptr_t new_frame = NULL;
    
    lock_acquire(&lock_frame);
    
    while(new_frame == NULL)
    {
        frame_entry = (frame_entry_ptr_t)malloc(sizeof(struct frame_entry));    
        new_frame = palloc_get_page(PAL_USER);

        if(new_frame != NULL)//if fame allocated sucessufully
        {    
            frame_entry->frame_ptr = new_frame;
            frame_entry->thread_ptr = thread_current();
            frame_entry->page = page;
            frame_entry->order = hash_size(&frame_table);
            hash_insert(&frame_table,&frame_entry->elem);
        }
        else //if there is no space for frame allocation we must remove a frame
        {
        }
    }
    
    lock_release(&lock_frame);
    
    return frame_entry;
}

//Removes the frame from memory
void destroy_frame(frame_entry_ptr_t frame)
{
    hash_elem_ptr_t frame_elem = NULL;
   
    lock_acquire(&lock_frame);
    
    if(frame != NULL)
    {
        frame_elem = hash_find(&frame_table, &frame->elem);

        if(frame_elem != NULL && frame->frame_ptr != NULL)
        {
            hash_delete(&frame_table, &frame->elem);
            palloc_free_page(frame->frame_ptr);
            free(frame);        
        }
    }
    lock_release(&lock_frame);
}

//Get the frame for the given page
frame_entry_ptr_t get_frame(page_entry_ptr_t page)
{
    struct hash_iterator i;
    
    lock_acquire(&lock_frame);
    
    hash_first (&i, &frame_table);
    while (hash_next (&i))
    {
        frame_entry_ptr_t frame = hash_entry (hash_cur (&i), struct frame_entry, elem);
        if(frame->page == page)
	{
            lock_release(&lock_frame);
            return frame;
        }      
    }    
    
    lock_release(&lock_frame);
    
    return NULL;
}

