/*
 * $Log$
 * Revision 1.5  2000/07/06 12:44:25  mab
 * added PIgetOldTypes
 *
 * Revision 1.4  2000/07/05 09:13:10  mab
 * fixed problem with global data structure pad_info
 *
 * Revision 1.3  2000/06/28 10:43:10  mab
 * made some code modifications according to code review
 *
 * Revision 1.2  2000/06/14 10:45:04  mab
 * added methods for accessing data structure
 *
 * Revision 1.1  2000/06/08 11:20:16  mab
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   pad_info.h
 *
 * prefix: PI
 *
 * description:
 *
 *   This is an abstract data structure for storing information needed by
 *   the array padding.
 *
 *
 *****************************************************************************/

#ifndef sac_pad_info_h

#define sac_pad_info_h

/* structure containing old and infered array shape */
typedef struct pad_i {
    int dim;
    simpletype type;
    shpseg *old_shape;
    shpseg *new_shape;
    node *fundef_pad;
    node *fundef_unpad;
    struct pad_i *next;
} pad_info_t;

extern void PIinit ();
extern void PIadd (types *old_type, shpseg *new_shape);
extern types *PIgetNewType (types *old_type);
extern types *PIgetOldType (types *old_type);
extern node *PIgetFundefPad (types *old_type);
extern node *PIgetFundefUnpad (types *old_type);
extern void PIfree ();

#endif /* sac_pad_info_h */
