/*
 *
 * $Log$
 * Revision 3.13  2002/07/15 08:53:30  dkr
 * PREC2ap added
 *
 * Revision 3.12  2002/04/16 18:31:02  dkr
 * PREC4... functions added
 *
 * Revision 3.11  2002/03/07 02:24:01  dkr
 * some functions renamed
 *
 * Revision 3.10  2002/03/01 02:32:14  dkr
 * PREC3return() removed
 *
 * Revision 3.7  2002/02/21 14:50:02  dkr
 * some functions added
 *
 * Revision 3.6  2001/03/26 15:05:25  dkr
 * header abbrevated
 *
 * Revision 3.5  2001/03/05 13:54:59  dkr
 * PREC1code added
 *
 * Revision 3.4  2001/03/02 16:10:31  dkr
 * PREC1withop added
 *
 * Revision 3.3  2001/01/19 11:55:32  dkr
 * PREC2WLseg() and PREC2WLsegVar() replaced by PREC2WLsegx()
 *
 * Revision 3.2  2000/12/04 12:31:48  dkr
 * PREC2array added
 *
 * Revision 3.1  2000/11/20 18:01:27  sacbase
 * new release made
 *
 * Revision 2.11  2000/10/16 13:43:33  dkr
 * PREC1block added
 *
 * Revision 2.10  2000/10/16 11:19:15  dkr
 * PREC1assign added
 *
 * Revision 2.9  2000/10/09 19:16:04  dkr
 * prototype for AdjustFoldFundef() removed
 *
 * Revision 2.8  2000/08/17 10:12:25  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 2.7  2000/07/14 14:46:05  nmw
 * PRECObjInitFunctionName added
 *
 * Revision 2.6  2000/07/11 09:02:39  dkr
 * minor changes done
 *
 * Revision 2.5  2000/05/29 14:30:53  dkr
 * functions PREC... renamed into PREC2...
 * PREC1let added
 *
 * Revision 2.4  2000/05/26 19:25:37  dkr
 * signature of AdjustFoldFundef() modified
 *
 * Revision 2.3  2000/05/25 23:03:40  dkr
 * prototype for AdjustFoldFundef added
 *
 * Revision 2.2  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * [...]
 *
 */

#ifndef _sac_precompile_h
#define _sac_precompile_h

extern node *Precompile (node *syntax_tree);

extern node *PREC1modul (node *arg_node, node *arg_info);
extern node *PREC1objdef (node *arg_node, node *arg_info);
extern node *PREC1fundef (node *arg_node, node *arg_info);
extern node *PREC1arg (node *arg_node, node *arg_info);
extern node *PREC1vardec (node *arg_node, node *arg_info);
extern node *PREC1assign (node *arg_node, node *arg_info);
extern node *PREC1let (node *arg_node, node *arg_info);
extern node *PREC1icm (node *arg_node, node *arg_info);
extern node *PREC1ap (node *arg_node, node *arg_info);
extern node *PREC1return (node *arg_node, node *arg_info);
extern node *PREC1id (node *arg_node, node *arg_info);

extern node *PREC2modul (node *arg_node, node *arg_info);
extern node *PREC2fundef (node *arg_node, node *arg_info);
extern node *PREC2assign (node *arg_node, node *arg_info);
extern node *PREC2let (node *arg_node, node *arg_info);
extern node *PREC2ap (node *arg_node, node *arg_info);

extern node *PREC3fundef (node *arg_node, node *arg_info);
extern node *PREC3block (node *arg_node, node *arg_info);
extern node *PREC3assign (node *arg_node, node *arg_info);
extern node *PREC3let (node *arg_node, node *arg_info);
extern node *PREC3with2 (node *arg_node, node *arg_info);
extern node *PREC3withop (node *arg_node, node *arg_info);
extern node *PREC3code (node *arg_node, node *arg_info);

extern node *PREC4modul (node *arg_node, node *arg_info);
extern node *PREC4typedef (node *arg_node, node *arg_info);
extern node *PREC4objdef (node *arg_node, node *arg_info);
extern node *PREC4fundef (node *arg_node, node *arg_info);
extern node *PREC4arg (node *arg_node, node *arg_info);
extern node *PREC4vardec (node *arg_node, node *arg_info);
extern node *PREC4let (node *arg_node, node *arg_info);
extern node *PREC4return (node *arg_node, node *arg_info);
extern node *PREC4icm (node *arg_node, node *arg_info);
extern node *PREC4array (node *arg_node, node *arg_info);
extern node *PREC4id (node *arg_node, node *arg_info);
extern node *PREC4do (node *arg_node, node *arg_info);
extern node *PREC4while (node *arg_node, node *arg_info);
extern node *PREC4cond (node *arg_node, node *arg_info);
extern node *PREC4with2 (node *arg_node, node *arg_info);
extern node *PREC4withid (node *arg_node, node *arg_info);
extern node *PREC4code (node *arg_node, node *arg_info);
extern node *PREC4WLsegx (node *arg_node, node *arg_info);

extern char *ObjInitFunctionName (bool before_rename);

extern char *RenameLocalIdentifier (char *id);

#endif /* _sac_precompile_h */
