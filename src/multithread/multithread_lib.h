/*
 *
 * $Log$
 * Revision 1.3  2000/03/09 18:34:47  jhs
 * Additional features.
 *
 * Revision 1.2  2000/03/02 13:01:02  jhs
 * Added MUTHExchangeApplication,
 * added MUTHExpandFundefName,
 * added DBUG_PRINTS and DBUG_ASSERTS.
 *
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
extern node *MUTHExchangeApplication (node *arg_node, node *new_fundef);
extern node *MUTHExpandFundefName (node *fundef, char *prefix);

#endif /* CONCURRENT_LIB_H */
