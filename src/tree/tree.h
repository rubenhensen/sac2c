/*
 *
 * $Log$
 * Revision 1.1  2000/01/21 15:38:43  dkr
 * Initial revision
 *
 * Revision 2.2  1999/09/10 14:20:23  jhs
 * Removed those ugly macros: MAKENODE_NUM, MAKENODE_ID, MAKENODE_BOOL,
 * MAKENODE_FLOAT, MAKENODE_DOUBLE, MAKENODE_ID_REUSE_IDS.
 *
 * Revision 2.1  1999/02/23 12:39:51  sacbase
 * new release made
 *
 * Revision 1.66  1999/01/06 13:04:14  cg
 * extern declaration of prf_name_str moved from tree.h to tree_basic.h
 *
 * Revision 1.65  1997/11/26 11:01:05  srs
 * removed use of old macros from acssass_macros.h
 *
 * Revision 1.64  1996/02/12 16:32:47  cg
 * macro MAKENODE_ID_REUSE_IDS corrected: refcount will be copied
 * from ids-structure to node-structure now
 *
 * Revision 1.63  1995/12/28  10:31:18  cg
 * Malloc is used instead of malloc in GEN_NODE
 *
 * Revision 1.62  1995/12/04  13:08:02  hw
 * changed makro MODEMODE_ID(no, str) ( str will be copied now)
 *
 * Revision 1.61  1995/11/01  16:24:16  cg
 * moved function AppendIdsChain to tree_compound.[ch]
 *
 * Revision 1.60  1995/10/06  17:10:44  cg
 * call to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.59  1995/09/27  15:16:54  cg
 * ATTENTION:
 * tree.c and tree.h are not part of the new virtual syntax tree.
 * They are kept for compatibility reasons with old code only !
 * All parts of their old versions which are to be used in the future are moved
 * to tree_basic and tree_compound.
 * DON'T use tree.c and tree.h when writing new code !!
 *
 * Revision 1.58  1995/09/07  09:49:48  sbs
 * first set of Make<N_...> functions/ access macros
 * inserted.
 *
 * Revision 1.57  1995/09/05  09:51:40  hw
 * added macro MAKENODE_DOUBLE
 *
 * Revision 1.56  1995/08/21  13:09:30  cg
 * new type charlist added.
 * new entries ST_prototype, ST_duplicate, ST_inline_import added to statustype.
 *
 * Revision 1.55  1995/08/15  16:52:11  hw
 * new status ST_duplicted_fun & ST_artificial_fun inserted (they are used to
 * tag functions while typechecking)
 *
 * Revision 1.54  1995/08/03  14:53:45  cg
 * NIF-macro adjusted to 26 parameters.
 *
 * Revision 1.53  1995/07/26  08:40:10  cg
 * new status ST_readonly_reference
 *
 * Revision 1.52  1995/07/24  09:07:41  hw
 * new typedef file_type inserted (moved from sac.y)
 *
 * Revision 1.51  1995/07/13  15:23:25  hw
 * macros MAKENODE_BOOL & MAKENODE_FLOAT added
 *
 * Revision 1.50  1995/07/10  07:31:59  asi
 * removed bblock from structure node and added def to structure ids
 *
 * Revision 1.49  1995/07/07  16:21:19  hw
 * added 'char *prf_name_str[]'( moved from typecheck.c)
 *
 * Revision 1.48  1995/07/07  14:28:53  hw
 * enlarged macro PRF_IF( there are 4 args now)
 *
 * Revision 1.47  1995/07/06  17:29:12  cg
 * statustype modified.
 *
 * Revision 1.46  1995/07/04  08:34:59  hw
 * cdbl in union node.info inserted
 *
 * Revision 1.45  1995/06/30  11:54:52  hw
 * macros for module_name-access added( they are moved from
 * typecheck.c & convert.h
 *
 * Revision 1.44  1995/06/26  14:07:56  hw
 * added new macros (moved from compile.c )
 *
 * Revision 1.43  1995/06/23  12:18:43  hw
 * enlarged macro TYP_IF
 *
 * Revision 1.42  1995/06/06  14:06:28  cg
 * statustype modified.
 *
 * Revision 1.41  1995/06/02  12:15:25  sbs
 * NIF macro prolongated
 *
 * Revision 1.40  1995/06/02  10:02:52  sbs
 * use-node in ids and info.use inserted
 *
 * Revision 1.39  1995/06/01  10:09:55  cg
 * statustype added and status in struct types inserted.
 *
 * Revision 1.38  1995/05/30  12:14:39  cg
 * number of sons in node structure set to 6 by macro MAX_SONS.
 *
 * Revision 1.37  1995/04/24  15:17:18  asi
 * MAX_MASK set to 7
 *
 * Revision 1.36  1995/04/24  15:13:46  asi
 * added AppendIdsChain
 *
 * Revision 1.35  1995/04/21  15:17:06  asi
 * added 'flag' to struct 'ids'
 *
 * Revision 1.34  1995/04/11  15:57:47  asi
 * NIF macro enlarged
 *
 * Revision 1.33  1995/04/11  11:34:45  asi
 * added 'flag' to struct 'node'
 *
 * Revision 1.32  1995/04/07  05:56:42  sbs
 * SHP_SEG_SIZE turned from 5 to 16 !
 *
 * Revision 1.31  1995/04/06  11:38:26  asi
 * MAX_MASK set to 6
 *
 * Revision 1.30  1995/03/15  18:40:35  asi
 * added refcnt to struct nchain
 *
 * Revision 1.29  1995/03/14  14:12:42  asi
 * added new entry to struct node (int bblock)
 *
 * Revision 1.28  1995/03/13  15:47:32  hw
 * MakeIds inserted
 *
 * Revision 1.27  1995/03/13  15:12:34  asi
 * added new structur 'nchain'
 * added new entry in structur 'ids' -> 'nchain'
 *
 * Revision 1.26  1995/03/08  10:28:57  hw
 * - added new entry to struct ids (int refcnt)
 * - added new entry to struct node (int refcnt)
 *
 * Revision 1.25  1995/02/28  18:25:26  asi
 * added varno in structure node
 *
 * Revision 1.24  1995/02/02  14:54:36  hw
 * bug fixed prf_dec is now a struct
 *
 * Revision 1.23  1995/01/31  14:59:33  asi
 * opt4_tab inserted and NIF macro enlarged
 *
 * Revision 1.22  1995/01/31  10:57:29  hw
 * added new entrie in union 'info' of struct 'node'
 *
 * Revision 1.21  1995/01/18  17:39:17  asi
 * MAX_MASK inserted
 *
 * Revision 1.20  1995/01/05  12:37:02  sbs
 * third component for type_info.mac inserted
 *
 * Revision 1.19  1995/01/02  11:20:44  asi
 * changed type of mask from char to long
 *
 * Revision 1.18  1995/01/02  10:50:03  asi
 * *** empty log message ***
 *
 * Revision 1.17  1994/12/30  16:57:48  sbs
 * added MakeTypes
 *
 * Revision 1.16  1994/12/30  13:49:08  hw
 * *** empty log message ***
 *
 * Revision 1.15  1994/12/30  13:22:09  hw
 * changed struct types (added id_mod & name_mod)
 * new struct fun_name
 * added fun_name to node.info
 *
 * Revision 1.14  1994/12/21  11:34:50  hw
 * changed definition of simpletype (now with macro & include)
 *
 * Revision 1.13  1994/12/20  15:56:58  sbs
 * T_hidden inserted
 *
 * Revision 1.12  1994/12/20  15:42:17  sbs
 * externals for Makenode and AppandChain added
 *
 * Revision 1.11  1994/12/20  11:23:48  sbs
 * extern decl of syntax_tree moved to scnpars.h
 *
 * Revision 1.10  1994/12/16  14:20:59  sbs
 * imp_tab inserted and NIF macro enlarged
 *
 * Revision 1.9  1994/12/15  14:19:11  asi
 * added member char *mask[2] to union node->info
 *
 * Revision 1.8  1994/12/14  16:51:24  sbs
 * type->name for T_user inserted
 *
 * Revision 1.7  1994/12/14  10:59:16  sbs
 * T_user inserted
 *
 * Revision 1.6  1994/12/14  10:48:25  asi
 * T_unknown added
 *
 * Revision 1.4  1994/12/01  17:43:43  hw
 * inserted struct NODE *node; to typedef struct IDS
 *  changed parameters of NIF
 *
 * Revision 1.3  1994/11/29  10:52:01  hw
 * added pointer to struct NODE to struct ids
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _sac_tree_h

#define _sac_tree_h

/*
 * The following included header files contain all the syntax tree
 * information. They are included here for compatability reasons only!
 * Please, include them directly when your files are converted to the
 * new virtual syntax tree.
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

/*
 *  The following macros for the generation of nodes are only supported
 *  for compatibility with old code. Please, do not use them in new code.
 *  You will find equivalent ones in tree_basic.h and tree_compound.h
 */

#define GEN_NODE(type) (type *)Malloc (sizeof (type))

extern types *MakeTypes (simpletype simple);
extern node *MakeNode (nodetype nodetype);
extern node *AppendNodeChain (int pos, node *first, node *second);

#endif /* _sac_tree_h  */
