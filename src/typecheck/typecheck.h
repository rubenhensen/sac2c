/*
 * $Log$
 * Revision 1.22  1995/10/17 08:29:12  cg
 * new function declaration 'TCobjdef' added
 *
 * Revision 1.21  1995/08/09  15:55:07  cg
 * extern declaration of CmpFunParam deleted.
 *
 * Revision 1.20  1995/08/08  09:54:12  cg
 * declaration of function CmpFunParams added for external use in import.c.
 *
 * Revision 1.19  1995/07/24  09:11:48  hw
 * variable "module_name" will be exported (set while typecheckng)
 *
 * Revision 1.18  1995/07/14  16:39:26  hw
 * macro SHAPE_2_ARRAY inserted
 *
 * Revision 1.17  1995/07/14  12:03:49  hw
 * macro GET_BASIC_SIMPLETYPE_OF_NODE( stype, Node) inserted
 *
 * Revision 1.16  1995/07/13  15:43:05  hw
 * changed macro GET_LENGTH( second argument is now a 'types' struct;
 *  size of basic-type will be computed correctly now)
 *
 * Revision 1.15  1995/06/30  12:10:57  hw
 * -renamed macro GET_BASIC_TYPE to GET_BASIC_SIMPLETYPE
 * - new macro GET_BASIC_TYPE inserted
 *
 * Revision 1.14  1995/06/28  09:29:21  hw
 * moved some macros form compile.h to typecheck.h
 *
 * Revision 1.13  1995/06/23  12:38:43  hw
 * added parameter to function 'DuplicateTypes'
 *
 * Revision 1.12  1995/04/20  10:23:43  asi
 * added DuplicateTypes
 *
 * Revision 1.11  1995/03/28  12:14:22  hw
 * removed StringCopy
 *
 * Revision 1.10  1995/03/17  15:53:51  hw
 * changed function Typecheck (now it returns the syntax_tree)
 *
 * Revision 1.9  1995/03/09  16:04:47  hw
 * added extern declaration for function StringCopy
 *
 * Revision 1.8  1995/03/01  12:49:50  hw
 * added LookupType
 *
 * Revision 1.7  1994/12/30  10:16:36  hw
 * *** empty log message ***
 *
 * Revision 1.6  1994/12/20  14:20:44  hw
 * added function TCunaryOp to handle typechecking of N_post & N_pre
 *
 * Revision 1.6  1994/12/20  14:20:44  hw
 * added function TCunaryOp to handle typechecking of N_post & N_pre
 *
 * Revision 1.5  1994/12/19  13:01:42  hw
 * inserted TCdo & TCwhile
 *
 * Revision 1.4  1994/12/14  08:47:42  hw
 * added declarations of TCcond & TCassign
 *
 * Revision 1.3  1994/12/06  09:58:19  hw
 * changed log-header
 *
 *
 */

#ifndef _typecheck_h

#define _typecheck_h

extern node *Typecheck (node *arg_node);
extern node *TCfundef (node *arg_node, node *arg_info);
extern node *TClet (node *arg_node, node *arg_info);
extern node *TCreturn (node *arg_node, node *arg_info);
extern node *TCcond (node *arg_node, node *arg_info);
extern node *TCassign (node *arg_node, node *arg_info);
extern node *TCdo (node *arg_node, node *arg_info);
extern node *TCwhile (node *arg_node, node *arg_info);
extern node *TCunaryOp (node *arg_node, node *arg_info);
extern node *TCobjdef (node *arg_node, node *arg_info);

extern node *LookupType (char *type_name, char *mod_name, int line);
extern types *DuplicateTypes (types *source, int share);

/* some global variables */
extern file_type kind_of_file; /* to distinguish between compilation of a
                                * SAC-program or a SAC-module implementation
                                */
extern char *module_name;      /* name of module to typecheck;
                                * is set in function Typecheck
                                */

/* and now some useful macros to get some information */

#define GET_DIM(result, type)                                                            \
    if (T_user == type->simpletype) {                                                    \
        result = LookupType (type->name, type->name_mod, 042)->DIM;                      \
        result += type->dim;                                                             \
    } else                                                                               \
        result = type->dim

#define GET_BASIC_SIMPLETYPE(res, type)                                                  \
    if (T_user == type->simpletype)                                                      \
        res = LookupType (type->name, type->name_mod, 042)->SIMPLETYPE;                  \
    else                                                                                 \
        res = type->simpletype

#define GET_BASIC_SIMPLETYPE_OF_NODE(stype, Node)                                        \
    if (N_array == Node->nodetype) {                                                     \
        DBUG_ASSERT (NULL != Node->TYPES, "info.types of node N_array missing");         \
        GET_BASIC_SIMPLETYPE (stype, Node->TYPES);                                       \
    } else if (N_id == Node->nodetype) {                                                 \
        DBUG_ASSERT (NULL != Node->IDS_NODE, "pointer to var_dec missing");              \
        GET_BASIC_SIMPLETYPE (stype, Node->IDS_NODE->TYPES);                             \
    } else                                                                               \
        DBUG_ASSERT (0, "wrong nodetype != N_id,N_array")

