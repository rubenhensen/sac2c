/*
 *
 * $Log$
 * Revision 1.11  1995/01/18 17:43:14  asi
 * Dead-Code-Removal now frees memory
 *
 * Revision 1.10  1995/01/17  11:05:16  asi
 * Added OPT3assign for loop invariant removal
 * OPT2while now named OPT2loop and used for do and while loops
 *
 * Revision 1.9  1995/01/12  13:15:00  asi
 * inserted OPT2fundef OPT2block OPT2assign OPT2while OPT2cond
 * for dead code removal in loops and in conditionals
 *
 * Revision 1.8  1995/01/06  14:56:09  asi
 * Dead-Vardec-Removal implemented
 *
 * Revision 1.7  1995/01/05  15:30:02  asi
 * Dead-Code-Removal implemented
 *
 * Revision 1.6  1995/01/02  16:04:46  asi
 * Renamed opt_tab in opt1_tab and all OPT.. in OPT1..
 * Added OPT1while, OPT1do, OPT1cond, OPT1cc
 * Added opt2_tab
 *
 * Revision 1.5  1995/01/02  10:50:49  asi
 * added OPTassign
 *
 * Revision 1.4  1994/12/19  15:26:07  asi
 * Added - OPTid
 *
 * Revision 1.3  1994/12/19  14:43:17  asi
 * Added - OPTfundef OPTarg OPTvardec OPTmodul OPTlet OPTblock
 * preperation for loop invariant removal
 *
 * Revision 1.2  1994/12/12  11:04:18  asi
 * added OPTdo
 *
 * Revision 1.1  1994/12/09  10:48:23  sbs
 * Initial revision
 *
 *
 */

#ifndef _optimize_h

#define _optimize_h

#define MAX_VAR 255
#define INC_VAR(mask, var) mask[var] += 1
#define DEC_VAR(mask, var) mask[var] -= 1

#define VAR_LENGTH 10

extern node *Optimize (node *arg_node);
extern char *PrintMask (long *mask);

extern node *OPT1fundef (node *arg_node, node *arg_info);
extern node *OPT1arg (node *arg_node, node *arg_info);
extern node *OPT1vardec (node *arg_node, node *arg_info);
extern node *OPTmodul (node *arg_node, node *arg_info);
extern node *OPT1let (node *arg_node, node *arg_info);
extern node *OPT1id (node *arg_node, node *arg_info);
extern node *OPT1assign (node *arg_node, node *arg_info);
extern node *OPT1pp (node *arg_node, node *arg_info);
extern node *OPT1block (node *arg_node, node *arg_info);
extern node *OPT1cond (node *arg_node, node *arg_info);
extern node *OPT1loop (node *arg_node, node *arg_info);

extern node *OPT2fundef (node *arg_node, node *arg_info);
extern node *OPT2assign (node *arg_node, node *arg_info);
extern node *OPT2block (node *arg_node, node *arg_info);
extern node *OPT2vardec (node *arg_node, node *arg_info);
extern node *OPT2cond (node *arg_node, node *arg_info);
extern node *OPT2loop (node *arg_node, node *arg_info);

extern node *OPT3assign (node *arg_node, node *arg_info);

#endif /* _optimize_h */
