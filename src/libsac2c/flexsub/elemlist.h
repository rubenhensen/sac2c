#ifndef _ELEMLIST_H_
#define _ELEMLIST_H_

#define ELEMLIST_CURR(n) ((n)->curr)
#define ELEMLIST_PREV(n) ((n)->prev)
#define ELEMLIST_NEXT(n) ((n)->next)

void ELinit (elemlist *el);
elemlist *ELfreeNonRecursive (elemlist *el);
elemlist *ELfreeRecursive (elemlist *el);

#endif
