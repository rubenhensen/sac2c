/*
 *
 * $Log$
 * Revision 1.166  1998/05/14 12:50:12  sbs
 * N_info  accesses added for flatten:  INFO_FLTN_xxx
 *
 * Revision 1.165  1998/05/13 14:39:48  srs
 * added NPART_COPY
 *
 * Revision 1.164  1998/05/13 13:45:56  srs
 * added comment to N_id
 *
 * Revision 1.163  1998/05/12 22:45:28  dkr
 * added NWITH2_DIM, NWITH2_IDX_MIN, NWITH2_IDX_MAX
 * removed SYNC_DEC_RC_IDS, SYNC_INOUT_IDS
 *
 * Revision 1.162  1998/05/12 15:50:22  dkr
 * removed ???_VARINFO macros
 *
 * Revision 1.161  1998/05/12 13:56:29  dkr
 * renamed ???_RC_IDS into ???_DEC_RC_IDS
 *
 * Revision 1.160  1998/05/12 13:46:18  srs
 * added INFO_AE_TYPES and ID_IDS
 *
 * Revision 1.159  1998/05/12 13:17:37  dkr
 * added SYNC_RC_IDS, SPMD_LIFTED_FROM
 *
 * Revision 1.158  1998/05/08 00:47:44  dkr
 * added some attributes to N_Nwith/N_Nwith2
 *
 * Revision 1.157  1998/05/07 22:49:31  dkr
 * added NWITH_RC_IDS, NWITH2_RC_IDS
 *
 * Revision 1.156  1998/05/06 22:20:48  dkr
 * added support for DFMasks
 *
 * Revision 1.155  1998/05/06 15:12:16  srs
 * changed INFO_DUP_CONT macro
 * (node[1] instead of node[0] because of conflicts with INFO_INL_ macros).
 *
 * Revision 1.154  1998/05/06 14:27:41  dkr
 * added macros for DataFlowMasks
 *
 * Revision 1.153  1998/05/04 18:04:09  dkr
 * removed VARDEC in comment for N_let
 *
 * Revision 1.152  1998/05/04 17:56:41  sbs
 * LET_VARDEC(n) moved from tree_basic.h to tree_compound.h!!!!
 *
 * Revision 1.151  1998/05/02 17:44:34  dkr
 * added macros for N_with2, N_sync, N_spmd
 *
 * Revision 1.150  1998/04/30 12:23:16  srs
 * changed comment
 *
 * Revision 1.149  1998/04/29 20:15:50  dkr
 * added macro INFO_SPMD_FIRST
 *
 * Revision 1.148  1998/04/29 19:52:51  dkr
 * changed macros for N_spmd, N_sync
 *
 * Revision 1.147  1998/04/26 21:51:53  dkr
 * MakeSPMD renamed to MakeSpmd
 * added macro SYNC_OUT
 *
 * Revision 1.146  1998/04/26 12:32:06  dkr
 * changed a comment for N_fundef
 *
 * Revision 1.145  1998/04/25 12:35:26  dkr
 * changed a comment for NCODE_COPY
 *
 * Revision 1.144  1998/04/24 17:19:27  dkr
 * changed attributes/macros for N_spmd, N_sync
 *
 * Revision 1.143  1998/04/24 01:14:53  dkr
 * added N_sync
 *
 * Revision 1.142  1998/04/22 18:52:06  dkr
 * removed INFO_COMP_WITHID, INFO_COMP_ENDASSIGN
 *
 * Revision 1.141  1998/04/22 17:02:11  dkr
 * added INFO_COMP_WITHID
 *
 * Revision 1.140  1998/04/22 13:04:12  dkr
 * added INFO_COMP_ENDASSIGN
 *
 * Revision 1.139  1998/04/21 15:40:46  srs
 * added INFO_WLI_LET and INFO_WLI_REPLACE
 *
 * Revision 1.138  1998/04/21 13:30:27  dkr
 * NWITH2_SEG renamed to NWITH2_SEGS
 *
 * Revision 1.137  1998/04/20 14:10:58  sbs
 * N_icm remarks for NEXT inserted
 *
 * Revision 1.136  1998/04/20 12:28:54  srs
 * added comment to N_id
 *
 * Revision 1.135  1998/04/20 00:05:14  dkr
 * changed INFO_PREC_LETIDS
 *
 * Revision 1.134  1998/04/19 23:24:04  dkr
 * INFO_PREC_LETVARS renamed to INFO_PREC_LETIDS
 *
 * Revision 1.133  1998/04/19 23:17:35  dkr
 * added macro INFO_PREC_LETVARS
 *
 * Revision 1.132  1998/04/19 18:27:32  dkr
 * changed INFO_COMP_... macros
 *
 * Revision 1.131  1998/04/17 18:55:34  dkr
 * added INFO_PREC_FUNDEF
 *
 * Revision 1.130  1998/04/17 17:26:25  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.129  1998/04/17 11:39:54  srs
 * added NCODE_COPY
 *
 * Revision 1.128  1998/04/16 19:07:42  dkr
 * changed a comment for N_ap
 *
 * Revision 1.127  1998/04/16 16:00:04  srs
 * added INFO_INL* macros
 *
 * Revision 1.126  1998/04/16 15:42:21  dkr
 * new macro INFO_COMP_CONCFUNS
 *
 * Revision 1.125  1998/04/16 11:46:29  dkr
 * changed macro INFO_DUPCONT
 *
 * Revision 1.124  1998/04/16 11:44:06  srs
 * new INFO_WLI* macros,
 * new section of INFO_PRINT* macros
 *
 * Revision 1.123  1998/04/14 22:46:55  dkr
 * new macros:
 *   INFO_COMP_...
 *   INFO_DUPCONT
 *
 * Revision 1.122  1998/04/14 15:03:50  srs
 * added INFO_* macros
 *
 * Revision 1.121  1998/04/11 15:09:45  srs
 * addec comments
 *
 * Revision 1.120  1998/04/10 02:25:01  dkr
 * changed macros for N_WLseg
 * changed macros for N_conc
 *
 * Revision 1.119  1998/04/09 13:58:43  dkr
 * new INFO_CONC_..., CONC_... macros added
 *
 * Revision 1.118  1998/04/07 16:50:56  srs
 * new macro INFO_WLI_FLAG
 *
 * Revision 1.117  1998/04/07 14:42:49  srs
 * added comment
 *
 * Revision 1.116  1998/04/07 12:11:17  srs
 * changed comment
 *
 * Revision 1.115  1998/04/04 18:44:55  dkr
 * renamed CONC_AP to CONC_AP_LET
 *
 * Revision 1.114  1998/04/03 21:06:08  dkr
 * added N_info access macros for precompile
 * added attributes for N_conc: CONC_AP, CONC_MASK
 *
 * Revision 1.113  1998/04/02 17:39:35  dkr
 * removed NWITH2_FUNAP
 * new node N_conc added
 *
 * Revision 1.112  1998/04/02 13:13:30  dkr
 * added NWITH2_FUNAP
 *
 * Revision 1.111  1998/04/02 13:00:49  srs
 * added ID_WL()
 *
 * Revision 1.110  1998/04/01 23:57:56  dkr
 * added N_WLstriVar, N_WLgridVar
 *
 * Revision 1.109  1998/04/01 17:22:56  srs
 * added macro ASSIGN_LEVEL
 *
 * Revision 1.108  1998/04/01 12:55:06  dkr
 * added some comments for N_WL... nodes
 *
 * Revision 1.107  1998/04/01 07:39:25  srs
 * renames INFO_* macros for CF
 *
 * Revision 1.106  1998/03/30 23:42:32  dkr
 * added attribute LEVEL for N_WLgrid
 *
 * Revision 1.105  1998/03/29 23:27:45  dkr
 * added temp. attribute WLGRID_MODIFIED
 *
 * Revision 1.104  1998/03/27 18:37:16  dkr
 * WLproj renamed in WLstride
 * WLPROJ... renamed in WLSTRIDE
 *
 * Revision 1.103  1998/03/26 18:05:43  srs
 * added NWITH_NO_CHANCE
 *
 * Revision 1.102  1998/03/26 14:00:25  dkr
 * some changes in N_WLgrid:
 *   changed usage of MakeWLgrid
 *   CODE is now a attribute, not a son!
 * fixed a bug with WLNODE_NEXT
 *
 * Revision 1.101  1998/03/26 11:03:22  dkr
 * changed WLNODE-macros
 *
 * Revision 1.100  1998/03/25 18:11:52  srs
 * added INFO_WLI_FUNDEF
 *
 * Revision 1.99  1998/03/25 13:04:02  srs
 * corrected comment of N_modarray
 *
 * Revision 1.98  1998/03/24 21:08:25  dkr
 * added temp. attr. WLPROJ_PART
 *
 * Revision 1.97  1998/03/24 10:56:23  srs
 * added comment to N_Ncode
 *
 * Revision 1.96  1998/03/22 23:45:19  dkr
 * fixed a bug with WLNODE_BOUND1
 *
 * Revision 1.95  1998/03/22 15:31:54  dkr
 * N_WLproj: OFFSET, WIDTH -> BOUND1, BOUND2
 *
 * Revision 1.94  1998/03/22 14:21:26  dkr
 * WLBLOCK_BLOCKING -> WLBLOCK_STEP
 *
 * Revision 1.93  1998/03/21 21:58:54  dkr
 * added macros WLNODE_???
 *
 * Revision 1.92  1998/03/21 14:06:00  dkr
 * changed MakeWLublock
 *
 * Revision 1.91  1998/03/21 12:35:30  dkr
 * added comments for WLseg
 *
 * Revision 1.90  1998/03/20 20:51:33  dkr
 * added attributes to N_WLseg for blocking
 * changed usage of MakeWLseg
 *
 * Revision 1.89  1998/03/20 17:24:54  dkr
 * in N_WL... nodes: INNER is now called CONTENTS
 *
 * Revision 1.88  1998/03/18 11:04:12  dkr
 * added WLPROJ_MODIFIED
 *
 * Revision 1.86  1998/03/17 10:36:25  dkr
 * added WLSEG_DIM, WLSEG_BV, WLSEG_UBV
 *
 * Revision 1.85  1998/03/17 09:57:50  cg
 * added permanent optional attribute ARRAY_STRING
 *
 * Revision 1.84  1998/03/15 13:10:55  srs
 * added node decription
 *
 * Revision 1.83  1998/03/13 16:21:42  dkr
 * new nodes added:
 *   N_WLblock, N_WLublock
 *
 * Revision 1.82  1998/03/06 13:23:58  srs
 * new ASSIGN, INFO, NCODE and NWITH access macros
 *
 * Revision 1.81  1998/03/03 23:00:59  dkr
 * added CODE_NO
 *
 * Revision 1.80  1998/03/03 19:39:27  dkr
 * renamed N_WLindex to N_WLproj
 *
 * Revision 1.79  1998/03/03 19:16:05  dkr
 * changed something in N_WLgrid
 *
 * Revision 1.78  1998/03/03 17:40:49  dkr
 * changed MakeWLgrid(), MakeWLindex()
 *
 * Revision 1.77  1998/03/03 16:13:26  dkr
 * changed something in WLindex, WLgrid
 *
 * Revision 1.76  1998/03/02 22:28:29  dkr
 * added nodes for precompilation of new with-loop
 *
 * Revision 1.75  1998/02/23 13:12:08  srs
 * added comments and access macros for masks
 *
 * Revision 1.74  1998/02/17 14:08:53  srs
 * changed comments for N_info
 *
 * Revision 1.72  1998/02/16 16:33:55  srs
 * Changed MakeNwith
 *
 * Revision 1.71  1998/02/12 11:03:07  srs
 * added INFO_.. macros for ConstantColding
 *
 * Revision 1.70  1998/02/11 17:21:36  srs
 * changed NPART_IDX to NPART_WITHID
 *
 * Revision 1.69  1998/02/10 14:57:05  dkr
 * added macro WITH_USEDVARS
 *
 * Revision 1.68  1998/02/10 12:28:54  srs
 * added macro for N_block
 *
 * Revision 1.67  1998/02/09 16:07:51  srs
 * changed N_Nwithid and introduced new
 * macros for N_info
 *
 * Revision 1.66  1998/02/05 16:25:16  srs
 * added REFCNT to N_pre and N_post.
 * added FUNDEF to  N_Nwithop.
 *
 * Revision 1.65  1998/01/30 17:54:15  srs
 * changed node structure of N_Nwithid
 *
 * Revision 1.64  1997/12/20 17:33:24  srs
 * changed comment for N_vardec
 *
 * Revision 1.62  1997/12/02 18:47:10  srs
 * changed comments
 *
 * Revision 1.61  1997/11/24 17:05:06  sbs
 * Nwithop modified :
 * FUN points to funname.id
 * and MOD points to funname.modid
 *
 * Revision 1.60  1997/11/22 23:32:14  dkr
 * uups! it is not recommended to use c-comments in this message ... :(
 *
 * Revision 1.57  1997/11/18 18:05:43  srs
 * changed new WL-macros
 *
 * Revision 1.56  1997/11/18 17:33:31  dkr
 * modified MakeNWith()
 *
 * Revision 1.55  1997/11/13 16:16:00  srs
 * created access macros for new WL-syntaxtree
 *
 * Revision 1.54  1997/05/28 12:36:51  sbs
 * Profiling integrated
 *
 * Revision 1.53  1997/05/16  09:54:01  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.52  1997/05/14  08:16:43  sbs
 * N_annotate added
 *
 * Revision 1.51  1997/05/02  13:52:39  sbs
 * SHAPES_SHPSEG inserted
 *
 * Revision 1.50  1997/04/30  11:49:29  cg
 * new macro VARDEC_OBJDEF
 *
 * Revision 1.49  1997/03/19  13:38:16  cg
 * Added new data type 'deps' with respective access macros and
 * creation function
 *
 * Revision 1.48  1996/02/11  20:19:01  sbs
 * some minor corrections on stuff concerning N_vinfo,
 * VARDEC_ACTCHN, VARDEC_COLCHN, ARG_ACTCHN, and ARG_COLCHN added.
 *
 * Revision 1.47  1996/01/26  15:29:02  cg
 * added macro ID_MAKEUNIQUE(n)
 *
 * Revision 1.46  1996/01/25  18:39:05  cg
 * added macro MODUL_CLASSTYPE(n)
 *
 * Revision 1.45  1996/01/22  17:24:40  cg
 * added macro OBJDEC_DEF(n)
 *
 * Revision 1.44  1996/01/22  14:44:44  asi
 * added ASSIGN_CSE
 *
 * Revision 1.43  1996/01/22  14:37:45  cg
 * modified some macros for N_objdef nodes
 *
 * Revision 1.42  1996/01/22  14:04:50  asi
 * added MakeId2
 *
 * Revision 1.41  1996/01/22  09:03:48  cg
 * added some new macros for N_objdef node
 *
 * Revision 1.40  1996/01/16  16:55:15  cg
 * added some macros for N_info nodes
 *
 * Revision 1.39  1996/01/15  11:06:07  asi
 * added ID_DEF
 *
 * Revision 1.38  1996/01/12  15:53:00  asi
 * added LET_VARDEC
 *
 * Revision 1.37  1996/01/12  14:56:36  cg
 * added some new macros for N_info node, corrected ID_REFCNT.
 *
 * Revision 1.36  1996/01/09  12:04:59  cg
 * added macro ARG_REFCNT
 *
 * Revision 1.35  1996/01/07  16:54:49  cg
 * modified some pragma-related macros
 *
 * Revision 1.34  1996/01/05  14:33:22  cg
 * removed macros MODARRAY_RETURN, GENARRAY_RETURN,
 * FOLDFUN_RETURN, FOLDPRF_RETURN
 *
 * Revision 1.33  1996/01/05  14:14:10  asi
 * added While2Do: This function converts a while-loop into a do-loop
 *
 * Revision 1.32  1996/01/02  14:44:35  cg
 * macro MODUL_CCCALL removed again
 *
 * Revision 1.31  1995/12/29  14:22:41  cg
 * added macro MODUL_CCCALL
 *
 * Revision 1.30  1995/12/29  10:31:18  cg
 * new macros for N_sib and N_info nodes, added new macros to restore
 * the new with-loop syntax from the internal representation.
 * added macros LINKMOD for fundef and objdef nodes
 *
 * Revision 1.29  1995/12/21  13:24:16  asi
 * added OPERATOR_MASK for N_genarray, N_modarray, N_foldprf and N_foldfun - nodes
 *
 * Revision 1.28  1995/12/21  10:07:33  cg
 * now MakePragma has no argument at all, new macros PRAGMA_LINKSIGNNUMS etc.
 * for temporary storage of pragma as list of nums instead of array.
 *
 * Revision 1.27  1995/12/20  08:15:10  cg
 * renamed macro WITH_BODY to WITH_OPERTATOR
 * new macros for new N_char node
 * changed macros for N_pragma node with respect to using arrays for pragmas linksign,
 * refcounting, and readonly.
 *
 * Revision 1.26  1995/12/19  13:38:46  cg
 * renamed macro WITH_BODY to WITH_OPERATOR
 * added macros for new N_char node
 *
 * Revision 1.25  1995/12/18  18:27:11  cg
 * added macro OBJDEF_ICM
 *
 * Revision 1.24  1995/12/13  13:32:46  asi
 * added ASSIGN_STATUS, modified ICM_NAME
 *
 * Revision 1.23  1995/12/13  09:38:26  cg
 * modified macro RETURN_REFERENCE(n)
 *
 * Revision 1.22  1995/12/12  18:26:02  asi
 * added VARDEC_FLAG
 *
 * Revision 1.21  1995/12/07  16:23:29  asi
 * comment added for INLREC
 *
 * Revision 1.20  1995/12/04  15:03:58  asi
 * added temporary attribute FUNDEF_INLREC(n)
 *
 * Revision 1.19  1995/12/01  20:26:14  cg
 * removed macro OBJDEF_INITFUN
 *
 * Revision 1.18  1995/12/01  17:09:20  cg
 * added new node type N_pragma
 * removed macro FUNDEF_ALIAS
 *
 * Revision 1.17  1995/11/16  19:40:45  cg
 * Macros for accessing Masks modified,
 * old style macros for masks moved to tree_compound.h
 *
 * Revision 1.16  1995/11/06  09:22:21  cg
 * added macro VARDEC_ATTRIB
 *
 * Revision 1.15  1995/11/02  13:13:31  cg
 * added new macros OBJDEF_ARG(n) and renamed IDS_DECL(i)
 * to IDS_VARDEC(i).
 *
 * Revision 1.14  1995/11/01  16:27:04  cg
 * added some comments
 *
 * Revision 1.13  1995/11/01  07:10:12  sbs
 * neutral addded to N_foldprf;
 *
 * Revision 1.12  1995/10/30  09:53:35  cg
 * added temporary attribute FUNDEF_INLINE(n)
 *
 * Revision 1.11  1995/10/26  15:58:58  cg
 * new macro MODUL_STORE_IMPORTS(n)
 *
 * Revision 1.10  1995/10/22  17:33:12  cg
 * new macros FUNDEC_DEF and TYPEDEC_DEF
 * new slot DECL of N_modul node to store declaration when compiling
 * a module/class implementation
 *
 * Revision 1.8  1995/10/19  11:16:00  cg
 * bug in macro AP_FUNDEF fixed.
 *
 * Revision 1.7  1995/10/19  10:06:42  cg
 * new slots for VARDEC nodes: VARDEC_STATUS and VARDEC_TYPEDEF
 *
 * Revision 1.6  1995/10/16  13:00:38  cg
 * new macro OBJDEF_VARNAME added.
 *
 * Revision 1.5  1995/10/12  13:45:15  cg
 * new macros TYPEDEF_STATUS, FUNDEF_STATUS and OBJDEF_STATUS to mark
 * imported items
 *
 * Revision 1.4  1995/10/12  12:25:19  cg
 * bug in N_block macros fixed
 *
 * Revision 1.3  1995/10/06  17:16:50  cg
 * basic access facilities for new type nodelist added
 * IDS structure modified to store global objects.
 * MakeIds extended to 3 parameters
 * basic facilities for compiler steps obj-analysis and fun-analysis added.
 *
 * Revision 1.2  1995/09/29  17:50:51  cg
 * new access structures for strings, nums, shpseg.
 * shape handling modified.
 *
 * Revision 1.1  1995/09/27  15:13:12  cg
 * Initial revision
 *
 *
 *
 */

