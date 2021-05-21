#include "ptr_buffer.h"

#define DBUG_PREFIX "PBUF"
#include "debug.h"

#include "memory.h"

struct PTR_BUF {
    void **buf;
    unsigned int pos;
    unsigned int size;
};

/*****************************************************************************
 *
 * @fn  ptr_buf *PBUFcreate(unsigned int size)
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
PBUFcreate (unsigned int size)
{
    ptr_buf *res;

    DBUG_ENTER ();

    res = (ptr_buf *)MEMmalloc (sizeof (ptr_buf));
    res->buf = (void **)MEMmalloc (size * sizeof (void *));
    res->pos = 0;
    res->size = size;

    DBUG_PRINT ("allocating buffer size %u : %p", size, (void *)res);

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
    unsigned int new_size;
    void **new_buf;
    unsigned int i;

    DBUG_ENTER ();

    if (s->pos == s->size) {
        new_size = 2 * s->size;
        DBUG_PRINT ("increasing buffer %p from size %u to size %u",
                    (void *)s,
                    s->size,
                    new_size);

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
    DBUG_PRINT ("%p added to buffer %p", (void *)ptr, (void *)s);
    DBUG_PRINT ("pos of buffer %p now is %u", (void *)s, s->pos);

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * @fn  unsigned int PBUFsize(ptr_buf *s)
 *
 *   @brief  retrieve size of given pointer buffer
 *
 *   @param  s
 *   @return size of the buffer
 *
 ******************************************************************************/
unsigned int
PBUFsize (ptr_buf *s)
{
    DBUG_ENTER ();
    DBUG_RETURN (s->size);
}

/******************************************************************************
 *
 * @fn  int PBUFpos(ptr_buf *s)
 *
 *   @brief  retrieve pos of given pointer buffer
 *
 *   @param  s
 *   @return pos of the buffer
 *
 ******************************************************************************/

unsigned int
PBUFpos (ptr_buf *s)
{
    DBUG_ENTER ();
    DBUG_RETURN (s->pos);
}

/*****************************************************************************
 *
 * @fn  void *PBUFptr(ptr_buf *s, unsigned int pos)
 *
 *   @brief  get pointer entry at specified position
 *
 *   @param  s
 *   @param  pos
 *   @return entry
 *
 ******************************************************************************/

void *
PBUFptr (ptr_buf *s, unsigned int pos)
{
    void *res;

    DBUG_ENTER ();
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
    DBUG_ENTER ();

    s->pos = 0;
    DBUG_PRINT ("pos of buffer %p reset to %u", (void *)s, s->pos);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn  void PBUFflushFrom(ptr_buf *s, int pos)
 *
 *   @brief  flushes the given pointer buffer (no deallocation!) starting from
 *           the position specified. If pos is bigger or equal to size nothing
 *           happens!
 *
 *   @param  s
 *
 ******************************************************************************/

void
PBUFflushFrom (ptr_buf *s, unsigned int pos)
{
    DBUG_ENTER ();

    if (pos < s->size) {
        s->pos = pos;
    }
    DBUG_PRINT ("pos of buffer %p reset to %u", (void *)s, s->pos);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    DBUG_PRINT ("freeing buffer %p", (void *)s);
    s->buf = MEMfree (s->buf);
    s = MEMfree (s);

    DBUG_RETURN (s);
}

#undef DBUG_PREFIX
