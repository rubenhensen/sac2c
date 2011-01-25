#ifndef _BINHEAP_H
#define _BINHEAP_H

void PQinsertElem (elem *x, dynarray *q);
void PQinsert (int x, dynarray *q);
void PQdeleteMin (dynarray *q);
int PQgetMin (dynarray *q);
elem *PQgetMinElem (dynarray *q);
void PQprint (dynarray *q);

#endif