/*============================================================================

How to read this file ?

This file contains the basic structure of the new virtual syntax tree.

First, it distinguishes between non-node structures such as SHAPES or
TYPES and node structures such as N_fundef or N_modul. The node
structures build up the main part of the syntax tree while the
non-node structures are designed for special purposes within the syntax
tree.

Each structure is described in three sections: the description section,
the remark section, and the implementation section. The description
section presents a detailed view of the structure in a formal way. The
optional remark section gives additional information in plain text where
necessary. The implementation section contains function declarations and
macro definitions.

The description section is organized as follows:
Each structure consists of a name and couple of slots. Each slot consists
of a type and a name. If the type is node*, then the required node type
is given as well. All types of global use are stored in types.h.

Three different kinds of slots are distinguished: sons, permant attributes,
and temporary attributes. All of them may be only optional which is marked
by an (O).

Sons are traversed (if present) by the automatic traversal mechanism.

Permanent attributes are unchanged throughout the
different compilation phases. They mainly contain information which is
derived from the SAC source code.

Temporary attributes contain additional
information which is gathered during one of the many compilation steps.
It may then stay unchanged in following compliation steps, be changed,
or even discarded. These situations are characterized in the following
way:

(typecheck -> )

means that the attribute is derived by the typechecker and then stays
unchanged.

(refcount -> compile -> )

means that the attribute is derived by the refcounter and is used during
compilation but it stays unchanged.

(typecheck -> compile !!)

means that the attribute is derived by the typechecker and is used
during compilation but is not available any further.

The following compilation steps are used:

 - scanparse
 - objinit
 - import
 - flatten
 - typecheck
 - impl_types
 - analysis
 - write-SIB
 - obj-handling
 - unique-check
 - rm-void-fun
 - optimize
   - inl  = function inlining
   - ael  = array elimination
   - wli  = withloop information
   - wlf  = withloop folding
   - dcr1 = search for redundant code
   - dcr2 = dead code removal
 - psi-optimize
 - refcount
 - precompile
 - compile

In general, the implementation section consists of the declaration of
a create function for the respective structure and one basic access
macro for each slot. The definitions of all functions declared in this
file can be found in tree_basic.c

============================================================================*/

#ifndef _sac_tree_basic_h

#define _sac_tree_basic_h

/* Uncomment the #define statement to use new virtual syntaxtree
 *
 * #define NEWTREE
 */

/*
 *   Global Access-Macros
 *   --------------------
 */

#define NODE_TYPE(n) ((n)->nodetype)

#define NODE_LINE(n) ((n)->lineno)

/*
 *   Non-node-structures
 *   -------------------
 */

/***
 ***  SHAPES :
 ***
 ***  permanent attributes:
 ***
 ***    int                DIM
 ***    int[SHP_SEG_SIZE]  SELEMS  (O)
 ***
 ***/

#define SHAPES_DIM(s) (s->dim)
#define SHAPES_SHPSEG(s) (s->shpseg)
#define SHAPES_SELEMS(s) (s->shpseg->shp)

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***
 ***  permanent attributes:
 ***
 ***    int[SHP_SEG_SIZE]  SHAPE
 ***
 ***/

extern shpseg *MakeShpseg (nums *num);

