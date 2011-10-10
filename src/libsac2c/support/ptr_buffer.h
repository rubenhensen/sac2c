/*
 *
 * $Id$
 *
 */

#ifndef _PTR_BUFFER_H_
#define _PTR_BUFFER_H_

typedef struct PTR_BUF ptr_buf;

extern ptr_buf *PBUFcreate (int size);
extern ptr_buf *PBUFadd (ptr_buf *s, void *ptr);
extern int PBUFsize (ptr_buf *s);
extern int PBUFpos (ptr_buf *s);
extern void *PBUFptr (ptr_buf *s, int pos);
extern void PBUFflush (ptr_buf *s);
extern void PBUFflushFrom (ptr_buf *s, int pos);
extern void *PBUFfree (ptr_buf *s);

#endif /* _PTR_BUFFER_H_ */
