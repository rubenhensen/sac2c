#ifndef _ELEMSTACK_H_
#define _ELEMSTACK_H_

#define ELEMSTACK_CURR(n) ((n)->curr)
#define ELEMSTACK_NEXT(n) ((n)->next)

extern int isElemstackEmpty (elemstack *s);
extern void initElemstack (elemstack *s);
extern void pushElemstack (elemstack **s, elem *e);
extern elem *popElemstack (elemstack **s);

#endif