#define SHPSEG_SHAPE(s, x) (s->shp[x])

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
 ***
 ***  permanent attributes:
 ***
 ***    simpletype         BASETYPE
 ***    int                DIM
 ***    shpseg*            SHPSEG    (O)
 ***    char*              NAME      (O)
 ***    char*              MOD       (O)
 ***    types*             NEXT      (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*              TDEF      (O)  (typecheck -> )
 ***/

/*
 *  TDEF is a reference to the defining N_typedef node of a user-defined
 *  type (not yet implemented).
 */

extern types *MakeType (simpletype basetype, int dim, shpseg *shpseg, char *name,
                        char *mod);

#define TYPES_BASETYPE(t) (t->simpletype)
#define TYPES_DIM(t) (t->dim)
#define TYPES_SHPSEG(t) (t->shpseg)
#define TYPES_NAME(t) (t->name)
#define TYPES_MOD(t) (t->name_mod)
#define TYPES_TDEF(t) (t->tdef)
#define TYPES_NEXT(t) (t->next)

/*--------------------------------------------------------------------------*/

/***
 ***  IDS :
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD         (O)
 ***    statustype  ATTRIB
 ***    ids*        NEXT        (O)
 ***
 ***  temporary attributes:
 ***
 ***    int         REFCNT       (refcount -> )
 ***    node*       VARDEC       (typecheck -> )
 ***    node*       DEF          (psi-optimize -> )
 ***    node*       USE          (psi-optimize -> )
 ***    statustype  STATUS       (obj-handling -> compile !!)
 ***/

/*
 *  ATTRIB: ST_regular    :  local variable or function parameter
 *          ST_global     :  reference to global object
 *
 *  STATUS: ST_regular    :  from original source code
 *          ST_artificial :  added by obj-handling
 *
 */

extern ids *MakeIds (char *name, char *mod, statustype status);

#define IDS_NAME(i) (i->id)
#define IDS_MOD(i) (i->mod)
#define IDS_REFCNT(i) (i->refcnt)
#define IDS_NEXT(i) (i->next)
#define IDS_VARDEC(i) (i->node)
#define IDS_DEF(i) (i->def)
#define IDS_USE(i) (i->use)
#define IDS_STATUS(i) (i->status)
#define IDS_ATTRIB(i) (i->attrib)

/*--------------------------------------------------------------------------*/

/***
 ***  NUMS :
 ***
 ***  permanent attributes:
 ***
 ***    int    NUM
 ***    nums*  NEXT  (O)
 ***/

extern nums *MakeNums (int num, nums *next);

#define NUMS_NUM(n) (n->num)
#define NUMS_NEXT(n) (n->next)

/*--------------------------------------------------------------------------*/

/***
 ***  STRINGS :
 ***
 ***  permanent attributes:
 ***
 ***    char*  STRING
 ***    nums*  NEXT    (O)
 ***/

extern strings *MakeStrings (char *string, strings *next);

#define STRINGS_STRING(s) (s->name)
#define STRINGS_NEXT(s) (s->next)

/*--------------------------------------------------------------------------*/

/***
 ***  DEPS :
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       DECNAME
 ***    char*       LIBNAME
 ***    statustype  STATUS
 ***    deps*       SUB        (O)
 ***    deps*       NEXT       (O)
 ***/

extern deps *MakeDeps (char *name, char *decname, char *libname, statustype status,
                       deps *sub, deps *next);

#define DEPS_NAME(d) (d->name)
#define DEPS_DECNAME(d) (d->decname)
#define DEPS_LIBNAME(d) (d->libname)
#define DEPS_STATUS(d) (d->status)
#define DEPS_SUB(d) (d->sub)
#define DEPS_NEXT(d) (d->next)

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***
 ***  permanent attributes:
 ***
 ***    node*       NODE
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***    nodelist*   NEXT    (O)
 ***/

/*
 *  Possible values for ATTRIB :
 *      in function node lists : ST_resolved | ST_unresolved
 *      in object node lists   : ST_reference | ST_readonly_reference
 *      in typedef node lists  : ST_regular
 *  Possible values for STATUS : ST_regular | ST_artificial
 */

extern nodelist *MakeNodelist (node *node, statustype status, nodelist *next);

#define NODELIST_NODE(n) (n->node)
#define NODELIST_ATTRIB(n) (n->attrib)
#define NODELIST_STATUS(n) (n->status)
#define NODELIST_NEXT(n) (n->next)

/*==========================================================================*/

/*
 *   Node-structures
 *   ---------------
 */

/*--------------------------------------------------------------------------*/

/***
 ***  N_modul :
 ***
 ***  sons:
 ***
 ***    node*      IMPORTS   (O)  (N_implist)
 ***    node*      TYPES     (O)  (N_typedef)
 ***    node*      OBJS      (O)  (N_objdef)
 ***    node*      FUNS      (O)  (N_fundef)
 ***
 ***  permanent attributes:
 ***
 ***    char*      NAME      (O)
 ***    file_type  FILETYPE
 ***    types*     CLASSTYPE (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*      DECL      (O)  (N_moddec, N_classdec)  (check-dec -> )
 ***                                                      ( -> write-SIB !!)
 ***    node*      STORE_IMPORTS (O) (N_implist)          (import -> )
 ***                                                      ( -> checkdec !!)
 ***/

/*
 *  CLASSTYPE points to the type of a class implementation.
 *
 *  The temporary attributes DECL and STORE_IMPORTS are mapped
 *  to the same real node because they are never used in the same
 *  phase of compilation.
 */

extern node *MakeModul (char *name, file_type filetype, node *imports, node *types,
                        node *objs, node *funs);

#define MODUL_NAME(n) (n->info.id)
#define MODUL_FILETYPE(n) ((file_type) (n->varno))
#define MODUL_IMPORTS(n) (n->node[0])
#define MODUL_TYPES(n) (n->node[1])
#define MODUL_OBJS(n) (n->node[3])
#define MODUL_FUNS(n) (n->node[2])
#define MODUL_DECL(n) (n->node[4])
#define MODUL_STORE_IMPORTS(n) (n->node[4])
#define MODUL_CLASSTYPE(n) ((types *)(n->node[5]))

/*--------------------------------------------------------------------------*/

/***
 ***  N_moddec :
 ***
 ***  sons:
 ***
 ***    node*  IMPORTS    (O)  (N_implist)
 ***    node*  OWN        (O)  (N_explist)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    deps*  LINKWITH   (O)
 ***    int    ISEXTERNAL
 ***/

extern node *MakeModdec (char *name, deps *linkwith, int isexternal, node *imports,
                         node *exports);

#define MODDEC_NAME(n) (n->info.fun_name.id)
#define MODDEC_LINKWITH(n) ((deps *)(n->info.fun_name.id_mod))
#define MODDEC_ISEXTERNAL(n) (n->refcnt)
#define MODDEC_IMPORTS(n) (n->node[1])
#define MODDEC_OWN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_classdec :
 ***
 ***  sons:
 ***
 ***    node*  IMPORTS  (O)  (N_implist)
 ***    node*  OWN      (O)  (N_explist)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    deps*  LINKWITH (O)
 ***    int    ISEXTERNAL
 ***/

extern node *MakeClassdec (char *name, deps *linkwith, int isexternal, node *imports,
                           node *exports);

#define CLASSDEC_NAME(n) (n->info.fun_name.id)
#define CLASSDEC_LINKWITH(n) ((deps *)(n->info.fun_name.id_mod))
#define CLASSDEC_ISEXTERNAL(n) (n->refcnt)
#define CLASSDEC_IMPORTS(n) (n->node[1])
#define CLASSDEC_OWN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_sib :
 ***
 ***  sons:
 ***
 ***    node*     TYPES    (O)  (N_typedef)
 ***    node*     OBJS     (O)  (N_objdef)
 ***    node*     FUNS     (O)  (N_fundef)
 ***    node*     NEXT     (O)  (N_sib)
 ***
 ***  permanent attributes:
 ***
 ***    char*     NAME
 ***    int       LINKSTYLE
 ***    deps*     LINKWITH
 ***/

/*
 *  This node structure is used as head structure for SIBs.
 *  LINKSTYLE corresponds to the global variable linkstyle.
 */

extern node *MakeSib (char *name, int linkstyle, deps *linkwith, node *types, node *objs,
                      node *funs);

#define SIB_TYPES(n) (n->node[0])
#define SIB_OBJS(n) (n->node[1])
#define SIB_FUNS(n) (n->node[2])
#define SIB_LINKSTYLE(n) (n->varno)
#define SIB_NAME(n) (n->info.fun_name.id)
#define SIB_LINKWITH(n) ((deps *)(n->info.fun_name.id_mod))
#define SIB_NEXT(n) (n->node[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_implist :
 ***
 ***  sons:
 ***
 ***    node*  NEXT    (O)  (N_implist)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    ids*   ITYPES  (O)
 ***    ids*   ETYPES  (O)
 ***    ids*   OBJS    (O)
 ***    ids*   FUNS    (O)
 ***/

extern node *MakeImplist (char *name, ids *itypes, ids *etypes, ids *objs, ids *funs,
                          node *next);

#define IMPLIST_NAME(n) (n->info.id)
#define IMPLIST_ITYPES(n) ((ids *)(n->node[1]))
#define IMPLIST_ETYPES(n) ((ids *)(n->node[2]))
#define IMPLIST_OBJS(n) ((ids *)(n->node[4]))
#define IMPLIST_FUNS(n) ((ids *)(n->node[3]))
#define IMPLIST_NEXT(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_explist :
 ***
 ***  sons:
 ***
 ***    node* ITYPES  (O)  (N_typedef)
 ***    node* ETYPES  (O)  (N_typedef)
 ***    node* OBJS    (O)  (N_objdef)
 ***    node* FUNS    (O)  (N_fundef)
 ***/

extern node *MakeExplist (node *itypes, node *etypes, node *objs, node *funs);

#define EXPLIST_ITYPES(n) (n->node[0])
#define EXPLIST_ETYPES(n) (n->node[1])
#define EXPLIST_OBJS(n) (n->node[3])
#define EXPLIST_FUNS(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_typedef :
 ***
 ***  sons:
 ***
 ***    node*       NEXT  (O)  (N_typedef)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD     (O)
 ***    types*      TYPE
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    types*      IMPL         (O)        (import -> )
 ***                                        ( -> writesib !!)
 ***    node*       PRAGMA       (O)        (import -> readsib !!)
 ***    char*       COPYFUN      (O)        (readsib -> compile -> )
 ***    char*       FREEFUN      (O)        (readsib -> compile -> )
 ***    node*       TYPEDEC_DEF  (O)        (checkdec -> writesib !!)
 ***/

/*
 *  The STATUS indicates whether a type is defined or imported.
 *  Possible values: ST_regular | ST_imported
 *
 *  The TYPEDEC_DEF slot is only used when a typedef node is used as a
 *  representation of a type declaration. It then points to the
 *  typedef node which contains the respective definition.
 *
 *  For each Non-SAC hidden type the name of a copy and a free function
 *  is stored in COPYFUN and FREEFUN, respectively. These must be provided
 *  with the external module/class. The names may be generic or user-defined
 *  using pragmas.
 */

extern node *MakeTypedef (char *name, char *mod, types *type, statustype attrib,
                          node *next);

#define TYPEDEF_NAME(n) (n->info.types->id)
#define TYPEDEF_MOD(n) (n->info.types->id_mod)
#define TYPEDEF_TYPE(n) (n->info.types)
#define TYPEDEF_ATTRIB(n) (n->info.types->attrib)
#define TYPEDEF_STATUS(n) (n->info.types->status)
#define TYPEDEF_IMPL(n) (n->info.types->next)
#define TYPEDEF_NEXT(n) (n->node[0])
#define TYPEDEF_PRAGMA(n) (n->node[2])
#define TYPEDEF_COPYFUN(n) ((char *)(n->node[3]))
#define TYPEDEF_FREEFUN(n) ((char *)(n->node[4]))

#define TYPEDEC_DEF(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_objdef :
 ***
 ***  sons:
 ***
 ***    node*       EXPR  (O)  ("N_expr")
 ***    node*       NEXT  (O)  (N_objdef)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD     (O)
 ***    char*       LINKMOD (O)
 ***    types*      TYPE
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***
 ***  temporary attributes:
 ***
 ***    char*       VARNAME      (typecheck -> obj-handling ->
 ***                             ( -> precompile -> compile -> )
 ***    node*       PRAGMA    (O)  (N_pragma)  (import -> readsib ->
 ***                                            precompile -> )
 ***    node*       ARG       (O)  (obj-handling !!)
 ***    node*       ICM       (O)  (compile ->)
 ***    node*       SIB       (O)  (readsib !!)
 ***    nodelist*   NEEDOBJS  (O)  (import -> analysis -> objects -> )
 ***
 ***    node*       OBJDEC_DEF (O)  (checkdec -> writesib -> )
 ***/

/*
 *  The STATUS indicates whether an object is defined or imported.
 *  Possible values: ST_regular | ST_imported
 *
 *  ATTRIB : ST_regular | ST_resolved
 *  used in objects.c to distinguish between already initialized and
 *  not yet initialized global objects.
 *
 *  The VARNAME is a combination of NAME and MOD. It's used as parameter
 *  name when making global objects local.
 *
 *  ARG is a pointer to the additional argument which is added to a function's
 *  parameter list for this global object. ARG changes while traversing
 *  the functions !!
 *
 *  ICM contains a pointer to the respective icm if the global object
 *  is an array (ND_KS_DECL_ARRAY_GLOBAL or ND_KD_DECL_ARRAY_EXTERN)
 *
 *  ATTENTION: ARG, INIT, and ICM are mapped to the same real node !
 *
 *  LINKMOD contains the name of the module which has to be linked with
 *  in order to make the code of this function available. If LINKMOD is
 *  NULL, then link with the module given by MOD.
 *
 *  The OBJDEC_DEF slot is only used when an objdef node is used as a
 *  representation of an object declaration. It then points to the
 *  objdef node which contains the respective definition.
 *
 */

extern node *MakeObjdef (char *name, char *mod, types *type, node *expr, node *next);

#define OBJDEF_NAME(n) (n->info.types->id)
#define OBJDEF_MOD(n) (n->info.types->id_mod)
#define OBJDEF_LINKMOD(n) (n->info.types->id_cmod)
#define OBJDEF_TYPE(n) (n->info.types)
#define OBJDEF_EXPR(n) (n->node[1])
#define OBJDEF_NEXT(n) (n->node[0])
#define OBJDEF_STATUS(n) (n->info.types->status)
#define OBJDEF_ATTRIB(n) (n->info.types->attrib)
#define OBJDEF_VARNAME(n) ((char *)(n->node[2]))
#define OBJDEF_ARG(n) (n->node[3])
#define OBJDEF_PRAGMA(n) (n->node[4])
#define OBJDEF_ICM(n) (n->node[3])
#define OBJDEF_NEEDOBJS(n) ((nodelist *)(n->node[5]))
#define OBJDEF_SIB(n) (n->node[3])

#define OBJDEC_DEF(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_fundef :
 ***
 ***  sons:
 ***
 ***    node*           BODY     (O)   (N_block)
 ***    node*           ARGS     (O)   (N_arg)
 ***    node*           NEXT     (O)   (N_fundef)
 ***
 ***  permanent attributes:
 ***
 ***    char*           NAME
 ***    char*           MOD      (O)
 ***    char*           LINKMOD  (O)
 ***    types*          TYPES
 ***    statustype      STATUS
 ***    statustype      ATTRIB
 ***    int             INLINE
 ***    int             FUNNO
 ***    node*           PRAGMA   (O)   (N_pragma)
 ***
 ***  temporary attributes:
 ***
 ***    node*           SIB      (O)   (N_sib)    (readsib !!)
 ***    node*           RETURN         (N_return) (typecheck -> compile !!)
 ***    nodelist*       NEEDOBJS (O)              (import -> )
 ***                                              ( -> analysis -> )
 ***                                              ( -> write-SIB -> )
 ***                                              ( -> obj-handling -> )
 ***    node*           ICM            (N_icm)    (compile -> print )
 ***    int             VARNO                     (optimize -> )
 ***    long*           MASK[x]                   (optimize -> )
 ***    int             INLREC                    (inl !!)
 ***
 ***    DFMmask_base_t  DFM_BASE                  (refcount -> spmd -> )
 ***                                              ( -> compile -> )
 ***
 ***    node*           FUNDEC_DEF (O) (N_fundef) (checkdec -> writesib !!)
 ***
 ***/

/*
 *  STATUS: ST_regular      function defined in this module
 *          ST_objinitfun   generic function for object initialization
 *          ST_imported     imported function (maybe declaration only)
 *          ST_generic      class conversion function
 *          ST_spmdfun      function containing lifted SPMD-region
 *
 *  ATTRIB: ST_regular      dimension-dependent or non-array function
 *          ST_independent  dimension-independent array function
 *          ST_generic      generic function derived from dimension-
 *                          independent array function
 *
 *
 *  The FUNDEC_DEF slot is only used when a fundef node is used as a
 *  representation of a function declaration. It then points to the
 *  fundef node which contains the respective definition.
 *
 *  LINKMOD contains the name of the module which has to be linked with
 *  in order to make the code of this function available. If LINKMOD is
 *  NULL, then link with the module given by MOD.
 */

extern node *MakeFundef (char *name, char *mod, types *types, node *args, node *body,
                         node *next);

#define FUNDEF_FUNNO(n) (n->counter)
#define FUNDEF_NAME(n) (n->info.types->id)
#define FUNDEF_MOD(n) (n->info.types->id_mod)
#define FUNDEF_LINKMOD(n) (n->info.types->id_cmod)
#define FUNDEF_PRAGMA(n) (n->node[5])
#define FUNDEF_TYPES(n) (n->info.types)
#define FUNDEF_BODY(n) (n->node[0])
#define FUNDEF_ARGS(n) (n->node[2])
#define FUNDEF_NEXT(n) (n->node[1])
#define FUNDEF_RETURN(n) (n->node[3])
#define FUNDEF_SIB(n) (n->node[3])
#define FUNDEF_NEEDOBJS(n) ((nodelist *)(n->node[4]))
#define FUNDEF_ICM(n) (n->node[3])
#define FUNDEF_VARNO(n) (n->varno)
#define FUNDEF_MASK(n, x) (n->mask[x])
#define FUNDEF_STATUS(n) (n->info.types->status)
#define FUNDEF_ATTRIB(n) (n->info.types->attrib)
#define FUNDEF_INLINE(n) (n->flag)
#define FUNDEF_INLREC(n) (n->refcnt)
#define FUNDEF_DFM_BASE(n) (n->dfmask[0])

#define FUNDEC_DEF(n) (n->node[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_arg :
 ***
 ***  sons:
 ***
 ***    node*       NEXT    (O)  (N_arg)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    types*      TYPE
 ***    statustype  STATUS
 ***    statustype  ATTRIB
 ***
 ***  temporary attributes:
 ***
 ***    int         VARNO                        (optimize -> )
 ***    int         REFCNT                       (refcount -> compile -> )
 ***    char*       TYPESTRING (O)               (precompile -> )
 ***    node*       OBJDEF     (O)  (N_objdef)   (obj-handling ->
 ***                                             ( -> precompile !!)
 ***    node*       ACTCHN     (O)  (N_vinfo)    (psi-optimize -> )
 ***    node*       COLCHN     (O)  (N_vinfo)    (psi-optimize -> )
 ***    node*       FUNDEF     (O)  (N_fundef)   (psi-optimize -> )
 ***/

/*
 *  STATUS: ST_regular     original argument
 *          ST_artificial  additional argument added by object-handler
 *
 *  ATTRIB: ST_regular     non-unique parameter
 *          ST_unique      unique parameter
 *          ST_reference   (unique) reference parameter
 *          ST_readonly_reference (unique) reference parameter which remains
 *                                         unmodified
 *
 *  TYPESTRING contains the argument's type as a string, used for renaming
 *             of functions.
 */

extern node *MakeArg (char *name, types *type, statustype status, statustype attrib,
                      node *next);

#define ARG_NAME(n) (n->info.types->id)
#define ARG_TYPE(n) (n->info.types)
#define ARG_STATUS(n) (n->info.types->status)
#define ARG_ATTRIB(n) (n->info.types->attrib)
#define ARG_NEXT(n) (n->node[0])
#define ARG_VARNO(n) (n->varno)
#define ARG_REFCNT(n) (n->refcnt)
#define ARG_TYPESTRING(n) ((char *)(n->node[1]))
#define ARG_OBJDEF(n) (n->node[2])
#define ARG_ACTCHN(n) (n->node[3])
#define ARG_COLCHN(n) (n->node[4])
#define ARG_FUNDEF(n) (n->node[5])

/*--------------------------------------------------------------------------*/

/***
 ***  N_block :
 ***
 ***  sons:
 ***
 ***    node*      INSTR           (N_assign, N_empty)
 ***    node*      VARDEC     (O)  (N_vardec)
 ***
 ***  temporary attributes:
 ***
 ***    nodelist*  NEEDFUNS   (O)         (analysis -> )
 ***                                      ( -> analysis -> )
 ***                                      ( -> write-SIB -> )
 ***    nodelist*  NEEDTYPES  (O)         (analysis -> )
 ***                                      ( -> write-SIB -> )
 ***    long*      MASK[x]                (optimize -> )
 ***    int        VARNO                  (optimize -> )
 ***/

extern node *MakeBlock (node *instr, node *vardec);

#define BLOCK_INSTR(n) (n->node[0])
#define BLOCK_VARDEC(n) (n->node[1])
#define BLOCK_MASK(n, x) (n->mask[x])
#define BLOCK_NEEDFUNS(n) ((nodelist *)(n->node[2]))
#define BLOCK_NEEDTYPES(n) ((nodelist *)(n->node[3]))
#define BLOCK_VARNO(n) (n->varno)

/*--------------------------------------------------------------------------*/

/***
 ***  N_vardec :
 ***
 ***  sons:
 ***
 ***    node*       NEXT     (O)  (N_vardec)
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME          (NAME is an element of ...
 ***    types*      TYPE           ... this struct)
 ***    statustype  STATUS        (element of TYPE, too)
 ***
 ***  temporary attributes:
 ***
 ***    node*       TYPEDEF  (O)  (N_typedef)  (typecheck -> fun_analysis -> )
 ***    node*       OBJDEF   (O)  (N_objdef)   (inlining ->
 ***                                           ( -> precompile !!)
 ***    node*       ACTCHN   (O)  (N_vinfo)    (psi-optimize -> )
 ***    node*       COLCHN   (O)  (N_vinfo)    (psi-optimize -> )
 ***    int         REFCNT                     (refcount -> compile -> )
 ***    int         VARNO                      (optimize -> )
 ***    statustype  ATTRIB                     (typecheck -> uniquecheck -> )
 ***    int         FLAG                       (ael  -> dcr2 !! )
 ***/

/*
 * STATUS : ST_regular    :  original vardec in source code
 *          ST_used       :  after typecheck detected necessity of vardec
 *          ST_artificial :  artificial vardec produced by function inlining
 *                           of a function which uses a global object.
 *                           Such vardecs are removed by the precompiler.
 *
 * ATTRIB : ST_regular :  normal variable
 *          ST_unique  :  unique variable
 *
 * TYPEDEF is a reference to the respective typedef node if the type of
 * the declared variable is user-defined.
 *
 * srs: ? if FLAG is true this is an array vardec ?
 *
 */

extern node *MakeVardec (char *name, types *type, node *next);

#define VARDEC_NAME(n) (n->info.types->id)
#define VARDEC_TYPE(n) (n->info.types)
#define VARDEC_NEXT(n) (n->node[0])
#define VARDEC_VARNO(n) (n->varno)
#define VARDEC_REFCNT(n) (n->refcnt)
#define VARDEC_STATUS(n) (n->info.types->status)
#define VARDEC_ATTRIB(n) (n->info.types->attrib)
#define VARDEC_TYPEDEF(n) (n->node[1])
#define VARDEC_ACTCHN(n) (n->node[2])
#define VARDEC_COLCHN(n) (n->node[3])
#define VARDEC_FLAG(n) (n->flag)
#define VARDEC_OBJDEF(n) (n->node[4])

/*--------------------------------------------------------------------------*/

/***
 ***  N_assign :
 ***
 ***  sons:
 ***
 ***    node*  INSTR         ("N_instr")
 ***    node*  NEXT     (O)  (N_assign)
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                    (optimize -> )
 ***    int    STATUS                     (dcr1 -> dcr2 !!)
 ***    node*  CSE                        (CSE (GenerateMasks()) -> ??? )
 ***    void*  INDEX    (O)               (wli -> wlf ->)
 ***    int    LEVEL                      (wli !!)
 ***
 ***  remarks:
 ***   there is no easy way to remove the INDEX information after wlf (another
 ***   tree traversal would be necessary), so it stays afterwards.
 ***   Nevertheless only wlf will use it. The type of INDEX is index_info*,
 ***   defined in WithloopFolding.c (not in types.h).
 ***/

extern node *MakeAssign (node *instr, node *next);

#define ASSIGN_INSTR(n) (n->node[0])
#define ASSIGN_NEXT(n) (n->node[1])
#define ASSIGN_CSE(n) (n->node[2])
#define ASSIGN_MASK(n, x) (n->mask[x])
#define ASSIGN_STATUS(n) (n->flag)
#define ASSIGN_INDEX(n) (n->info2)
#define ASSIGN_LEVEL(n) (n->info.cint)

/*--------------------------------------------------------------------------*/

/***
 ***  N_let :					( one of "N_instr" )
 ***
 ***  sons:
 ***
 ***    node*  EXPR      ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    ids*   IDS   (O)
 ***
 ***/

extern node *MakeLet (node *expr, ids *ids);

#define LET_EXPR(n) (n->node[0])
#define LET_IDS(n) (n->info.ids)

/*--------------------------------------------------------------------------*/

/***
 ***  N_cast :
 ***
 ***  sons:
 ***
 ***    node*   EXPR  ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    types*  TYPE
 ***/

extern node *MakeCast (node *expr, types *type);

#define CAST_EXPR(n) (n->node[0])
#define CAST_TYPE(n) (n->info.types)

/*--------------------------------------------------------------------------*/

/***
 ***  N_return :
 ***
 ***  sons:
 ***
 ***    node*  EXPRS      (N_exprs)  (O)
 ***
 ***
 ***  permanent attributes:
 ***
 ***    int    INWITH
 ***
 ***  temporary attributes:
 ***
 ***    node*  REFERENCE  (N_exprs)  (O)  (precompile -> compile !!)
 ***/

/*
 *  REFERENCE: List of artificial return values which correspond to
 *             reference parameters.
 *
 *  INWITH is used to mark those return statements which are used in the
 *  internal representation of with loops.
 *
 *  ATTENTION: node[1] of N_return node already used by compile.c
 */

extern node *MakeReturn (node *exprs);

#define RETURN_EXPRS(n) (n->node[0])
#define RETURN_REFERENCE(n) (n->node[2])
#define RETURN_INWITH(n) (n->varno)

/*--------------------------------------------------------------------------*/

/***
 ***  N_cond :
 ***
 ***  sons:
 ***
 ***    node*  COND         ("N_expr")
 ***    node*  THEN         (N_block)
 ***    node*  ELSE         (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    node*  THENVARS     (N_exprs)   (refcount -> compile -> )
 ***    node*  ELSEVARS     (N_exprs)   (refcount -> compile -> )
 ***    long*  MASK[x]                  (optimize -> )
 ***/

extern node *MakeCond (node *cond, node *Then, node *Else);

#define COND_COND(n) (n->node[0])
#define COND_THEN(n) (n->node[1])
#define COND_ELSE(n) (n->node[2])
#define COND_THENVARS(n) (n->node[3])
#define COND_ELSEVARS(n) (n->node[4])
#define COND_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_do :
 ***
 ***  sons:
 ***
 ***    node*  COND       ("N_expr")
 ***    node*  BODY       (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    node*  USEVARS    (N_exprs)   (refcount -> compile -> )
 ***    node*  DEFVARS    (N_exprs)   (refcount -> compile -> )
 ***    long*  MASK[x]                (optimize -> )
 ***/

extern node *MakeDo (node *cond, node *body);

#define DO_COND(n) (n->node[0])
#define DO_BODY(n) (n->node[1])

#define DO_USEVARS(n) (n->node[2])
#define DO_DEFVARS(n) (n->node[3])

#define DO_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_while :
 ***
 ***  sons:
 ***
 ***    node*  COND       ("N_expr")
 ***    node*  BODY       (N_block)
 ***
 ***  temporary attributes:
 ***
 ***    node*  USEVARS    (N_exprs)   (refcount -> compile -> )
 ***    node*  DEFVARS    (N_exprs)   (refcount -> compile -> )
 ***    long*  MASK[x]                (optimize -> )
 ***/

extern node *MakeWhile (node *cond, node *body);

extern node *While2Do (node *while_node);

#define WHILE_COND(n) (n->node[0])
#define WHILE_BODY(n) (n->node[1])

#define WHILE_USEVARS(n) (n->node[2])
#define WHILE_DEFVARS(n) (n->node[3])

#define WHILE_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_annotate :                              ( one of "N_instr" )
 ***
 ***  permanent attributes:
 ***
 ***    int    TAG
 ***	int    FUNNUMBER
 ***    int    FUNAPNUMBER
 ***/

extern node *MakeAnnotate (int tag, int funnumber, int funapnumber);

#define CALL_FUN 0x0001
#define RETURN_FROM_FUN 0x0002
#define INL_FUN 0x0004
#define LIB_FUN 0x0008
#define OVRLD_FUN 0x0010
#define ANNOTATE_TAG(n) (n->flag)
#define ANNOTATE_FUNNUMBER(n) (n->counter)
#define ANNOTATE_FUNAPNUMBER(n) (n->varno)
#define ANNOTATE_FUN(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_ap :
 ***
 ***  sons:
 ***
 ***    node*  ARGS    (O)  (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    char*  MOD     (O)
 ***    int    ATFLAG  (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*  FUNDEF       (N_fundef)  (typecheck -> analysis -> )
 ***                                    ( -> obj-handling -> compile -> )
 ***/

extern node *MakeAp (char *name, char *mod, node *args);

#define AP_NAME(n) (n->info.fun_name.id)
#define AP_MOD(n) (n->info.fun_name.id_mod)
#define AP_ATFLAG(n) (n->counter)
#define AP_ARGS(n) (n->node[0])
#define AP_FUNDEF(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_with :
 ***
 ***  sons:
 ***
 ***    node*  GEN      (N_generator)
 ***    node*  OPERATOR (N_modarray, N_genarray, N_foldprf, N_foldfun)
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )
 ***    node*  USEDVARS (N_info)       (refcount -> ) ???
 ***
 ***  remark (srs): the 'body' of MakeWith() can be reached by WITH_OPERATOR.
 ***
 ***/

extern node *MakeWith (node *gen, node *body);

#define WITH_GEN(n) (n->node[0])
#define WITH_OPERATOR(n) (n->node[1])
#define WITH_MASK(n, x) (n->mask[x])
#define WITH_USEDVARS(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_generator :
 ***
 ***  sons:
 ***
 ***    node*  LEFT    ("N_expr")
 ***    node*  RIGHT   ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    char*  ID
 ***    ids*   IDS
 ***
 ***  temporary attributes:
 ***
 ***    node*  VARDEC                  (typechecker -> )
 ***    long*  MASK[x]                 (optimize -> )
 ***    node*  USE     (O) (N_vinfo)   (psi-optimize -> )
 ***
 ***  remark: IDS->id == ID
 ***
 ***/

extern node *MakeGenerator (node *left, node *right, char *id);

#define GEN_LEFT(n) (n->node[0])
#define GEN_RIGHT(n) (n->node[1])
#define GEN_ID(n) (n->info.ids->id)
#define GEN_IDS(n) (n->info.ids)
#define GEN_USE(n) (n->info.ids->use)
#define GEN_VARDEC(n) (n->info.ids->node)
#define GEN_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_genarray :
 ***
 ***  sons:
 ***
 ***    node*  ARRAY  (N_array)
 ***    node*  BODY   (N_block)
 ***
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )
 ***/

extern node *MakeGenarray (node *array, node *body);

#define GENARRAY_ARRAY(n) (n->node[0])
#define GENARRAY_BODY(n) (n->node[1])
#define OPERATOR_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_modarray :
 ***
 ***  sons:
 ***
 ***    node*  ARRAY  (N_id)
 ***    node*  BODY   (N_block)
 ***
 ***
 ***  permanent attributes:
 ***
 ***    char*  ID
 ***
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )  (see N_genarray)
 ***/

extern node *MakeModarray (node *array, node *body);

#define MODARRAY_ARRAY(n) (n->node[0])
#define MODARRAY_BODY(n) (n->node[1])
#define MODARRAY_ID(n) (n->info.id)

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldprf :
 ***
 ***  sons:
 ***
 ***    node*  BODY         (N_block)
 ***    node*  NEUTRAL  (O) ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    prf    PRF
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK[x]                 (optimize -> )  (see N_genarray)
 ***/

extern node *MakeFoldprf (prf prf, node *body, node *neutral);

#define FOLDPRF_PRF(n) (n->info.prf)
#define FOLDPRF_BODY(n) (n->node[0])
#define FOLDPRF_NEUTRAL(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_foldfun :
 ***
 ***  sons:
 ***
 ***    node*  BODY          (N_block)
 ***    node*  NEUTRAL       ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***    char*  MOD                (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*  FUNDEF        (N_fundef)  (typecheck -> )
 ***    long*  MASK[x]                   (optimize -> )  (see N_genarray)
 ***/

extern node *MakeFoldfun (char *name, char *mod, node *body, node *neutral);

#define FOLDFUN_NAME(n) (n->info.fun_name.id)
#define FOLDFUN_MOD(n) (n->info.fun_name.id_mod)
#define FOLDFUN_BODY(n) (n->node[0])
#define FOLDFUN_NEUTRAL(n) (n->node[1])
#define FOLDFUN_FUNDEF(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_exprs :
 ***
 ***  sons:
 ***
 ***    node*  EXPR       ("N_expr")
 ***    node*  NEXT  (O)  (N_exprs)
 ***/

extern node *MakeExprs (node *expr, node *next);

#define EXPRS_EXPR(n) (n->node[0])
#define EXPRS_NEXT(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_array :
 ***
 ***  sons:
 ***
 ***    node*  AELEMS   (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    char*  STRING       (O)
 ***
 ***  temporary attributes:
 ***
 ***    types* TYPE               (typecheck -> )
 ***/

/*
 * In the case of constant character arrays defined as strings, the
 * optional permanent attribute STRING holds the original definition.
 * This may be retrieved for C code generation.
 */

extern node *MakeArray (node *aelems);

#define ARRAY_AELEMS(n) (n->node[0])
#define ARRAY_TYPE(n) (n->info.types)
#define ARRAY_STRING(n) ((char *)(n->node[1]))

/*--------------------------------------------------------------------------*/

/***
 ***  N_vinfo :
 ***
 ***  sons:
 ***
 ***    node*    NEXT  (O)  (N_vinfo)
 ***
 ***  permanent attributes:
 ***
 ***    useflag  FLAG
 ***    types*   TYPE   (O)
 ***    node*    VARDEC (O)  (N_vardec)
 ***
 ***/

extern node *MakeVinfo (useflag flag, types *type, node *next);

#define VINFO_FLAG(n) (n->info.use)
#define VINFO_TYPE(n) ((types *)(n->node[1]))
#define VINFO_NEXT(n) (n->node[0])
#define VINFO_VARDEC(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_id :
 ***
 ***  permanent attributes:
 ***
 ***    char*       NAME
 ***    char*       MOD     (O)
 ***    statustype  ATTRIB
 ***    statustype  STATUS
 ***    ids*        IDS
 ***
 ***  temporary attributes:
 ***
 ***    node*  VARDEC    (N_vardec/N_arg)  (typecheck -> )
 ***    node*  OBJDEF    (N_objdef)        (typecheck -> )
 ***                                       ( -> analysis -> )
 ***    int    REFCNT                      (refcount -> compile -> )
 ***    int    MAKEUNIQUE                  (precompile -> compile -> )
 ***    node*  DEF                         (Unroll !, Unswitch !)
 ***    node*  WL        (O)               (wli -> wlf !!)
 ***
 ***  remarks:
 ***    ID_WL is only used in wli, wlf. But every call of DupTree() initializes
 ***    the copy's WL_ID with a pointer to it's original N_id node. The function
 ***    SearchWL() can define ID_WL in another way (pointer to N_assign node
 ***    of WL which is referenced by this Id).
 ***
 ***    Unroll uses ->flag without a macro :(
 ***    Even worse: Unroll uses ->flag of *every* LET_EXPR node :(((
 ***/

/*
 *  STATUS:  ST_regular     original argument
 *                          in a function application or return-statement
 *           ST_artificial  additional argument added by object-handler
 *                          in a function application or return-statement
 *
 *  ATTRIB:  ST_regular     ordinary argument
 *                          in a function application or return-statement
 *           ST_global      global object
 *           ST_readonly_reference/
 *           ST_reference   argument in a function application which
 *                          is passed as a reference parameter or
 *                          additional argument in a return-statement
 *                          which belongs to a reference parameter
 *
 *  MAKEUNIQUE is a flag which is set in those N_id nodes which were
 *  arguments to class conversion function.
 */

extern node *MakeId (char *name, char *mod, statustype status);

extern node *MakeId2 (ids *ids_node);

#define ID_IDS(n) (n->info.ids)
#define ID_NAME(n) (n->info.ids->id)
#define ID_DEF(n) (n->info.ids->def)
#define ID_VARDEC(n) (n->info.ids->node)
#define ID_OBJDEF(n) (n->info.ids->node)
#define ID_MOD(n) (n->info.ids->mod)
#define ID_ATTRIB(n) (n->info.ids->attrib)
#define ID_STATUS(n) (n->info.ids->status)
#define ID_REFCNT(n) (n->refcnt)
#define ID_MAKEUNIQUE(n) (n->flag)
#define ID_WL(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_num :
 ***
 ***  permanent attributes:
 ***
 ***    int  VAL
 ***/

extern node *MakeNum (int val);

#define NUM_VAL(n) (n->info.cint)

/*--------------------------------------------------------------------------*/

/***
 ***  N_char :
 ***
 ***  permanent attributes:
 ***
 ***    char  VAL
 ***/

extern node *MakeChar (char val);

#define CHAR_VAL(n) (n->info.cchar)

/*--------------------------------------------------------------------------*/

/***
 ***  N_float :
 ***
 ***  permanent attributes:
 ***
 ***    float  VAL
 ***/

extern node *MakeFloat (float val);

#define FLOAT_VAL(n) (n->info.cfloat)

/*--------------------------------------------------------------------------*/

/***
 ***  N_double :
 ***
 ***  permanent attributes:
 ***
 ***    double  VAL
 ***/

extern node *MakeDouble (double val);

#define DOUBLE_VAL(n) (n->info.cdbl)

/*--------------------------------------------------------------------------*/

/***
 ***  N_bool :
 ***
 ***  permanent attributes:
 ***
 ***    int  VAL
 ***/

extern node *MakeBool (int val);

#define BOOL_VAL(n) (n->info.cint)

/*--------------------------------------------------------------------------*/

/***
 ***  N_str :
 ***
 ***  permanent attributes:
 ***
 ***    char*  STRING
 ***/

extern node *MakeStr (char *str);

#define STR_STRING(n) (n->info.id)

/*--------------------------------------------------------------------------*/

/***
 ***  N_prf :
 ***
 ***  sons:
 ***
 ***     node*  ARGS   (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***     prf    PRF
 ***/

extern node *MakePrf (prf prf, node *args);

#define PRF_PRF(n) (n->info.prf)
#define PRF_ARGS(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_empty :
 ***/

extern node *MakeEmpty ();

/*--------------------------------------------------------------------------*/

/***
 ***  N_post :
 ***
 ***  permanent attributes:
 ***
 ***    node*  INCDEC
 ***    char*  ID
 ***
 ***  temporary attributes:
 ***
 ***    node*  DECL    (N_vardec)  (typecheck -> )
 ***    int    REFCNT              (refcount  -> )
 ***/

extern node *MakePost (int incdec, char *id);

#define POST_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)
#define POST_ID(n) (n->info.id)
#define POST_DECL(n) (n->node[1])
#define POST_REFCNT(n) (n->info.ids->refcnt)

/*
 * Attention : The way incrementations and decrementation are represented
 * is not changed up to now. The macro POST_INCDEC must not be used on the
 * left side of the assignment operator. In comments the new representation
 * is shown. The MakePost function is intended to support both ways.
 */

/*--------------------------------------------------------------------------*/

/***
 ***  N_ok :
 ***
 ***  dummynode, last declared in node_info.mac
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_inc :
 ***  N_dec :
 ***
 ***  no description yet
 ***/

/*--------------------------------------------------------------------------*/

/***
 ***  N_stop :
 ***
 ***  no description yet
 ***  barely used in typecheck.c
 ***/
/*--------------------------------------------------------------------------*/

/***
 ***  N_pre :
 ***
 ***  permanent attributes:
 ***
 ***    node*  INCDEC
 ***    char*  ID
 ***
 ***  temporary attributes:
 ***
 ***    node*  DECL    (N_vardec)  (typecheck -> )
 ***    int    REFCNT              (refcount  -> )
 ***/

extern node *MakePre (nodetype incdec, char *id);

#define PRE_INCDEC(n) ((n->node[0]->nodetype == N_dec) ? 0 : 1)
#define PRE_ID(n) (n->info.id)
#define PRE_DECL(n) (n->node[1])
#define PRE_REFCNT(n) (n->info.ids->refcnt)

/*
 * Attention : The way incrementations and decrementation are represented
 * is not changed up to now. The macro POST_INCDEC must not be used on the
 * left side of the assignment operator. In comments the new representation
 * is shown. The MakePost function is intended to support both ways.
 */

/*--------------------------------------------------------------------------*/

/***
 ***  N_icm :
 ***
 ***  sons:
 ***
 ***    node*  ARGS  (O)  (N_exprs)
 ***    node*  NEXT  (O)  (N_icm)
 ***
 ***  permanent attributes:
 ***
 ***    char*  NAME
 ***
 *** remarks:
 ***    NEXT at least (!) is used for the compilation of N_typedef's
 ***    whenever the defining type is an array type!!
 ***/

extern node *MakeIcm (char *name, node *args, node *next);

#define ICM_NAME(n) (n->info.fun_name.id)
#define ICM_ARGS(n) (n->node[0])
#define ICM_NEXT(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 ***  N_pragma :
 ***
 ***  permanent attributes:
 ***
 ***    char*  LINKNAME         (O)
 ***    int[]  LINKSIGN         (O)
 ***    int[]  REFCOUNTING      (O)
 ***    char*  INITFUN          (O)
 ***
 ***    node*  WLCOMP_APS       (0)      (N_exprs)
 ***
 ***  temporary attributes:
 ***
 ***    int[]  READONLY         (O)   (import -> readsib !!)
 ***    ids*   EFFECT           (O)   (import -> readsib !!)
 ***    ids*   TOUCH            (O)   (import -> readsib !!)
 ***    char*  COPYFUN          (O)   (import -> readsib !!)
 ***    char*  FREEFUN          (O)   (import -> readsib !!)
 ***    ids*   NEEDTYPES        (O)   (import -> readsib !!)
 ***    node*  NEEDFUNS         (O)   (import -> readsib !!)
 ***    char*  LINKMOD          (O)   (import -> readsib !!)
 ***    int    NUMPARAMS        (O)   (import -> readsib !!)
 ***
 ***    nums*  LINKSIGNNUMS     (O)   (scanparse -> import !!)
 ***    nums*  REFCOUNTINGNUMS  (O)   (scanparse -> import !!)
 ***    nums*  READONLYNUMS     (O)   (scanparse -> import !!)
 ***
 ***/

/*
 *  Not all pragmas may occur at the same time:
 *  A typedef pragma may contain COPYFUN and FREEFUN.
 *  A objdef pragma may contain LINKNAME only.
 *  And a fundef pragma may contain all pragmas except COPYFUN and FREEFUN,
 *  but LINKMOD, TYPES and FUNS are only for internal use in SIBS.
 *  A wlcomp pragma contains WLCOMP only.
 *
 *  NUMPARAMS is not a pragma but gives the number of parameters of the
 *  function (return values + arguments). This is the size of the arrays
 *  which store the LINKSIGN, REFCOUNTING, and READONLY pragmas.
 *
 *  The temporary attributes serve only for parsing the respective
 *  pragmas. Immediately after parsing, the pragmas are checked and
 *  converted into the array representation. So, all usage of these
 *  pragmas must rely on the respective permanent attributes.
 */

extern node *MakePragma ();

#define PRAGMA_LINKNAME(n) (n->info.id)
#define PRAGMA_LINKSIGN(n) ((int *)(n->mask[0]))
#define PRAGMA_LINKSIGNNUMS(n) ((nums *)(n->mask[0]))
#define PRAGMA_REFCOUNTING(n) ((int *)(n->mask[1]))
#define PRAGMA_REFCOUNTINGNUMS(n) ((nums *)(n->mask[1]))
#define PRAGMA_READONLY(n) ((int *)(n->mask[2]))
#define PRAGMA_READONLYNUMS(n) ((nums *)(n->mask[2]))
#define PRAGMA_EFFECT(n) ((ids *)(n->mask[3]))
#define PRAGMA_TOUCH(n) ((ids *)(n->mask[4]))
#define PRAGMA_COPYFUN(n) ((char *)(n->mask[5]))
#define PRAGMA_FREEFUN(n) ((char *)(n->mask[6]))
#define PRAGMA_INITFUN(n) ((char *)(n->node[3]))
#define PRAGMA_LINKMOD(n) ((char *)(n->node[2]))
#define PRAGMA_NEEDTYPES(n) ((ids *)(n->node[1]))
#define PRAGMA_NEEDFUNS(n) (n->node[0])
#define PRAGMA_NUMPARAMS(n) (n->flag)

#define PRAGMA_WLCOMP_APS(n) (n->node[0])

/*--------------------------------------------------------------------------*/

/***
 ***  N_info :
 ***
 ***  The N_info node is used to store additional compile time information
 ***  outside the syntax tree. So, its concrete look depends on the
 ***  specific task.
 ***
 ***  when used in flatten.c:
 ***
 ***    contextflag CONTEXT       (O)
 ***    node *      LASTASSIGN    (O)  (N_assign)
 ***
 ***  when used in typecheck.c :
 ***
 ***    node *     NEXTASSIGN    (O)  (N_assign)
 ***    node *     LASSIGN       (0)  (N_assign)
 ***
 ***  when used in writesib.c :
 ***
 ***    nodelist*  EXPORTTYPES   (O)
 ***    nodelist*  EXPORTOBJS    (O)
 ***    nodelist*  EXPORTFUNS    (O)
 ***
 ***  when used in precompile.c :
 ***
 ***    char*      NAME          (0)
 ***    node*      FUNDEFS       (0)  (N_fundef)
 ***
 ***  when used in compile.c :
 ***
 ***    ids*       LASTIDS       (O)
 ***    node*      LASTLET       (O)  (N_let)
 ***    node*      LASTASSIGN    (O)  (N_assign)
 ***    node*      VARDECS       (O)  (N_vardec)
 ***    node*      WITHBEGIN     (O)  (N_icm)
 ***
 ***    node*      FIRSTASSIGN   (O)  (N_assign)
 ***    node*      FUNDEF        (O)  (N_fundef)
 ***    int        CNTPARAM
 ***    node**     ICMTAB        (O)
 ***    types**    TYPETAB       (O)
 ***
 ***  when used in optimize.c :
 ***
 ***    long*      MASK[x]
 ***
 ***  when used while withloop folding:
 ***    node*      NEXT               (N_info)
 ***    node*      SUBST              (N_Ncode)
 ***    node*      WL                 (N_Nwith)
 ***    node*      NEW_ID             (N_id)
 ***    node*      ASSIGN             (N_assign)
 ***    node*      FUNDEF             (N_fundef)
 ***    int        FLAG               (0/1)
 ***    node*      ID                 (N_id)
 ***    node*      NCA                (N_assign) (new code assignments)
 ***    node*      LET                (N_let)
 ***    node*      REPLACE            (N_id, N_array or N_num/float...)
 ***
 ***  when used in ConstantFolding.c :
 ***    node*      ASSIGN             (N_assign)
 ***    types      TYPE               (no son)
 ***
 *** remarks:
 ***    N_info is used in many other phases without access macros :((
 ***/

/*
 *  When used in writesib.c, the N_info node collects lists of nodes which
 *  have to be printed to the SIB.
 */

/*
 * srs: the number of sons being traversed is set to MAX_SONS, so
 *  don't use (temporary) attributes in the node-slots.
 */

extern node *MakeInfo ();

/* DupTree
 *
 * ATTENTION: Usage of DUP and INL macros on arg_info are mixed. Be careful
 *            to avoid overlapping addresses.
 */
#define INFO_DUP_CONT(n) (n->node[1])

/* flatten */
#define INFO_FLTN_CONTEXT(n) (n->flag)
#define INFO_FLTN_LASTASSIGN(n) (n->node[0])

/* typecheck */
#define INFO_TC_NEXTASSIGN(n) (n->node[1])
#define INFO_TC_LASSIGN(n) (n->node[3])

/* writesib */
#define INFO_EXPORTTYPES(n) ((nodelist *)(n->node[0]))
#define INFO_EXPORTOBJS(n) ((nodelist *)(n->node[1]))
#define INFO_EXPORTFUNS(n) ((nodelist *)(n->node[2]))

/* refcount */
#define INFO_RC_PRF(n) (n->node[0])
#define INFO_RC_WITH(n) (n->node[1])
#define INFO_RC_RCDUMP(n) ((int *)(n->node[2]))

/* spmdregions */
#define INFO_SPMD_FUNDEF(n) (n->node[0])
#define INFO_SPMD_FIRST(n) (n->flag)

/* precompile */
#define INFO_PREC_MODUL(n) (n->node[0])
#define INFO_PREC_CNT_ARTIFICIAL(n) (n->lineno)

/* ArrayElemination */
#define INFO_AE_TYPES(n) (n->node[1])

/* compile */
#define INFO_COMP_LASTASSIGN(n) (n->node[0])
#define INFO_COMP_LASTLET(n) (n->node[1])
#define INFO_COMP_LASTIDS(n) (n->info.ids)
#define INFO_COMP_FUNDEF(n) (n->node[2])
#define INFO_COMP_VARDECS(n) (n->node[3])
#define INFO_COMP_WITHBEGIN(n) (n->node[4])

#define INFO_COMP_FIRSTASSIGN(n) (n->node[0])
#define INFO_COMP_CNTPARAM(n) (n->lineno)
#define INFO_COMP_ICMTAB(n) ((node **)(n->node[1]))
#define INFO_COMP_TYPETAB(n) ((types **)(n->info.types))

/* optimize */
#define INFO_MASK(n, x) (n->mask[x])

/* inline */
/* ATTENTION: Usage of DUP and INL macros on arg_info are mixed. Be careful
   to avoid overlapping addresses. */
#define INFO_INL_FIRST_FUNC(n) (n->node[0])
#define INFO_INL_TYPES(n) (n->node[2])

/* WLF, all phases of WLF use these macros, not only WLI. */
#define INFO_WLI_NEXT(n) (n->node[0])
#define INFO_WLI_SUBST(n) (n->node[0])
#define INFO_WLI_WL(n) (n->node[1])
#define INFO_WLI_NEW_ID(n) (n->node[1])
#define INFO_WLI_ASSIGN(n) (n->node[2])
#define INFO_WLI_FUNDEF(n) (n->node[3])
#define INFO_WLI_ID(n) (n->node[4])
#define INFO_WLI_LET(n) (n->node[4])
#define INFO_WLI_NCA(n) (n->node[5])
#define INFO_WLI_REPLACE(n) (n->node[5])
#define INFO_WLI_FLAG(n) (n->flag)

/* CF */
#define INFO_CF_ASSIGN(n) (n->node[0])
#define INFO_CF_TYPE(n) (n->info.types)

/* Icm2c, ... */
#define INFO_FUNDEF(n) (n->node[0])

/* Print */
#define INFO_PRINT_FUNDEF(n) (n->node[0])
#define INFO_PRINT_INT_SYN(n) (n->node[2])
#define INFO_PRINT_WITH_RET(n) (n->node[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_spmd :
 ***
 ***  sons:
 ***
 ***    node*      REGION      (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    ---
 ***
 ***  temporary attributes:
 ***
 ***    ids*       INOUT_IDS             (spmdinit -> compile -> )
 ***
 ***    DFMmask_t  IN                    (spmdinit -> spmd... -> compile -> )
 ***    DFMmask_t  OUT                   (spmdinit -> spmd... -> compile -> )
 ***    DFMmask_t  INOUT                 (spmdinit -> spmd... -> compile -> )
 ***    DFMmask_t  LOCAL                 (spmdinit -> spmd... -> )
 ***
 ***    char*      LIFTED_FROM           (spmdlift -> compile -> )
 ***    char*      FUNNAME               (spmdlift -> compile -> )
 ***    node*      ICM         (N_icm)   (compile -> print -> )
 ***
 ***  remarks:
 ***
 ***    INOUT_IDS contains all LET_IDS(...) of the inout-lets found in REGION.
 ***    This is needed by 'compile' to find the right RCs.
 ***
 ***/

extern node *MakeSpmd (node *region);

#define SPMD_REGION(n) (n->node[0])

#define SPMD_INOUT_IDS(n) ((ids *)(n->node[1]))

#define SPMD_IN(n) ((DFMmask_t)n->dfmask[0])
#define SPMD_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define SPMD_OUT(n) ((DFMmask_t)n->dfmask[2])
#define SPMD_LOCAL(n) ((DFMmask_t)n->dfmask[3])

#define SPMD_LIFTED_FROM(n) ((char *)(n->node[3]))
#define SPMD_FUNNAME(n) (n->info.id)
#define SPMD_ICM(n) (n->node[2])

/*--------------------------------------------------------------------------*/

/***
 ***  N_sync :
 ***
 ***  sons:
 ***
 ***    node*      REGION      (N_block)
 ***
 ***  permanent attributes:
 ***
 ***    int        FIRST            (is the sync-region the first one
 ***                                 of the current SPMD-region?)
 ***
 ***  temporary attributes:
 ***
 ***    DFMmask_t  IN                    (spmdinit -> spmd... -> compile -> )
 ***    DFMmask_t  OUT                   (spmdinit -> spmd... -> compile -> )
 ***    DFMmask_t  INOUT                 (spmdinit -> spmd... -> compile -> )
 ***    DFMmask_t  LOCAL                 (spmdinit -> spmd... -> compile -> )
 ***
 ***  remarks:
 ***
 ***    INOUT_IDS contains all LET_IDS(...) of the inout-lets found in REGION.
 ***    This is needed by 'compile' to find the right RCs.
 ***
 ***/

extern node *MakeSync (node *region, int first);

#define SYNC_REGION(n) (n->node[0])
#define SYNC_FIRST(n) (n->flag)

#define SYNC_IN(n) ((DFMmask_t)n->dfmask[0])
#define SYNC_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define SYNC_OUT(n) ((DFMmask_t)n->dfmask[2])
#define SYNC_LOCAL(n) ((DFMmask_t)n->dfmask[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith :
 ***
 ***  sons:
 ***
 ***    node*      PART       (N_Npart)
 ***    node*      CODE       (N_Ncode)
 ***    node*      WITHOP     (N_Nwithop)
 ***
 ***  permanent attributes:
 ***
 ***    int        PARTS   (number of N_Npart nodes for this WL.
 ***                         -1: no complete partition, exactly one N_Npart,
 ***                         >0: complete partition.
 ***
 ***  temporary attributes:
 ***
 ***    node*      PRAGMA     (N_pragma)  (scanparse -> precompile ! )
 ***    int        REFERENCED             (wli -> wlf !!
 ***    int        REFERENCED_FOLD        (wli -> wlf !!)
 ***    int        COMPLEX                (wli -> wlf !!)
 ***    int        FOLDABLE               (wli -> wlf !!)
 ***    int        NO_CHANCE              (wli -> wlf !!)
 ***    ids*       DEC_RC_IDS             (refcount -> wltransform )
 ***
 ***    DFMmask_t  IN                     (refcount -> wltransform )
 ***    DFMmask_t  INOUT                  (refcount -> wltransform )
 ***    DFMmask_t  OUT                    (refcount -> wltransform )
 ***    DFMmask_t  LOCAL                  (refcount -> wltransform )
 ***/

extern node *MakeNWith (node *part, node *code, node *withop);

#define NWITH_PART(n) (n->node[0])
#define NWITH_CODE(n) (n->node[1])
#define NWITH_WITHOP(n) (n->node[2])
#define NWITH_PRAGMA(n) (n->node[3])

#define NWITH_PARTS(n) (((wl_info *)(n->info2))->parts)
#define NWITH_REFERENCED(n) (((wl_info *)(n->info2))->referenced)
#define NWITH_REFERENCED_FOLD(n) (((wl_info *)(n->info2))->referenced_fold)
#define NWITH_COMPLEX(n) (((wl_info *)(n->info2))->complex)
#define NWITH_FOLDABLE(n) (((wl_info *)(n->info2))->foldable)
#define NWITH_NO_CHANCE(n) (((wl_info *)(n->info2))->no_chance)
#define NWITH_DEC_RC_IDS(n) ((ids *)(n->node[4]))

#define NWITH_IN(n) ((DFMmask_t)n->dfmask[0])
#define NWITH_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define NWITH_OUT(n) ((DFMmask_t)n->dfmask[2])
#define NWITH_LOCAL(n) ((DFMmask_t)n->dfmask[3])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Npart :
 ***
 ***  sons:
 ***
 ***    node*  WITHID        (N_Nwithid)
 ***    node*  GEN           (N_Ngenerator)
 ***    node*  NEXT      (O) (N_Npart)
 ***
 ***  permanent attributes:
 ***
 ***    node*  CODE          (N_Ncode)
 ***
 ***  temporary attributes:
 ***
 ***    long*  MASK          (optimize -> )
 ***    int    COPY          (Unroll!)
 ***
 ***
 ***/

extern node *MakeNPart (node *withid, node *generator, node *code);

#define NPART_WITHID(n) (n->node[0])
#define NPART_GEN(n) (n->node[1])
#define NPART_NEXT(n) (n->node[2])
#define NPART_CODE(n) (n->node[3])
#define NPART_MASK(n, x) (n->mask[x])
#define NPART_COPY(n) (n->flag)

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithid :
 ***
 ***  permanent attributes:
 ***
 ***    ids*         VEC
 ***    ids*         IDS
 ***/

extern node *MakeNWithid (ids *vec, ids *scalars);

#define NWITHID_VEC(n) (n->info.ids)
#define NWITHID_IDS(n) ((ids *)(n->info2))

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ngenerator :
 ***
 ***  sons:
 ***
 ***    node*  BOUND1    (O)  ("N_expr")
 ***    node*  BOUND2    (O)  ("N_expr")
 ***    node*  STEP      (O)  ("N_expr")
 ***    node*  WIDTH     (O)  ("N_expr")
 ***
 ***  permanent attributes:
 ***
 ***    prf    OP1
 ***    prf    OP2
 ***
 ***  remarks:
 ***    the BOUNDs are NULL if upper or lower bounds are not specified.
 ***    if STEP is NULL, step 1 is assumed (no grid)
 ***    if WIDTH is NULL, width 1 is assumed
 ***
 ***/

extern node *MakeNGenerator (node *bound1, node *bound2, prf op1, prf op2, node *step,
                             node *width);

#define NGEN_BOUND1(n) (n->node[0])
#define NGEN_BOUND2(n) (n->node[1])
#define NGEN_STEP(n) (n->node[2])
#define NGEN_WIDTH(n) (n->node[3])
#define NGEN_OP1(n) (n->info.genrel.op1)
#define NGEN_OP2(n) (n->info.genrel.op2)

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwithop :
 ***
 ***  the meaning of the sons/attributes of this node depend on WithOpType.
 ***
 ***  sons:
 ***    node*  SHAPE      ("N_expr": N_array, N_id) (iff WithOpType == WO_genarray)
 ***    node*  ARRAY      ("N_expr": N_array, N_id) (iff WithOpType == WO_modarray)
 ***    node*  NEUTRAL    ("N_expr")                (otherwise )
 ***
 ***  permanent attributes:
 ***
 ***    WithOpType TYPE
 ***    char*      FUN         (iff WithOpType == WO_foldfun)
 ***    char*      MOD         (iff WithOpType == WO_foldfun)
 ***    prf        PRF         (iff WithOpType == WO_foldprf)
 ***
 ***  temporary attributes:
 ***
 ***    node*  EXPR            (scanparse, NULL afterwards)
 ***    node*  FUNDEF          (N_fundef)  (typecheck -> )
 ***    long*  MASK                        (optimize -> )
 ***
 ***  remarks:
 ***    WithOpType is WO_genarray, WO_modarray, WO_foldfun, WO_foldprf.
 ***    FUNDEF-node is used if TYPE == WO_foldfun.
 ***
 ***/

extern node *MakeNWithOp (WithOpType WithOp);

#define NWITHOP_TYPE(n) (*((WithOpType *)(n)->info2))
#define NWITHOP_SHAPE(n) (n->node[0])
#define NWITHOP_ARRAY(n) (n->node[0])
#define NWITHOP_NEUTRAL(n) (n->node[0])
#define NWITHOP_EXPR(n) (n->node[1])
#define NWITHOP_FUN(n) (n->info.fun_name.id)
#define NWITHOP_MOD(n) (n->info.fun_name.id_mod)
#define NWITHOP_PRF(n) (n->info.prf)
#define NWITHOP_FUNDEF(n) (n->node[2])
#define NWITHOP_MASK(n, x) (n->mask[x])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Ncode :
 ***
 ***  sons:
 ***
 ***    node*  CBLOCK    (O) (N_block)
 ***    node*  CEXPR         ("N_expr")
 ***    node*  NEXT      (O) (N_exprs)
 ***
 ***  permanent attributes:
 ***
 ***    int    USED       (number of times this code is used)
 ***
 ***  temporary attributes:
 ***
 ***    int    NO         (unambiguous number for PrintNwith2())
 ***                                   (precompile -> )
 ***    long*  MASK                    (optimize -> )
 ***    int    FLAG                    (WLI -> WLF)
 ***    node*  COPY                    ( -> DupTree )
 ***    ids*   DEC_RC_IDS              (refcount -> compile )
 ***
 ***  remarks:
 ***   1)
 ***   The CBLOCK 'plus' the CEXPR is the whole assignment block
 ***   to calculate each element of the WL. The CEXPR is the pseudo
 ***   return statement of the block.
 ***   In the flatten phase every node unequal N_id is flattened from
 ***   the CEXPR into the CBLOCK. After that we do not have to inspect the
 ***   CEXPR for every reason because we know that the *last let assignment*
 ***   in CBLOCK holds the return statement (CEXPR).
 ***
 ***   2)
 ***   The USED component is a reference counter for the NPART_CODE pointer.
 ***   MakeNPart increments it if the code parameter is != NULL,
 ***   FreeNPart decrements it if NPART_CODE is != NULL.
 ***   DupNpart  increments it (implicitly in MakeNPart, see condition above).
 ***
 ***/

extern node *MakeNCode (node *block, node *expr);

#define NCODE_CBLOCK(n) (n->node[0])
#define NCODE_CEXPR(n) (n->node[1])
#define NCODE_NEXT(n) (n->node[2])
#define NCODE_USED(n) (n->info.cint)

#define NCODE_MASK(n, x) (n->mask[x])
#define NCODE_NO(n) (n->refcnt)
#define NCODE_FLAG(n) (n->flag)
#define NCODE_DEC_RC_IDS(n) ((ids *)(n->node[3]))

#define NCODE_COPY(n) (n->node[4])

/*--------------------------------------------------------------------------*/

/***
 ***  N_Nwith2 :
 ***
 ***  sons:
 ***
 ***    node*      WITHID        (N_Nwithid)
 ***    node*      SEG           (N_WLseg)
 ***    node*      CODE          (N_Ncode)
 ***    node*      WITHOP        (N_Nwithop)
 ***
 ***    int        DIMS
 ***
 ***  temporary attributes:
 ***
 ***    int*       IDX_MIN            (wltransform -> compile )
 ***    int*       IDX_MAX            (wltransform -> compile )
 ***
 ***    ids*       DEC_RC_IDS         (wltransform -> compile )
 ***
 ***    DFMmask_t  IN                 (wltransform -> spmd -> )
 ***    DFMmask_t  INOUT              (wltransform -> spmd -> )
 ***    DFMmask_t  OUT                (wltransform -> spmd -> )
 ***    DFMmask_t  LOCAL              (wltransform -> spmd -> )
 ***
 ***/

extern node *MakeNWith2 (node *withid, node *seg, node *code, node *withop, int dims);

#define NWITH2_WITHID(n) (n->node[0])
#define NWITH2_SEGS(n) (n->node[1])
#define NWITH2_CODE(n) (n->node[2])
#define NWITH2_WITHOP(n) (n->node[3])
#define NWITH2_DIMS(n) (n->flag)

#define NWITH2_IDX_MIN(n) ((int *)(n->mask[2]))
#define NWITH2_IDX_MAX(n) ((int *)(n->mask[3]))

#define NWITH2_DEC_RC_IDS(n) ((ids *)(n->node[4]))

#define NWITH2_IN(n) ((DFMmask_t)n->dfmask[0])
#define NWITH2_INOUT(n) ((DFMmask_t)n->dfmask[1])
#define NWITH2_OUT(n) ((DFMmask_t)n->dfmask[2])
#define NWITH2_LOCAL(n) ((DFMmask_t)n->dfmask[3])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLseg :
 ***
 ***  sons:
 ***
 ***    node*    CONTENTS       (N_WLblock, N_WLublock, N_WLstride)
 ***    node*    NEXT           (N_WLseg)
 ***
 ***  permanent attributes:
 ***
 ***    int      DIMS      (number of dims)
 ***
 ***  temporary attributes:
 ***
 ***    int      BLOCKS    (number of blocking levels
 ***                         --- without unrolling-blocking)
 ***    long*    SV        (step vector)           (Precompile -> )
 ***    long*    BV        (blocking vector)       (Precompile -> )
 ***    long*    UBV       (unrolling-b. vector)   (Precompile -> )
 ***
 ***/

extern node *MakeWLseg (int dims, node *contents, node *next);

#define WLSEG_DIMS(n) (n->refcnt)
#define WLSEG_CONTENTS(n) (n->node[0])
#define WLSEG_NEXT(n) (n->node[1])

#define WLSEG_BLOCKS(n) (n->flag)

#define WLSEG_SV(n) (n->mask[0])
#define WLSEG_BV(n, level) (n->mask[level + 2])
#define WLSEG_UBV(n) (n->mask[1])

/*--------------------------------------------------------------------------*/

/*
 * here are some macros for N_WL... nodes:
 *
 * CAUTION: not every macro is suitable for all node tpyes.
 *          e.g. NEXTDIM is not a son of N_WLstride nodes
 *
 *          it would be better to contruct these macros like this:
 *            #define WLNODE_NEXTDIM(n) ((NODE_TYPE(n) == N_WLstride) ?
 *                                        DBUG_ASSERT(...) :
 *                                        (NODE_TYPE(n) == N_WLblock) ?
 *                                         WLBLOCK_NEXTDIM(n) :
 *                                         (NODE_TYPE(n) == N_WLublock) ?
 *                                          WLUBLOCK_NEXTDIM(n) : ...)
 *          but unfortunately this is not a modifiable l-value in ANSI-C :(
 *          so it would be impossible to use them on the left side of an
 *          assignment.
 *          because of that I designed this "static" macros to make a
 *          concise modelling of routines still possible.
 */

#define WLNODE_LEVEL(n) (n->lineno)
#define WLNODE_DIM(n) (n->refcnt)
#define WLNODE_BOUND1(n) (n->flag)
#define WLNODE_BOUND2(n) (n->counter)
#define WLNODE_STEP(n) (n->varno)
#define WLNODE_NEXTDIM(n) (n->node[0])
#define WLNODE_NEXT(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLblock :
 ***
 ***  sons:
 ***
 ***    node*    NEXTDIM       (N_WLblock)
 ***    node*    CONTENTS      (N_WLublock, N_WLstride)
 ***    node*    NEXT          (N_WLblock)
 ***
 ***  permanent attributes:
 ***
 ***    int      LEVEL                (number of blocking-levels so far)
 ***    int      DIM
 ***    int      BOUND1
 ***    int      BOUND2
 ***    int      STEP
 ***
 ***  temporary attributes:
 ***
 ***    ---
 ***
 ***
 ***  remarks:
 ***
 ***    it makes no sense to use the nodes NEXTDIM and CONTENTS simultaneous!
 ***
 ***/

extern node *MakeWLblock (int level, int dim, int bound1, int bound2, int step,
                          node *nextdim, node *contents, node *next);

#define WLBLOCK_LEVEL(n) (WLNODE_LEVEL (n))
#define WLBLOCK_DIM(n) (WLNODE_DIM (n))
#define WLBLOCK_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLBLOCK_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLBLOCK_STEP(n) (WLNODE_STEP (n))
#define WLBLOCK_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLBLOCK_CONTENTS(n) (n->node[2])
#define WLBLOCK_NEXT(n) (WLNODE_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLublock :
 ***
 ***  sons:
 ***
 ***    node*    NEXTDIM       (N_WLublock)
 ***    node*    CONTENTS      (N_WLstride)
 ***    node*    NEXT          (N_WLublock)
 ***
 ***  permanent attributes:
 ***
 ***    int      LEVEL
 ***    int      DIM
 ***    int      BOUND1
 ***    int      BOUND2
 ***    int      STEP
 ***
 ***  temporary attributes:
 ***
 ***    ---
 ***
 ***
 ***  remarks:
 ***
 ***    it makes no sense to use the nodes NEXTDIM and CONTENTS simultaneous!
 ***
 ***/

extern node *MakeWLublock (int level, int dim, int bound1, int bound2, int step,
                           node *nextdim, node *contents, node *next);

#define WLUBLOCK_LEVEL(n) (WLBLOCK_LEVEL (n))
#define WLUBLOCK_DIM(n) (WLBLOCK_DIM (n))
#define WLUBLOCK_BOUND1(n) (WLBLOCK_BOUND1 (n))
#define WLUBLOCK_BOUND2(n) (WLBLOCK_BOUND2 (n))
#define WLUBLOCK_STEP(n) (WLBLOCK_STEP (n))
#define WLUBLOCK_NEXTDIM(n) (WLBLOCK_NEXTDIM (n))
#define WLUBLOCK_CONTENTS(n) (WLBLOCK_CONTENTS (n))
#define WLUBLOCK_NEXT(n) (WLBLOCK_NEXT (n))

/*--------------------------------------------------------------------------*/

/***
 *** N_WLstride :
 ***
 ***  sons:
 ***
 ***    node*    CONTENTS     (N_WLgrid)
 ***    node*    NEXT         (N_WLstride)
 ***
 ***  permanent attributes:
 ***
 ***    int      LEVEL
 ***    int      DIM
 ***    int      BOUND1
 ***    int      BOUND2
 ***    int      STEP
 ***    int      UNROLLING                        (unrolling wanted?)
 ***
 ***  temporary attributes:
 ***
 ***    node*    PART            (Precompile ! )  (part this stride is ...
 ***                                                ... generated from)
 ***    int      MODIFIED        (Precompile ! )
 ***
 ***/

extern node *MakeWLstride (int level, int dim, int bound1, int bound2, int step,
                           int unrolling, node *contents, node *next);

#define WLSTRIDE_LEVEL(n) (WLNODE_LEVEL (n))
#define WLSTRIDE_DIM(n) (WLNODE_DIM (n))
#define WLSTRIDE_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLSTRIDE_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLSTRIDE_STEP(n) (WLNODE_STEP (n))
#define WLSTRIDE_UNROLLING(n) (n->info.prf_dec.tag)
#define WLSTRIDE_CONTENTS(n) (n->node[0])
#define WLSTRIDE_NEXT(n) (WLNODE_NEXT (n))

#define WLSTRIDE_PART(n) (n->node[5])
#define WLSTRIDE_MODIFIED(n) (n->info.prf_dec.tc)

/*--------------------------------------------------------------------------*/

/***
 *** N_WLgrid :
 ***
 ***  sons:
 ***
 ***    node*    NEXTDIM       (N_WLblock, N_WLublock, N_WLstride)
 ***    node*    NEXT          (N_WLgrid)
 ***
 ***  permanent attributes:
 ***
 ***    node*    CODE          (N_Ncode)
 ***    int      LEVEL
 ***    int      DIM
 ***    int      BOUND1
 ***    int      BOUND2
 ***    int      UNROLLING
 ***
 ***  temporary attributes:
 ***
 ***    int      MODIFIED           (Precompile ! )
 ***
 ***
 ***  remarks:
 ***
 ***    it makes no sense to use the nodes NEXTDIM and CODE simultaneous!
 ***
 ***/

extern node *MakeWLgrid (int level, int dim, int bound1, int bound2, int unrolling,
                         node *nextdim, node *next, node *code);

#define WLGRID_LEVEL(n) (WLNODE_LEVEL (n))
#define WLGRID_DIM(n) (WLNODE_DIM (n))
#define WLGRID_BOUND1(n) (WLNODE_BOUND1 (n))
#define WLGRID_BOUND2(n) (WLNODE_BOUND2 (n))
#define WLGRID_UNROLLING(n) (n->info.prf_dec.tag)
#define WLGRID_NEXTDIM(n) (WLNODE_NEXTDIM (n))
#define WLGRID_NEXT(n) (WLNODE_NEXT (n))
#define WLGRID_CODE(n) (n->node[2])

#define WLGRID_MODIFIED(n) (n->info.prf_dec.tc)

/*--------------------------------------------------------------------------*/

/***
 *** N_WLstriVar :
 ***
 ***  sons:
 ***
 ***    node*    CONTENTS      (N_WLgridVar)
 ***    node*    NEXT          (N_WLstriVar)
 ***
 ***  permanent attributes:
 ***
 ***    int      DIM
 ***    node*    BOUND1        (N_num, N_id)
 ***    node*    BOUND2        (N_num, N_id)
 ***    node*    STEP          (N_num, N_id)
 ***
 ***/

extern node *MakeWLstriVar (int dim, node *bound1, node *bound2, node *step,
                            node *contents, node *next);

#define WLSTRIVAR_DIM(n) (n->refcnt)
#define WLSTRIVAR_BOUND1(n) (n->node[2])
#define WLSTRIVAR_BOUND2(n) (n->node[3])
#define WLSTRIVAR_STEP(n) (n->node[4])
#define WLSTRIVAR_CONTENTS(n) (n->node[0])
#define WLSTRIVAR_NEXT(n) (n->node[1])

/*--------------------------------------------------------------------------*/

/***
 *** N_WLgridVar :
 ***
 ***  sons:
 ***
 ***    node*    NEXTDIM       (N_WLstriVar)
 ***    node*    NEXT          (N_WLgridVar)
 ***
 ***  permanent attributes:
 ***
 ***    node*    BOUND1        (N_num, N_id)
 ***    node*    BOUND2        (N_num, N_id)
 ***    node*    CODE          (N_Ncode)
 ***    int      DIM
 ***
 ***
 ***  remarks:
 ***
 ***    it makes no sense to use the nodes NEXTDIM and CODE simultaneous!
 ***
 ***/

extern node *MakeWLgridVar (int dim, node *bound1, node *bound2, node *nextdim,
                            node *next, node *code);

#define WLGRIDVAR_DIM(n) (n->refcnt)
#define WLGRIDVAR_BOUND1(n) (n->node[2])
#define WLGRIDVAR_BOUND2(n) (n->node[3])
#define WLGRIDVAR_NEXTDIM(n) (n->node[0])
#define WLGRIDVAR_NEXT(n) (n->node[1])
#define WLGRIDVAR_CODE(n) (n->node[4])

#endif /* _sac_tree_basic_h */
