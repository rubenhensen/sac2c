/*
 *
 * $Log$
 * Revision 1.6  1995/07/12 15:25:34  asi
 * added AnalyseLoop and InversePrf and some macros moved from .c to .h
 *
 * Revision 1.5  1995/07/07  15:02:22  asi
 * loop unrolling completed
 *
 * Revision 1.4  1995/06/26  16:24:38  asi
 * added UNRfundef
 *
 * Revision 1.3  1995/06/14  13:31:23  asi
 * added Unroll, UNRdo, UNRwhile and UNRassign
 *
 * Revision 1.2  1995/06/02  11:32:51  asi
 * Added Unroll, and UNRfundef.
 *
 * Revision 1.1  1995/05/26  14:22:26  asi
 * Initial revision
 *
 *
 */

#ifndef _Unroll_h

#define _Unroll_h

#define LEVEL arg_info->flag

/*
 * Following loop type is allowd:
 *
 * i = start;
 * LOOP(cond)
 *   ...
 *   i = i mod_prf mod_num;
 *   ...
 *   cond = i test_prf test_num;
 *   ...
 * LOOP END
 *
 */

typedef struct LINFO {
    long varno;     /* index variable	 */
    long start_num; /* initialization        */
    prf mod_prf;    /* modification function */
    long mod_num;   /* modification nummber  */
    prf test_prf;   /* test function         */
    long test_num;  /* test nummber          */
    long loop_num;  /* number of loop passes */
} linfo;

extern node *Unroll (node *arg_node, node *arg_info);

extern linfo *LoopIterations (linfo *loop_info);
extern linfo *AnalyseLoop (node *cond, int level);
extern prf InversePrf (prf fun);

extern node *UNRfundef (node *arg_node, node *arg_info);
extern node *UNRdo (node *arg_node, node *arg_info);
extern node *UNRwhile (node *arg_node, node *arg_info);
extern node *UNRwith (node *arg_node, node *arg_info);
extern node *UNRcond (node *arg_node, node *arg_info);
extern node *UNRid (node *arg_node, node *arg_info);
extern node *UNRlet (node *arg_node, node *arg_info);
extern node *UNRassign (node *arg_node, node *arg_info);

#endif /* _Unroll_h */
