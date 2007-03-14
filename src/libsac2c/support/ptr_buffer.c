/*
 *
 * $Id$
 *
 */

#include "ptr_buffer.h"

#include "dbug.h"
#include "memory.h"

struct PTR_BUF {
    void **buf;
    int pos;
    int size;
};

/*****************************************************************************
 *
 * @fn  ptr_buf *PBUFcreate(int size)
 *
 *   @brief  creates an (unbound) pointer buffer
 *
 *           Similar mechanism as used for StrBuf's.
 *           Here, instead of characters, void pointers are stored.
 *   @param  size
 *   @return the pointer to the freshly allocated buffer.
 *
 ******************************************************************************/

ptr_buf *
PBUFcreate (int size)
{
    ptr_buf *res;

    DBUG_ENTER ("PBUFcreate");

    res = (ptr_buf *)MEMmalloc (sizeof (ptr_buf));
    res->buf = (void **)MEMmalloc (size * sizeof (void *));
    res->pos = 0;
    res->size = size;

    DBUG_PRINT ("PBUF", ("allocating buffer size %d : %p", size, res));

    DBUG_RETURN (res);
}

/*****************************************************************************
 *
 * @fn  ptr_buf *PBUFadd(ptr_buf *s, void *ptr)
 *
 *   @brief  adds ptr to buffer s (new last element)
 *
 *   @param  s
 *   @param  ptr
 *   @return the modified buffer
 *
 ******************************************************************************/

ptr_buf *
PBUFadd (ptr_buf *s, void *ptr)
{
    int new_size;
    void **new_buf;
    int i;

    DBUG_ENTER ("PBUFadd");

    if (s->pos == s->size) {
        new_size = 2 * s->size;
        DBUG_PRINT ("PBUF", ("increasing buffer %p from size %d to size %d", s, s->size,
                             new_size));

        new_buf = (void **)MEMmalloc (new_size * sizeof (void *));
        for (i = 0; i < s->pos; i++) {
            new_buf[i] = s->buf[i];
        }
        s->buf = MEMfree (s->buf);
        s->buf = new_buf;
        s->size = new_size;
    }
    s->buf[s->pos] = ptr;
    s->pos++;
    DBUG_PRINT ("PBUF", ("%p added to buffer %p", ptr, s));
    DBUG_PRINT ("PBUF", ("pos of buffer %p now is %d", s, s->pos));

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * @fn  int PBUFsize(ptr_buf *s)
 *
 *   @brief  retrieve size of given pointer buffer
 *
 *   @param  s
 *   @return size of the buffer
 *
 ******************************************************************************/

int
PBUFsize (ptr_buf *s)
{
    DBUG_ENTER ("PBUFsize");
    DBUG_RETURN (s->size);
}

/*****************************************************************************
 *
 * @fn  void *PBUFptr(ptr_buf *s, int pos)
 *
 *   @brief  get pointer entry at specified position
 *
 *   @param  s
 *   @param  pos
 *   @return entry
 *
 ******************************************************************************/

void *
PBUFptr (ptr_buf *s, int pos)
{
    void *res;

    DBUG_ENTER ("PBUFptr");
    if (pos < s->pos) {
        res = s->buf[pos];
    } else {
        res = NULL;
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn  void PBUFflush(ptr_buf *s)
 *
 *   @brief  flushes the given pointer buffer (no deallocation!)
 *
 *   @param  s
 *
 ******************************************************************************/

void
PBUFflush (ptr_buf *s)
{
    DBUG_ENTER ("PBUFflush");

    s->pos = 0;
    DBUG_PRINT ("PBUF", ("pos of buffer %p reset to %d", s, s->pos));

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * @fn  void *PBUFfree(  ptr_buf *s)
 *
 *   @brief  deallocates the given pointer buffer!
 *
 *   @param  s
 *
 ******************************************************************************/

void *
PBUFfree (ptr_buf *s)
{
    DBUG_ENTER ("PBUFfree");

    DBUG_PRINT ("PBUF", ("freeing buffer %p", s));
    s->buf = MEMfree (s->buf);
    s = MEMfree (s);

    DBUG_RETURN (s);
}