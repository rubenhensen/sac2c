/*
 * $Log$
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

pad_info_t *pad_info;
/* BE CARFULL! These are only references to a global structure.
               Do not link them to syntax tree! */
extern int PIgetDim (pad_info_t *pi);
extern simpletype PIgetType (pad_info_t *pi);
extern shpseg *PIgetOldShape (pad_info_t *pi);
extern shpseg *PIgetNewShape (pad_info_t *pi);
extern pad_info_t *PInewShape (types *old_type);
extern node *PIgetFundefPad (pad_info_t *pi);
extern node *PIgetFundefUnpad (pad_info_t *pi);

extern shpseg *PIcopyNewShape (pad_info_t *pi);
extern bool PIvalid (pad_info_t *pi);
extern void PIadd (types *old_type, shpseg *new_shape);
extern void PIinit ();
extern void PIfree ();

#endif /* sac_pad_info_h */
