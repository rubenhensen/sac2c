/*
 *
 * $Log$
 * Revision 2.7  2000/08/04 17:19:24  dkr
 * NEWTREE removed
 *
 * Revision 2.6  2000/07/12 15:10:27  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 2.5  2000/05/30 12:35:45  dkr
 * functions for old with-loop removed
 *
 * Revision 2.4  2000/05/11 10:37:36  dkr
 * Macro SHAPE_2_ARRAY replaced by function Types2Array
 *
 * Revision 2.3  2000/02/17 16:17:53  cg
 * Function DuplicateTypes() moved to DupTree.c.
 *
 * Revision 2.2  1999/09/10 14:27:22  jhs
 * Removed those ugly MAKENODE_xxx macros.
 *
 * Revision 2.1  1999/02/23 12:40:58  sacbase
 * new release made
 *
 * Revision 1.32  1998/06/05 15:31:27  cg
 * macro MOD_NAME_CON no longer used in macro definitions.
 *
 * Revision 1.31  1998/05/12 13:19:59  cg
 * bug fixed: original vardecs are now traversed in advance and
 * module names are given to user-defined types where necessary.
 *
 * Revision 1.30  1998/02/11 16:35:08  dkr
 * typedef cmp_types moved to typecheck.h (compile.c needs to import this type)
 * function CmpTypes() declared external
 *
 *   [...]
 *
 * Revision 1.3  1994/12/06  09:58:19  hw
 * changed log-header
 *
 */

#ifndef _typecheck_h
#define _typecheck_h

/* following enum is used as return value of function CmpTypes and
 * Compatible Types
 */
#define CMP_T(n, s) n

typedef enum {
#include "cmp_type.mac"
} cmp_types;

#undef CMP_T

extern node *Typecheck (node *arg_node);
extern node *TCfundef (node *arg_node, node *arg_info);
extern node *TClet (node *arg_node, node *arg_info);
extern node *TCvardec (node *arg_node, node *arg_info);
extern node *TCreturn (node *arg_node, node *arg_info);
extern node *TCcond (node *arg_node, node *arg_info);
extern node *TCblock (node *arg_node, node *arg_info);
extern node *TCassign (node *arg_node, node *arg_info);
extern node *TCdo (node *arg_node, node *arg_info);
extern node *TCwhile (node *arg_node, node *arg_info);
extern node *TCunaryOp (node *arg_node, node *arg_info);
extern node *TCobjdef (node *arg_node, node *arg_info);
extern node *TCNcode (node *arg_node, node *arg_info);

extern types *TI_array (node *arg_node, node *arg_info);

extern node *Types2Array (types *type, types *res_type);
extern node *LookupType (char *type_name, char *mod_name, int line);
extern cmp_types CmpTypes (types *type_one, types *type_two);

/* some global variables */
extern file_type kind_of_file; /* to distinguish between compilation of a
                                * SAC-program or a SAC-module implementation
                                */

extern char *module_name; /* name of module to typecheck;
                           * is set in function Typecheck
                           */

/* and now some useful macros to get some information */

#define GET_DIM(result, type)                                                            \
    if (T_user == type->simpletype) {                                                    \
        result = LookupType (type->name, type->name_mod, 042)->info.types->dim;          \
        result += type->dim;                                                             \
    } else                                                                               \
        result = type->dim

#define GET_BASIC_SIMPLETYPE(res, type)                                                  \
    if (T_user == type->simpletype)                                                      \
        res = LookupType (type->name, type->name_mod, 042)->info.types->simpletype;      \
    else                                                                                 \
        res = type->simpletype

#define GET_BASIC_SIMPLETYPE_OF_NODE(stype, Node)                                        \
    if (N_array == Node->nodetype) {                                                     \
        DBUG_ASSERT (NULL != Node->info.types, "info.types of node N_array missing");    \
        GET_BASIC_SIMPLETYPE (stype, Node->info.types);                                  \
    } else if (N_id == Node->nodetype) {                                                 \
        DBUG_ASSERT (NULL != Node->info.ids->node, "pointer to var_dec missing");        \
        GET_BASIC_SIMPLETYPE (stype, Node->info.ids->node->info.types);                  \
    } else                                                                               \
        DBUG_ASSERT (0, "wrong nodetype != N_id,N_array")

/* a new types-stucture will be created */
#define GET_BASIC_TYPE(res_type, arg_type, line)                                         \
    {                                                                                    \
        if (T_user == arg_type->simpletype) {                                            \
            node *t_node = LookupType (arg_type->name, arg_type->name_mod, line);        \
            if (NULL == t_node)                                                          \
                ABORT (line, ("type '%s' is unknown",                                    \
                              ModName (TYPES_MOD (arg_type), TYPES_NAME (arg_type))))    \
            else {                                                                       \
                res_type = DupTypes (t_node->info.types);                                \
            }                                                                            \
        } else                                                                           \
            res_type = DupTypes (arg_type);                                              \
    }

/* number of total elements of an array */
#define GET_LENGTH(length, type)                                                         \
    {                                                                                    \
        int i;                                                                           \
        if (T_user == type->simpletype) {                                                \
            types *b_type = LookupType (type->name, type->name_mod, 042)->info.types;    \
            if (0 < b_type->dim + type->dim) {                                           \
                for (i = 0, length = 1; i < type->dim; i++)                              \
                    length *= type->shpseg->shp[i];                                      \
                for (i = 0; i < b_type->dim; i++)                                        \
                    length *= b_type->shpseg->shp[i];                                    \
            } else                                                                       \
                length = 0;                                                              \
        } else if (0 < type->dim)                                                        \
            for (i = 0, length = 1; i < type->dim; i++)                                  \
                length *= type->shpseg->shp[i];                                          \
        else                                                                             \
            length = 0;                                                                  \
    }

#define SAC_PRG F_prog
#define SAC_MOD F_modimp

#endif /* _typecheck_h */