/* a new types-stucture will be created */
#define GET_BASIC_TYPE(res_type, arg_type, line)                                         \
    {                                                                                    \
        if (T_user == arg_type->simpletype) {                                            \
            node *t_node = LookupType (arg_type->name, arg_type->name_mod, line);        \
            if (NULL == t_node)                                                          \
                ERROR2 (3, ("%s, %d: type '%s%s%s' is unknown", filename, line,          \
                            MOD_NAME (arg_type->name_mod), arg_type->name))              \
            else {                                                                       \
                res_type = DuplicateTypes (t_node->TYPES, 0);                            \
                if (arg_type->dim > 0) {                                                 \
                    if (res_type->dim >= 0) {                                            \
                        int dim, i;                                                      \
                        shpseg *shpseg_old;                                              \
                        int old_dim = res_type->dim;                                     \
                        dim = old_dim + arg_type->dim;                                   \
                        DBUG_ASSERT (dim <= SHP_SEG_SIZE, "shape out off range ");       \
                        shpseg_old = res_type->shpseg;                                   \
                        res_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));           \
                        res_type->shpseg->next = NULL;                                   \
                        for (i = 0; i < arg_type->dim; i++)                              \
                            res_type->shpseg->shp[i] = arg_type->shpseg->shp[i];         \
                        for (i = 0; i < old_dim; i++)                                    \
                            res_type->shpseg->shp[i + arg_type->dim]                     \
                              = shpseg_old->shp[i];                                      \
                        res_type->dim = dim;                                             \
                    }                                                                    \
                }                                                                        \
            }                                                                            \
        } else                                                                           \
            res_type = DuplicateTypes (arg_type, 0);                                     \
    }

/* number of total elements of an array */
#define GET_LENGTH(length, type)                                                         \
    {                                                                                    \
        int i;                                                                           \
        if (T_user == type->simpletype) {                                                \
            types *b_type = LookupType (type->name, type->name_mod, 042)->TYPES;         \
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
/* creates and computes the basic-shape out of a types struct as N_array
 * computed N_array-node is stored in Shape_array
 * NOTE: if the type is not the type of an array, NULL will be returned
 */
#define SHAPE_2_ARRAY(Shape_array, Type)                                                 \
    {                                                                                    \
        int i;                                                                           \
        if (T_user == Type->simpletype) {                                                \
            types *b_type = LookupType (Type->name, Type->name_mod, 042)->TYPES;         \
            if (0 < b_type->dim + Type->dim) {                                           \
                node *dummy = MakeNode (N_exprs);                                        \
                Shape_array = MakeNode (N_array);                                        \
                Shape_array->node[0] = dummy;                                            \
                Shape_array->nnode = 1;                                                  \
                for (i = 0; i < Type->dim - 1; i++) {                                    \
                    MAKENODE_NUM (dummy->node[0], Type->shpseg->shp[i]);                 \
                    dummy->node[1] = MakeNode (N_exprs);                                 \
                    dummy->nnode = 2;                                                    \
                    dummy = dummy->node[1];                                              \
                }                                                                        \
                if (0 < Type->dim) {                                                     \
                    MAKENODE_NUM (dummy->node[0], Type->shpseg->shp[i]);                 \
                    dummy->nnode = 1;                                                    \
                    if (0 < b_type->dim) {                                               \
                        dummy->node[1] = MakeNode (N_exprs);                             \
                        dummy->nnode = 2;                                                \
                        dummy = dummy->node[1];                                          \
                    }                                                                    \
                }                                                                        \
                for (i = 0; i < b_type->dim - 1; i++) {                                  \
                    MAKENODE_NUM (dummy->node[0], b_type->shpseg->shp[i]);               \
                    dummy->node[1] = MakeNode (N_exprs);                                 \
                    dummy->nnode = 2;                                                    \
                    dummy = dummy->node[1];                                              \
                }                                                                        \
                if (0 < b_type->dim) {                                                   \
                    MAKENODE_NUM (dummy->node[0], b_type->shpseg->shp[i]);               \
                    dummy->nnode = 1;                                                    \
                }                                                                        \
            }                                                                            \
        } else if (0 < Type->dim) {                                                      \
            node *dummy = MakeNode (N_exprs);                                            \
            Shape_array = MakeNode (N_array);                                            \
            Shape_array->node[0] = dummy;                                                \
            Shape_array->nnode = 1;                                                      \
            for (i = 0; i < Type->dim - 1; i++) {                                        \
                MAKENODE_NUM (dummy->node[0], Type->shpseg->shp[i]);                     \
                dummy->node[1] = MakeNode (N_exprs);                                     \
                dummy->nnode = 2;                                                        \
                dummy = dummy->node[1];                                                  \
            }                                                                            \
            MAKENODE_NUM (dummy->node[0], Type->shpseg->shp[i]);                         \
            dummy->nnode = 1;                                                            \
        } else                                                                           \
            Shape_array = NULL;                                                          \
    }

#endif /* _typecheck_h */
