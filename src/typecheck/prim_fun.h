/*
 *  $Log$
 *  Revision 1.1  1995/02/03 07:45:32  hw
 *  Initial revision
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
    int typed_tag; /* tag weather function is typechecked */
    int user_tag;  /* tag weather function is userdefined */
    prf new_prf;
    struct PRIM_FUN_TAB_ELEM *next;

} prim_fun_tab_elem;

#if 0

#define TT2(t, t_c, t1, t2, res) t,
typedef enum 
{
#include "prim_fun_tt.mac" 
xxxxx /* xxxxx is only a dummy argument */
}type_class_tag;
#undef TT2

#endif

extern prim_fun_tab_elem *prim_fun_tab;
extern void InitPrimFunTab ();

#endif /* _prim_fun_h */
