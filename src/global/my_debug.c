/*
 *
 * $Log$
 * Revision 3.4  2002/06/07 13:47:36  dkr
 * 'mdb_argtag' renamed into 'ATG_string' and move to argtag.[ch]
 *
 * Revision 3.3  2002/03/01 02:36:21  dkr
 * mdb_argtag added
 *
 * Revision 3.1  2000/11/20 17:59:34  sacbase
 * new release made
 *
 * Revision 2.4  2000/03/02 14:08:39  jhs
 * Added statustype_info.mac for statustype and mdb_statustype.
 *
 * Revision 2.3  1999/10/19 12:56:48  sacbase
 * inclusion of type_info.mac adjusted to new .mac mechanism
 *
 * Revision 2.2  1999/07/23 17:17:46  jhs
 * Restructured node_info.mac and simplified it's usage.
 *
 * Revision 2.1  1999/02/23 12:39:30  sacbase
 * new release made
 *
 * Revision 1.25  1999/01/15 15:17:30  cg
 * added new entries to NIF macro.
 *
 * Revision 1.24  1998/06/18 13:40:07  cg
 * macro NIF used in node_info.mac enlarged,
 * new traversal function tables added.
 *
 * Revision 1.23  1998/04/29 17:11:50  dkr
 * changed usage of NIF
 *
 * Revision 1.22  1998/04/23 18:57:34  dkr
 * changed usage of NIF
 *
 * Revision 1.21  1998/04/02 16:06:40  dkr
 * changed signature of NIF
 *
 * Revision 1.20  1997/11/05 09:33:26  dkr
 * usage of NIF-macro has changed
 *
 * Revision 1.19  1996/01/16 16:45:45  cg
 * extended macro TYP_IF to 5 positions
 *
 * [...]
 *
 */

/*
 *  global array used for DBUG purposes only
 */

char *mdb_nodetype[] = {
#define NIFmdb_nodetype(mdb_nodetype) mdb_nodetype
#include "node_info.mac"
};

char *mdb_prf[] = {
#define PRF_IF(n, s, x, y) s
#include "prf_node_info.mac"
#undef PRF_IF
};

char *mdb_type[] = {
#define TYP_IFdb_str(str) str
#include "type_info.mac"
};

char *mdb_statustype[] = {
#define SELECTtext(it_text) it_text
#include "status_info.mac"
};
