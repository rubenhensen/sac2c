/*
 * $Log$
 * Revision 1.1  2000/06/08 11:20:16  mab
 * Initial revision
 *
 */

#ifndef sac_pad_info_h

#define sac_pad__info_h

/* structure containing old and infered array shape */
typedef struct pad_i {
    int dim;
    simpletype type;
    nums *old_shape;
    nums *new_shape;
    struct pad_i *next;
} pad_info_t;

pad_info_t *pad_info;

extern int PIgetDim (pad_info_t *pi);
extern simpletype PIgetType (pad_info_t *pi);
extern nums *PIgetOldShape (pad_info_t *pi);
extern nums *PIgetNewShape (pad_info_t *pi);
extern bool PIvalid (pad_info_t *pi);
extern pad_info_t *PInewShape (int dim, simpletype type, nums *old_shape);
extern void PIadd (simpletype type, int dim, nums *old_shape, nums *new_shape);

#endif /* sac_pad_info_h */
