/*
 *
 * $Log$
 * Revision 1.6  2004/11/24 18:37:09  sah
 * Sac DevCamp DK
 *
 * Revision 1.5  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 1.4  2004/11/17 09:01:31  ktr
 * added InferInDFMAssignChain
 *
 * Revision 1.1  2000/12/06 19:57:54  dkr
 * Initial revision
 *
 */

#ifndef _SAC_INFERDFMS_H_
#define _SAC_INFERDFMS_H_

#include "types.h"

/*
 * bit field to steer hiding of local vars
 */

/* bit definitions */
#define HIDE_LOCALS_DO 1
#define HIDE_LOCALS_WHILE 2
#define HIDE_LOCALS_COND 4
#define HIDE_LOCALS_WITH 8
#define HIDE_LOCALS_WITH2 16

/* pre-defined bit field values */
#define HIDE_LOCALS_NEVER 0
#define HIDE_LOCALS_LAC                                                                  \
    SET_BIT (SET_BIT (SET_BIT (HIDE_LOCALS_NEVER, HIDE_LOCALS_DO), HIDE_LOCALS_WHILE),   \
             HIDE_LOCALS_COND)

/* test bit */
#define TEST_HIDE_LOCALS(bf, arg_node)                                                   \
    ((NODE_TYPE (arg_node) == N_do)                                                      \
       ? TEST_BIT (bf, HIDE_LOCALS_DO)                                                   \
       : ((NODE_TYPE (arg_node) == N_while)                                              \
            ? TEST_BIT (bf, HIDE_LOCALS_WHILE)                                           \
            : ((NODE_TYPE (arg_node) == N_cond)                                          \
                 ? TEST_BIT (bf, HIDE_LOCALS_COND)                                       \
                 : ((NODE_TYPE (arg_node) == N_Nwith)                                    \
                      ? TEST_BIT (bf, HIDE_LOCALS_WITH)                                  \
                      : ((NODE_TYPE (arg_node) == N_Nwith2)                              \
                           ? TEST_BIT (bf, HIDE_LOCALS_WITH2)                            \
                           : FALSE)))))

extern node *INFDFMSfundef (node *arg_node, info *arg_info);
extern node *INFDFMSarg (node *arg_node, info *arg_info);
extern node *INFDFMSassign (node *arg_node, info *arg_info);
extern node *INFDFMSlet (node *arg_node, info *arg_info);
extern node *INFDFMSap (node *arg_node, info *arg_info);
extern node *INFDFMSid (node *arg_node, info *arg_info);
extern node *INFDFMSwithid (node *arg_node, info *arg_info);
extern node *INFDFMScode (node *arg_node, info *arg_info);
extern node *INFDFMSwithx (node *arg_node, info *arg_info);
extern node *INFDFMScond (node *arg_node, info *arg_info);
extern node *INFDFMSwhile (node *arg_node, info *arg_info);
extern node *INFDFMSdo (node *arg_node, info *arg_info);
extern node *INFDFMSicm (node *arg_node, info *arg_info);

extern node *INFDFMSdoInferDfms (node *syntax_tree, int hide_locals);
extern dfmask_t *INFDFMSdoInferInDfmAssignChain (node *assign, node *fundef);

#endif /* _SAC_INFERDFMS_H_ */
