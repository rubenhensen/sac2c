/*
 *  $Log$
 *  Revision 2.1  1999/02/23 12:40:50  sacbase
 *  new release made
 *
 *  Revision 1.8  1996/01/25 16:19:07  hw
 *  added typechecking of primitive functions in modules
 *  (in modules the resultung type can be also one with known dimension only
 *   (without known shape). This feature can be added by compiling this file
 *   with #define KNOWN_DIM.
 *   KNOWN_DIM isn't set in this version, but can be added later)
 *
 * Revision 1.7  1995/12/06  17:03:37  hw
 * added typecheck of primitive function 'genarray'
 *
 * Revision 1.6  1995/08/11  17:27:49  hw
 * function Modarray inserted
 *
 * Revision 1.5  1995/07/04  09:06:30  hw
 * - Axs_F, F2I & I2F removed
 * - ConvertType inserted
 *
 * Revision 1.4  1995/06/30  11:35:12  hw
 * - #if 0 .. deleted
 * - functions F2I & I2F inserted
 *
 * Revision 1.3  1995/02/09  11:07:52  hw
 *  - changed function AxA
 *  - renamed function Ixf_F to Axs_f
 *  - added new functions: Shp, Reshp, TakeV, DropV, Psi, TakeDropS, Rot, Cat
 *
 * Revision 1.2  1995/02/03  16:04:18  hw
 * added new functions AxA & Ixf_F
 *
 * Revision 1.1  1995/02/03  07:45:32  hw
 * Initial revision
 *,$
 *
 */

#ifndef _prim_fun_h

#define _prim_fun_h

typedef struct PRIM_FUN_TAB_ELEM {
    prf prf;       /* kind of primitive function */
    id *id_mod;    /* name of module where userdefined
                    *  primitive function is defined
                    */
    node *node;    /* pointer to declaration (function header) */
    int typed_tag; /* tag whether function is typechecked */
    int user_tag;  /* tag whether function is userdefined */
    prf new_prf;
    struct PRIM_FUN_TAB_ELEM *next;

} prim_fun_tab_elem;

extern prim_fun_tab_elem *prim_fun_tab;
extern void InitPrimFunTab ();
extern types *AxA (types *array1, types *array2, simpletype s_type);
extern types *Reshp (node *vec, types *array, types *shp_vec);
extern types *Shp (types *array);
extern types *TakeV (node *vec, types *vec_type, types *array);
extern types *DropV (node *vec, types *vec_type, types *array);
extern types *Psi (types *vec, types *array);
extern types *TakeDropS (node *vec, types *array, int tag);
extern types *Cat (node *s_node, types *array1, types *array2);
extern types *Rot (node *s_node, types *array);
extern types *ConvertType (types *array1, simpletype s_type);
extern types *Modarray (types *array, types *vec, types *value, int line);
extern types *Genarray_S (node *v_node, types *vec, types *array);
extern types *Genarray_A (node *v_node, types *vec, types *array);

#if 0 
extern types *Psi(types *array1, types *array2);
extern types *Axs_F(types *array1);
#endif

#endif /* _prim_fun_h */
