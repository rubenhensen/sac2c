/*
 *
 * $Log$
 * Revision 1.1  2000/02/21 17:48:22  jhs
 * Initial revision
 *
 *
 */

#ifndef MULTITHREAD_LIB_H

#define MULTITHREAD_LIB_H

extern node *MUTHBlocksLastInstruction (node *block);
extern node *MUTHMeltBlocks (node *first_block, node *second_block);
extern node *MUTHMeltBlocksOnCopies (node *first_block, node *second_block);

#endif /* CONCURRENT_LIB_H */
