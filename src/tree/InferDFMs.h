/*
 *
 * $Log$
 * Revision 1.2  2001/04/19 09:51:43  dkr
 * INFDFMSwith, INFDFMSwith2 replaced by INFDFMSwithx
 *
 * Revision 1.1  2000/12/15 18:29:36  dkr
 * Initial revision
 *
 * Revision 1.6  2000/12/15 10:43:49  dkr
 * signature of InferDFMs() modified
 *
 * Revision 1.5  2000/12/08 11:58:32  dkr
 * INFDFMSicm added
 *
 * Revision 1.4  2000/12/06 20:05:53  dkr
 * ups, syntax error eliminated
 *
 * Revision 1.3  2000/12/06 20:03:23  dkr
 * InferDFMs added
 *
 * Revision 1.1  2000/12/06 19:57:54  dkr
 * Initial revision
 *
 */

#ifndef _sac_InferDFMs_h_
#define _sac_InferDFMs_h_

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

extern node *INFDFMSfundef (node *arg_node, node *arg_info);
extern node *INFDFMSarg (node *arg_node, node *arg_info);
extern node *INFDFMSassign (node *arg_node, node *arg_info);
extern node *INFDFMSlet (node *arg_node, node *arg_info);
extern node *INFDFMSap (node *arg_node, node *arg_info);
extern node *INFDFMSid (node *arg_node, node *arg_info);
extern node *INFDFMSwithid (node *arg_node, node *arg_info);
extern node *INFDFMScode (node *arg_node, node *arg_info);
extern node *INFDFMSwithx (node *arg_node, node *arg_info);
extern node *INFDFMScond (node *arg_node, node *arg_info);
extern node *INFDFMSwhile (node *arg_node, node *arg_info);
extern node *INFDFMSdo (node *arg_node, node *arg_info);
extern node *INFDFMSicm (node *arg_node, node *arg_info);

extern node *InferDFMs (node *syntax_tree, int hide_locals);

#endif /* _sac_InferDFMs_h_ */
