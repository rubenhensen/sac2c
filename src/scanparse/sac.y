%{

/*
 *
 * $Log$
 * Revision 1.145  1998/02/09 16:06:28  srs
 * adjust Ngenidx to new MakeNWithid syntax
 *
 * Revision 1.144  1998/02/09 15:40:33  sbs
 * forced check in 8-(((
 * not yet cleaned up!
 *
 * Revision 1.143  1997/12/10 14:19:32  sbs
 * SOURCE-CODE-BRUSHING   STARTED !
 * the area brushed is marked by
 * BRUSH BEGIN and BRUSH END
 * started the elimination of GENTREE-DBUG-output
 * eliminated early node-creations for linenum preservation:
 * instead linenum is pushed by $<cint>$=linenum and
 * the result's NODE_LINE is updated accordingly!
 * the usage of Make... functions is forced!
 *
 * Revision 1.142  1997/11/25 12:22:27  sbs
 * if then else ala C
 * and a[1] now is recognized as a[[1]]!
 *
 * Revision 1.141  1997/11/24 18:06:40  sbs
 * new with loop syntax added AND (!!!!)
 * ALL SHIFT/REDUCE ERRORS ELIMINATED !!!!!!!!
 * Furthermore: all ifndef NEWTREE's eliminated
 * => has to linked into a NEWTREE only environment 8-))
 *
 * Revision 1.140  1997/11/10 14:22:46  dkr
 * removed a bug with A[.] -> psi(., A)
 *
 * Revision 1.139  1997/11/07 12:31:23  srs
 * changed another nnode part
 *
 * Revision 1.138  1997/11/04 11:25:18  srs
 * fixed little bug with NEWTREE
 *
 * Revision 1.137  1997/11/04 09:58:16  srs
 * NEWTREE: nnode is ignored
 *
 * Revision 1.136  1997/10/29 14:31:41  srs
 * free -> FREE
 *
 * Revision 1.135  1997/10/13 21:02:57  dkr
 * min() and max() are now foldops.
 *
 * Revision 1.134  1997/10/07 14:10:15  srs
 * new prfs PRF_MIN, PRF_MAX
 *
 * Revision 1.133  1997/10/03 17:57:17  dkr
 * added prf abs()
 *
 * Revision 1.131  1997/10/01 12:32:31  dkr
 * added % and %=
 *
 * Revision 1.128  1997/05/05 11:53:18  cg
 * SIB syntax slightly modified
 *
 * Revision 1.127  1997/05/05  07:46:09  cg
 * SIB limit removed
 *
 * Revision 1.126  1997/04/28  12:00:25  cg
 * SIB syntax slightly changed:
 * key word classtype used instead of Class.
 *
 * Revision 1.125  1997/04/25  13:22:19  sbs
 * sibarg: CHECK_NULL inserted
 *
 * Revision 1.124  1997/04/25  12:13:52  sbs
 * malloc replaced by Malloc
 *
 * Revision 1.123  1997/04/25  08:53:07  sbs
 * second arg of MakeType changed from NULL to 0
 *
 * Revision 1.122  1997/03/19  15:31:08  cg
 * Now, module/class implementations without any functions are supported
 *
 * Revision 1.121  1997/03/19  13:45:17  cg
 * new pragma linkwith added in module/class declarations as well as
 * special version for sib files
 *
 * Revision 1.120  1996/09/11  06:18:31  cg
 * Added a short cut notation for applications of modarray:
 * A[i]=t is equivalent to A=modarray(A,i,t)
 *
 * Revision 1.119  1996/09/09  10:06:47  cg
 * new primitive functions toi, tof, tod, etc. may now be overloaded
 * by library functions.
 *
 * Revision 1.118  1996/08/01  08:54:03  cg
 * Bug fixed: module name is now initialized when parsing N_ap nodes
 *
 * Revision 1.117  1996/04/02  19:37:26  cg
 * reimplemented handling of string constants
 *
 * Revision 1.116  1996/04/02  14:40:52  cg
 * bug fixed in parsing function applications
 *
 * Revision 1.115  1996/04/02  14:26:49  hw
 * changed DBUG_PRINT in rule "apl"
 *
 * Revision 1.114  1996/04/02  13:48:02  cg
 * scanparse converted to new strings:
 * primitive type string replaced by call of function to_string with
 * a char array as argument. to_string resides in module StringBase
 *
 * Revision 1.113  1996/02/21  15:05:12  cg
 * usage of function specifiers 'class' and 'inline' in SIBs corrected
 *
 * Revision 1.112  1996/02/21  09:17:42  cg
 * bug fixed in parsing SIBs: better use breaks in cases
 *
 * Revision 1.111  1996/02/08  18:05:46  hw
 * type-declaration with known dimenson, but unknown shape will be parsed
 * now e.g int[.,.]
 *
 * Revision 1.110  1996/01/26  15:29:57  cg
 * prefix 'class' introduced to functions to mark generic class conversion
 * functions
 *
 * Revision 1.109  1996/01/25  18:45:08  cg
 * added new class header containing definition of class type
 *
 * Revision 1.108  1996/01/23  09:57:03  cg
 * warnings about too long module names removed
 *
 * Revision 1.107  1996/01/22  17:30:38  cg
 * added new pragma initfun
 *
 * Revision 1.106  1996/01/21  13:57:30  cg
 * Macro GEN_NODE not longer used because it fails to initialize
 * the given structure
 *
 * Revision 1.105  1996/01/05  14:35:17  cg
 * The return statement is no longer referenced by the
 * corresponding genarray, modarray, foldfun, or foldprf node
 *
 * Revision 1.104  1996/01/05  12:33:43  cg
 * Now, we check that module/class names are not longer than
 * 12 characters.
 *
 * Revision 1.103  1996/01/02  17:46:48  cg
 * SIBS can now deal with external implicit types.
 *
 * Revision 1.102  1996/01/02  15:56:05  cg
 * The LINKMOD slot of fundef, typedef, and objdef nodes from module/class
 * declarations are now always set to the respective module/class name
 *
 * Revision 1.101  1995/12/29  10:38:14  cg
 * parsing of SIBs entirely re-implemented
 *
 * Revision 1.100  1995/12/21  15:04:43  cg
 * added primitive type char and all pragmas to parser
 *
 * Revision 1.99  1995/12/18  16:19:52  cg
 * added some DBUG_PRINTs
 *
 * Revision 1.98  1995/12/04  16:12:24  hw
 * - changed parsing of primitive function genarray
 * - added primitive functions toi, tod & tof
 *
 * Revision 1.97  1995/12/01  20:28:09  cg
 * Changed generic module name for SAC programs to "__MAIN"
 *
 * Revision 1.96  1995/12/01  17:16:53  cg
 * changed storage of alternative linkname.
 * new pragma node used for storage but syntax is still unmodified
 *
 * Revision 1.95  1995/11/02  09:19:20  sbs
 * old with-loop syntax converted into
 * with(...) {...} operator( .., returnval) !
 *
 * Revision 1.94  1995/11/01  09:51:31  sbs
 * both with-versions enabled!
 *
 * Revision 1.93  1995/11/01  07:06:36  sbs
 * Syntax of with-construct changed!
 * Instead of retassignblock now exprOrArray!
 * Old version left as comment
 *
 * Revision 1.92  1995/10/30  09:51:09  cg
 * Distinction between module and class implementations now exclusively
 * by attribute MODUL_FILETYPE.
 *
 * Revision 1.91  1995/10/22  17:35:52  cg
 * Now, key words like explicit types: or global objects:
 * are allowed in import statements as well as in export
 * declarations even if not followed by any item.
 *
 * Revision 1.90  1995/10/18  13:15:03  cg
 * converted to new error macros.
 *
 * Revision 1.89  1995/10/16  12:36:22  cg
 * added:
 * - global objects with module name in identifier positions
 * - module name in function applications
 *
 * Revision 1.88  1995/10/12  14:21:02  cg
 * module implementations with no functions will now be parsed
 *
 * Revision 1.87  1995/10/12  13:47:41  cg
 * bug in SIB-part fixed
 * now different name spaces for external and SAC-main-program items. The latter ones
 * are now prefixed "__SAC".
 *
 * Revision 1.86  1995/10/06  17:13:36  cg
 * calls to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.85  1995/10/01  17:00:26  cg
 * Function MakeLet renamed to MakeLetNode to avoid name clash with tree_basic.c
 * Minor changes in parsing SIBs.
 *
 * Revision 1.84  1995/09/27  15:21:51  cg
 * type charlist renamed to strings
 * new representation of file type in the syntax tree added.
 *
 * Revision 1.83  1995/08/21  13:24:25  cg
 * now external funs, objects and types can be imported implicitly
 *
 * Revision 1.82  1995/08/15  13:53:10  hw
 * primitive function genarray and modarray inserted
 *
 * Revision 1.81  1995/08/15  11:58:55  cg
 * sib-grammar extended for specification of implicit definitions of
 * implicit and/or unique types.
 *
 * Revision 1.80  1995/08/15  09:25:39  cg
 * varargs exclusively allowed in external declarations now.
 * parsing SIBs completed.
 *
 * Revision 1.79  1995/08/08  09:55:08  cg
 * Bug in DBUG_PRINT of function declaration fixed for parameterless functions.
 *
 * Revision 1.78  1995/08/07  10:30:38  cg
 * varargs added. Optional function names for external declarations added.
 *
 * Revision 1.77  1995/07/26  08:41:57  cg
 * parser extended for parsing sac information blocks
 *
 * Revision 1.76  1995/07/24  09:10:17  hw
 * moved 'enum' file_kind to tree.h
 *
 * Revision 1.75  1995/07/19  12:18:03  hw
 * in "extern" ModuleDec or ClassDec's the decaration of a primitive function
 * requires the declaration of the associated "non-SAC-function"
 * in braces after the name of the primitive function
 * ( e.g:  new_type + {my_plus} (new_type, new_type); )
 *
 * Revision 1.74  1995/07/17  14:03:16  hw
 * - the module name will be added to all type definitions
 * - functions can be declared in a module/class declaration
 *   with or without the name of the formal parameters
 *
 * Revision 1.73  1995/07/14  14:51:34  sbs
 * module_name inserted in fundec:
 *
 * Revision 1.72  1995/07/14  11:58:30  sbs
 * keyword inline moved to the beginning of function declaration
 *
 * Revision 1.71  1995/07/14  09:48:22  hw
 * changed rule fundec( now name of a function can be a primitive
 *  function name)
 *
 * Revision 1.70  1995/07/13  15:24:36  hw
 * the neutral element of a primitive function can be spezified
 * in the fold-part of a with-loop now
 *
 * Revision 1.69  1995/07/11  10:02:45  cg
 * bug in parsing typedefs fixed.
 *
 * Revision 1.68  1995/07/11  09:00:45  cg
 * most grammar version 0.6 features integrated.
 *
 * Revision 1.67  1995/07/06  17:27:05  cg
 * new Tokens ASSIGN and AMPERS added
 *
 * Revision 1.66  1995/07/06  17:01:25  hw
 * - behind 'all' in the import declaration has to be a semicolon now
 * - behind 'while' of a do-loop has to be a semicolon now
 * - userdefined functins can have the same name like primitive functions now
 *
 * Revision 1.65  1995/07/04  11:34:49  hw
 * - parsing of primitive functions itod, ftod, dtoi, dtof inserted
 * - parsing of 'double constants' inserted
 *
 * Revision 1.64  1995/06/30  11:27:21  hw
 * - new primitive functions "ftoi" & "itof" inserted
 * - token C_KEYWORD for not used c-keywords inserted
 *
 * Revision 1.63  1995/06/09  10:01:34  cg
 * inline warning deleted.
 *
 * Revision 1.62  1995/05/31  13:03:11  hw
 * node[1] of N_foldfun is not an N_exprs anymore
 *
 * Revision 1.61  1995/05/31  11:38:23  hw
 * changed rule for 'fold' (case N_foldfun) ( exchanged 'expr' with 'exprORarray' )
 *
 * Revision 1.60  1995/05/30  08:22:28  cg
 * sac_grammar version 0.5 tokens inserted.
 *
 * Revision 1.59  1995/05/30  07:17:18  hw
 * - N_foldfun has now two child nodes (node[0]: body of with-loop
 *    node[1]: expr to compute neutral element of userdefined function
 *             that is used in fold
 * - name of userdefined function that is used in 'fold' is stored
 *   in info.fun_name.id of N_foldfun node.
 *   the modul name is stored in info.fun_name.id_mod , if any
 *
 * Revision 1.58  1995/05/23  07:49:23  hw
 * bug fixed in creation of "psi" (A[...] => psi(...,A) )
 *
 * Revision 1.57  1995/05/22  14:25:45  hw
 * - bug fixed in creation of "psi" (A[7] will be transformed to psi([7],A),
 *    NOTE: A[b] will be transformed to psi(b,A) !!!!!! )
 * - set arg_node->flag to 1 if function should be inlined
 *
 * Revision 1.56  1995/04/26  16:06:15  hw
 * bug fixed in MakeLet ( identifier will be copied now)
 *
 * Revision 1.55  1995/04/06  13:56:09  hw
 * typeinformation of sac_main is build with MakeTypes now
 *
 * Revision 1.54  1995/03/13  16:59:59  hw
 * -  the identifier of node N_id, N_pre and N_post is stored
 *    in 'info.ids->id' instead of 'info.id' now
 *
 * Revision 1.53  1995/03/07  19:13:04  hw
 *  - new rule for parsing of arrays ( no recursion anymore)
 *  - changed rule for parsing of 'A[..]'
 *   (now 'A[1,2,3] == A[[1,2,3]] == psi([1,2,3],A)' and
 *        'A[x] == psi(x,A)' )
 *
 * Revision 1.52  1995/03/01  16:28:00  hw
 * changed N_generator ( the name of the index vector is
 * 	now stored in info.ids)
 *
 * Revision 1.51  1995/02/20  09:35:47  hw
 * inserted rule expr -> MINUS ID %prec UMINUS
 *
 * Revision 1.50  1995/02/14  12:42:42  sbs
 * New Prf's allowed in fold( prf)!
 *
 * Revision 1.49  1995/02/14  12:00:42  sbs
 * fold inserted
 *
 * Revision 1.48  1995/02/14  10:11:33  hw
 * primitive function "not" inserted
 *
 * Revision 1.47  1995/02/13  16:41:21  sbs
 * bug in DBUG-PRINT of N_prf psi fixed.
 *
 * Revision 1.46  1995/02/09  09:01:56  hw
 * bug fixed in creation of cat, rotate & psi
 *
 * Revision 1.45  1995/02/02  14:57:46  hw
 * changed N_prf node
 *
 * Revision 1.44  1995/01/25  16:01:45  sbs
 * Priority of cast changed to %right CAST
 *
 * Revision 1.43  1995/01/09  12:16:07  sbs
 * bug fixed: mod_name set earlier in moddec.
 *
 * Revision 1.42  1995/01/06  19:05:51  sbs
 * no_mod_ext pragma deleted & extern declaration inserted
 *
 * Revision 1.41  1995/01/06  17:51:45  sbs
 * no_mod_ext pragma inserted
 *
 * Revision 1.40  1995/01/05  12:46:51  sbs
 * string inserted
 *
 * Revision 1.39  1995/01/03  17:57:00  hw
 * changed constructing of assigns to handle converted for-loops correctly
 * inserted DBUG_PRINT's in function Append.
 * bug fixed in DBUG_PRINT of rule 'forassign' part FOR ...
 *
 * Revision 1.38  1994/12/31  15:11:27  sbs
 * changed the structure of N_explist nodes:
 * node[0] contains now a typedef chain for implicit types!
 * 2) modul names inserted for export-decls
 *
 * Revision 1.37  1994/12/31  14:10:39  sbs
 * modul selection inserted for types and function applications
 *
 * Revision 1.36  1994/12/21  13:15:37  sbs
 * for fundec nnode decremented (1/0)
 *
 * Revision 1.35  1994/12/21  11:45:16  hw
 * added DBUG_PRINT in simpletype
 * changed N_typedef : now the name of the defined type is put in info.types->id
 *
 * Revision 1.34  1994/12/20  17:43:40  hw
 * deleted function MakeNode (now in tree.c)
 * deleted nodes N_addon, etc. they are converted to N_let
 *
 * Revision 1.33  1994/12/20  13:10:47  sbs
 * typedef bug fixed (types->id => types->name)
 *
 * Revision 1.32  1994/12/20  11:24:29  sbs
 * decl_tree inserted
 *
 * Revision 1.31  1994/12/16  14:34:28  sbs
 * moddec  and start_token inserted
 *
 * Revision 1.30  1994/12/15  17:29:16  sbs
 * bug in fundef2 inline fixed
 *
 * Revision 1.29  1994/12/15  17:09:16  sbs
 * typecasts as (: type) inserted.
 * The colon is due the context dependencies resulting from
 * expressions like ( ID ) [ expr ]
 * or ( ID ) + expr
 * or ( ID ) - expr
 * In later versions this has to be changed to a context sensitive
 * version which distinguishes between ID and TYPEID.....:-<
 *
 * Revision 1.28  1994/12/14  16:52:46  sbs
 * user defined types integrated :->>>>
 *
 * Revision 1.27  1994/12/14  13:57:26  sbs
 * bug in expressionblock2 fixed (superflouus SEMIC BRACE_R)
 *
 * Revision 1.26  1994/12/14  12:42:08  hw
 * error fixed in rule 'fundef2'
 *
 * Revision 1.25  1994/12/14  10:09:04  sbs
 * modul, imports and typedefs inserted;
 * typedeclarations with user defined types
 * are not yet possible:-(
 *
 * Revision 1.24  1994/12/09  15:19:14  hw
 * added rule expr -> expr EQ expr  (this one was missing :-(  )
 *
 * Revision 1.23  1994/12/08  17:46:57  sbs
 * modules inserted
 *
 * Revision 1.22  1994/12/08  14:15:04  hw
 * changed node[1] and node[2] of N_fundef
 * now node[1] points to the next function and
 *
 * node[2] points to the formal parameters
 * added rule to parse functions without parameters
 *
 * Revision 1.21  1994/12/07  17:37:15  sbs
 * err1 fixed: multiple vardecs
 *
 * Revision 1.20  1994/11/29  10:56:38  hw
 * added assignment to ids->node in rule "ids"
 *
 * Revision 1.19  1994/11/28  13:26:02  hw
 * error in "exprblock" fixed
 *
 * Revision 1.18  1994/11/23  09:45:00  hw
 * added rule: expr -> TRUE ; expr -> FALSE
 *
 * Revision 1.17  1994/11/22  14:22:53  hw
 * error in declaration of arrays without shape fixed
 *
 * Revision 1.16  1994/11/22  13:42:58  hw
 * - error fixed in Append
 * - changed rule expr -> ID [ exprs ]  into expr -> expr [ expr ]
 *
 * Revision 1.15  1994/11/22  11:41:25  hw
 * - declaration of arrays without shape is possible now
 * - changed exprblock assignblock retassignblock, because of changes in assigns
 *
 * Revision 1.14  1994/11/21  17:59:42  hw
 * inserted linenumber to each node
 *
 * Revision 1.13  1994/11/18  16:55:52  hw
 * error in assignblock fixed
 *
 * Revision 1.12  1994/11/18  14:54:56  hw
 * changed function Append
 * added function MakeEmptyBlock
 *
 * Revision 1.11  1994/11/17  16:53:57  hw
 * bug fixed in Append
 * all N_return nodes are children of N_assign nodes now.
 *
 * Revision 1.10  1994/11/15  16:55:53  hw
 * changed assignblock rule
 * converted for loop to while loop
 * => eliminated N_for
 *
 * Revision 1.9  1994/11/11  13:48:31  hw
 * added prefix & postfix increment-& decrementation
 * added new nodes: N_pre N_post N_inc N_dec
 *
 * Revision 1.8  1994/11/10  17:31:54  sbs
 * error in modarray fixed
 *
 * Revision 1.7  1994/11/10  17:30:44  sbs
 * Expr inserted in modarray + genarray
 *
 * Revision 1.6  1994/11/10  15:19:42  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "dbug.h"
#include "tree.h"
#include "typecheck.h"
#include "my_debug.h"
#include "internal_lib.h" /* for use of StringCopy */
#include "Error.h"
#include "free.h"

#include "readsib.h"


extern int linenum;
extern char yytext[];

int indent, i;

node *syntax_tree;
node *decl_tree;
node *sib_tree;


static char *mod_name="__MAIN";
static char *link_mod_name=NULL;
static node *store_pragma=NULL;


/* used to distinguish the different kinds of files  */
/* which are parsed with this single parser          */

static file_type file_kind = F_prog;


%}

%union {
         nodetype        nodetype;
         id              *id;
         ids             *ids;
         types           *types;
         node            *node;
         int             cint;
         char            cchar;
         float           cfloat;
         double          cdbl;
         nums            *nums;
         deps            *deps;
         prf             prf;
         statustype      statustype;
         strings         *strings;
       }

%token PARSE_PRG, PARSE_DEC, PARSE_SIB
%token BRACE_L, BRACE_R, BRACKET_L, BRACKET_R, SQBR_L, SQBR_R, COLON, SEMIC,
       COMMA, AMPERS, ASSIGN, DOT,
       INLINE, LET, TYPEDEF, CONSTDEF, OBJDEF, CLASSTYPE,
       INC, DEC, ADDON, SUBON, MULON, DIVON, MODON
       K_MAIN, RETURN, IF, ELSE, DO, WHILE, FOR, WITH, NWITH, FOLD,
       MODDEC, MODIMP, CLASSDEC, IMPORT, ALL, IMPLICIT, EXPLICIT, TYPES, FUNS,
       OWN, CONSTANTS, GLOBAL, OBJECTS, CLASSIMP,
       ARRAY,SC, TRUE, FALSE, EXTERN, C_KEYWORD,
       PRAGMA, LINKNAME, LINKSIGN, EFFECT, READONLY, REFCOUNTING,
       TOUCH, COPYFUN, FREEFUN, INITFUN, LINKWITH,
       STEP, WIDTH
%token <id> ID, STR, AND, OR, EQ, NEQ, NOT, LE, LT, GE, GT, MUL, DIV, PRF_MOD, PLUS,
            F2I, F2D, I2F,I2D, D2I, D2F,
            TOI, TOF, TOD, 
            MINUS, PRIVATEID, ABS, PRF_MIN, PRF_MAX
            RESHAPE, SHAPE, TAKE, DROP, DIM, ROTATE, CAT, PSI, GENARRAY, MODARRAY
%token <types> TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_UNS, TYPE_SHORT,
               TYPE_LONG, TYPE_CHAR, TYPE_DBL, TYPE_VOID, TYPE_DOTS
%token <cint> NUM
%token <cfloat> FLOAT
%token <cdbl> DOUBLE
%token <cchar> CHAR

%type <prf> foldop, Ngenop, monop, binop, triop
%type <nodetype> modclass
%type <cint> evextern, sibheader, sibevmarker, dots
%type <ids> ids, modnames, modname
%type <deps> linkwith, linklist, siblinkwith, siblinklist, sibsublinklist
%type <id> fun_name, prf_name, sibparam, id
%type <nums> nums
%type <statustype> sibreference, sibevclass, siblinkliststatus
%type <types> simpletype, simpletype_main, 
              localtype, localtype_main, local_type_udt_arr,
              type, typeNOudt_arr,
              types, returntypes, varreturntypes, vartypes;
%type <node> arg, args, fundefs, fundef, main, prg, modimp, module, class, 
             argtypes, argtype, varargs, varargtypes,
             fundec2, fundec3, pragmas,
             typedefs, typedef, defs, def2, def3, def4, fundef2,
             objdefs, objdef, exprblock, exprblock2,
             assign, assigns, assignblock, letassign, 
             selassign, forassign, assignsOPTret, optassignblock, optelse,
             exprsNOar, exprNOdot, exprORdot, exprNOar, exprNOnum,
             expr, expr_main, expr_ap, expr_ar, expr_num, exprs,
             Ngenerator, Nsteps, Nwidth, Nwithop, Ngenidx,
             conexpr, generator, unaryop,
             moddec, expdesc, expdesc2, expdesc3, expdesc4, fundecs, fundec,
             exptypes, exptype, objdecs, objdec, evimport, modheader,
             imptypes, imptype,import, imports, impdesc, impdesc2, impdesc3,
             impdesc4, foldfun,
             sib, sibtypes, sibtype, sibfuns, sibfun, sibfunbody,
             sibobjs, sibobj, sibpragmas, sibarglist,
             sibargs, sibarg, sibfunlist, sibfunlistentry

%left OR
%left AND
%left EQ, NEQ
%left LE, LT, GE, GT
%left PLUS, MINUS
%left MUL, DIV, PRF_MOD
%left TAKE, DROP, RESHAPE
%left SQBR_L
%right CAST
%right NOT
%nonassoc UMINUS
%nonassoc GENERATOR
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%start file

%%

file:   PARSE_PRG prg {syntax_tree=$2;}
      | PARSE_PRG modimp {syntax_tree=$2;}
      | PARSE_DEC moddec {decl_tree=$2;}
      | PARSE_SIB sib {sib_tree=$2;}

                 
  
	;

id: ID
      {
        $$=$1;
      }
  | PRIVATEID
      {
        if (file_kind!=F_sib)
        {
          ABORT(linenum, ("Identifier name '%s` illegal", $1));
        }
        else
        {
          $$=$1;
        }
      }
  ;


/*
 *********************************************************************
 *
 *  rules for module/class declarations
 *
 *********************************************************************
 */



moddec: modheader evimport OWN COLON expdesc
        {
          $$=$1;
          $$->node[0]=$5;
          $$->node[1]=$2;
          if($$->node[1] != NULL)
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[1]->nodetype ], $$->node[1]));
          }
          else
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
          }
	     }
      | modheader evimport
        {
          $$=$1;
          $$->node[0]=MakeNode(N_explist);
          $$->node[1]=$2;
          if($$->node[1] != NULL)
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[1]->nodetype ], $$->node[1]));
          }
          else
          {
             DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.fun_name.id,
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
          }
	}
      ;

modheader: modclass evextern id COLON linkwith
           {
             $$=MakeNode($1);
             link_mod_name=$3;

            if ($2)
             {
               mod_name=NULL;
             }
             else
             {
               mod_name=link_mod_name;
             }

             MODDEC_NAME($$)=$3;
             MODDEC_LINKWITH($$)=$5;
             MODDEC_ISEXTERNAL($$)=$2;
           }
         ;                 

modclass: MODDEC {$$=N_moddec; file_kind=F_moddec;}
	     | CLASSDEC {$$=N_classdec; file_kind=F_classdec;}
	     ;

evextern: EXTERN 
          {
            file_kind++;
            $$=1;
          }
	|
          {
            $$=0;
          }
	;

linkwith: PRAGMA LINKWITH linklist {$$=$3;}
        | {$$=NULL;}
        ;

linklist: STR COMMA linklist 
           {
             $$=MakeDeps($1, NULL, NULL, ST_system, NULL, $3);
           }
       | STR
           {
             $$=MakeDeps($1, NULL, NULL, ST_system, NULL, NULL);
           }
       ;

evimport: imports {$$=$1;}
	  | {$$=NULL;}
	  ;

imports: import imports { $$=$1;
                          $$->node[0]=$2;
                        }

	| import { $$=$1; }
	;

import: IMPORT id COLON impdesc { $$=$4; $$->info.id=$2; }
	;

impdesc: ALL SEMIC
            { $$=MakeNode( N_implist );

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
	| BRACE_L IMPLICIT TYPES COLON ids SEMIC impdesc2
            { $$=$7;
              $$->node[1]=(node *)$5; /* dirty trick for keeping ids !*/
            }
	| BRACE_L IMPLICIT TYPES COLON SEMIC impdesc2
            { $$=$6;
            }
	| BRACE_L impdesc2 { $$=$2; }
	;

impdesc2: EXPLICIT TYPES COLON ids SEMIC impdesc3
            { $$=$6;
              $$->node[2]=(node *)$4; /* dirty trick for keeping ids !*/
            }
        | EXPLICIT TYPES COLON SEMIC impdesc3
            { $$=$5;
            }
	     | impdesc3 { $$=$1; }
	     ;

impdesc3: GLOBAL OBJECTS COLON ids SEMIC impdesc4
            { $$=$6;
              $$->node[4]=(node *)$4; /* dirty trick for keeping ids !*/
            }
        | GLOBAL OBJECTS COLON SEMIC impdesc4
            { $$=$5;
            }
	     | impdesc4 { $$=$1; }
	;

impdesc4: FUNS COLON ids SEMIC BRACE_R
            { $$=MakeNode( N_implist );
              $$->node[3]=(node *)$3; /* dirty trick for keeping ids !*/

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
        | FUNS COLON SEMIC BRACE_R
            { $$=MakeNode( N_implist );

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
	     | BRACE_R
            { $$=MakeNode( N_implist );

              DBUG_PRINT("GENTREE",
                         ("%s:"P_FORMAT,mdb_nodetype[ $$->nodetype ], $$));
            }
	;

expdesc: BRACE_L IMPLICIT TYPES COLON imptypes expdesc2
         { $$=$6;
           $$->node[0]=$5;
         }
        | BRACE_L expdesc2 
           {$$=$2;}
        | BRACE_L IMPLICIT TYPES COLON expdesc2
           { $$=$5;}
        ;

expdesc2: EXPLICIT TYPES COLON exptypes expdesc3
          { $$=$5;
            $$->node[1]=$4;
          }
        | EXPLICIT TYPES COLON expdesc3
          { $$=$4;
          }
        | expdesc3 {$$=$1;}
        ;

expdesc3: GLOBAL OBJECTS COLON objdecs expdesc4
          { $$=$5;
            $$->node[3]=$4;
          }
        | GLOBAL OBJECTS COLON expdesc4
          { $$=$4;
          }
        | expdesc4 {$$=$1;}
        ;

expdesc4: FUNS COLON fundecs BRACE_R
          { $$=MakeNode(N_explist);
            $$->node[2]=$3;

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        | FUNS COLON BRACE_R
          { $$=MakeNode(N_explist);

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        | BRACE_R
          { $$=MakeNode(N_explist);
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$));
          }
        ;

imptypes: imptype imptypes {$$=$1;
                            $1->node[0]=$2;
                           }
	| imptype {$$=$1;}
	;

imptype: id SEMIC pragmas
              {
              $$=MakeNode(N_typedef);
              $$->info.types=MakeTypes(T_hidden);

              DBUG_PRINT("GENTREE",("type:"P_FORMAT" %s",
                                    $$->info.types,
                                    mdb_type[$$->info.types->simpletype]));

              $$->info.types->id=$1;
              $$->info.types->id_mod=mod_name;
              $$->info.types->id_cmod=link_mod_name;
              $$->info.types->status=ST_imported;
              TYPEDEF_PRAGMA($$)=$3;
              
              DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$,
                        $$->info.types, $$->info.types->id));

             }
             ;

exptypes: exptype exptypes {$$=$1;
                            $1->node[0]=$2;
                           }
	| exptype {$$=$1;}
	;

exptype: id LET type SEMIC pragmas
           { $$=MakeNode(N_typedef);
            $$->info.types=$3;
            $$->info.types->id=$1;
            $$->info.types->id_mod=mod_name;
            $$->info.types->id_cmod=link_mod_name;
            $$->info.types->status=ST_imported;
            TYPEDEF_PRAGMA($$)=$5;

            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
         ;

objdecs: objdec objdecs    {$$=$1;
                            $$->node[0]=$2;
                           }
	| objdec {$$=$1;}
	;

objdec: type id SEMIC pragmas
           { $$=MakeNode(N_objdef);
             $$->info.types=$1;
             $$->info.types->id=$2;              /* object name */
             $$->info.types->id_mod=mod_name;    /* SAC module name */
             $$->info.types->id_cmod=link_mod_name; /* external module name */
             $$->info.types->status=ST_imported;
             $$->node[1]=NULL;	/* there is no object init here! */
             OBJDEF_PRAGMA($$)=$4;
             
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
         ;

fundecs: fundec fundecs 
           {
             $$=$1;
             $$->node[1]=$2;
           }
       | fundec {$$=$1;}
       ;


fundec: varreturntypes id BRACKET_L fundec2
          {
            $$=$4;
            $$->info.types=$1;
            $$->info.types->id=$2; /* function name */
            $$->info.types->id_mod=mod_name;    /* SAC modul name */
            $$->info.types->id_cmod=link_mod_name; /* external modul name */
            $$->info.types->status=ST_imported;
            
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s , NULL body,  %s" P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id,
                        mdb_nodetype[ ($$->node[2]==NULL)
                                      ?T_void:($$->node[2]->nodetype) ],
                        $$->node[2]));
          }         
      | returntypes prf_name BRACKET_L fundec3
          { 
/*
            if(!((F_moddec == file_kind) || (F_classdec == file_kind)))
             {
                strcpy(yytext,"(");
                yyerror("syntax error");
             }
            else
             {
*/
                $$=$4;
                $$->info.types=$1;
                $$->info.types->id=$2;              /* function name */
                $$->info.types->id_mod=mod_name;    /* SAC module name */
                $$->info.types->id_cmod=link_mod_name; 
                                              /* external module name */
                $$->info.types->status=ST_imported;
                
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" Id: %s"P_FORMAT", NULL body,  %s"
                        P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, $$->info.types->id,
                        $$->info.types->id,
                        mdb_nodetype[ ($$->node[2]==NULL)
                                      ?T_void:($$->node[2]->nodetype) ],
                        $$->node[2]));

/*
             }
*/
             
          }
        ;

fundec2 : varargtypes BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | varargs BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | TYPE_DOTS BRACKET_R SEMIC pragmas
           {
              if((F_extmoddec != file_kind) && (F_extclassdec != file_kind))
              {
                strcpy(yytext,"...");
                yyerror("syntax error");
              }
              else
              {
                $$=MakeNode(N_fundef);
                $$->node[2]=MakeNode(N_arg);
                $$->node[2]->info.types=MakeTypes(T_dots);
                FUNDEF_PRAGMA($$)=$4;
              }
            }
        | BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              FUNDEF_PRAGMA($$)=$3;
            }
        ;

fundec3 : argtypes BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | args BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              $$->node[2]=$1;        /* argument declarations */
              FUNDEF_PRAGMA($$)=$4;
            }
        | BRACKET_R SEMIC pragmas
            {
              $$=MakeNode(N_fundef);
              FUNDEF_PRAGMA($$)=$3;
            }
        ;

pragmas: pragmalist
         {
           $$=store_pragma;
           store_pragma=NULL;
         }
       |
         {
           $$=NULL;
         }
       ;

pragmalist: pragmalist pragma
          | pragma
          ;

pragma: PRAGMA LINKNAME STR
        { 
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_LINKNAME(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'linkname`"))
          PRAGMA_LINKNAME(store_pragma)=$3;
        }
      | PRAGMA LINKSIGN SQBR_L nums SQBR_R
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_LINKSIGNNUMS(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'linksign`"))
         PRAGMA_LINKSIGNNUMS(store_pragma)=$4;
        }
      | PRAGMA REFCOUNTING SQBR_L nums SQBR_R
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_REFCOUNTINGNUMS(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'refcounting`"))
          PRAGMA_REFCOUNTINGNUMS(store_pragma)=$4;
        }
      | PRAGMA READONLY SQBR_L nums SQBR_R
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_READONLYNUMS(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'readonly`"))
          PRAGMA_READONLYNUMS(store_pragma)=$4;
        }
      | PRAGMA EFFECT modnames
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_EFFECT(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'effect`"))
          PRAGMA_EFFECT(store_pragma)=$3;
        }
      | PRAGMA TOUCH modnames
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_TOUCH(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'touch`"))
          PRAGMA_TOUCH(store_pragma)=$3;
        }
      | PRAGMA COPYFUN STR
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_COPYFUN(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'copyfun`"))
          PRAGMA_COPYFUN(store_pragma)=$3;
        }
      | PRAGMA FREEFUN STR
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_FREEFUN(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'freefun`"))
          PRAGMA_FREEFUN(store_pragma)=$3;
        }
      | PRAGMA INITFUN STR
        {
          if (store_pragma==NULL) store_pragma=MakePragma();
          if (PRAGMA_INITFUN(store_pragma)!=NULL)
            WARN(linenum, ("Conflicting definitions of pragma 'initfun`"))
          PRAGMA_INITFUN(store_pragma)=$3;
        }
      ;

modnames: modname COMMA modnames
          {
            $$=$1;
            IDS_NEXT($$)=$3;
          }
        | modname
          {
            $$=$1;
          }
        ;

modname: id
         {
           $$=MakeIds($1, NULL, ST_regular);
         }
       | id COLON id
         {
           $$=MakeIds($3, $1, ST_regular);
         }
       ;




/*
 *********************************************************************
 *
 *  rules for type, object, and function definitions
 *
 *********************************************************************
 */


defs: imports def2 {$$=$2;
                    $$->node[0]=$1;
                   }
	| def2 {$$=$1;}

def2: typedefs def3 {$$=$2;
                     $$->node[1]=$1;
                    }
	|def3 {$$=$1;}

def3: objdefs def4 { $$=$2;
                     $$->node[3]=$1;
                   }
      | def4 { $$=$1;}
      ;

def4: fundefs { $$=MakeNode(N_modul);
                $$->info.id=NULL;
                $$->node[2]=$1;

                DBUG_PRINT("GENTREE",
                           ("%s:"P_FORMAT"  %s"P_FORMAT,
                            mdb_nodetype[ $$->nodetype ], $$, 
                            mdb_nodetype[ $$->node[2]->nodetype ],
                            $$->node[2]));
              }
    |         { $$=MakeNode(N_modul);  /* module impl with no functions */
                $$->info.id=NULL;
                $$->node[2]=NULL;

                DBUG_PRINT("GENTREE",
                           ("%s:"P_FORMAT,
                            mdb_nodetype[ $$->nodetype ], $$));
                
              }
	;

modimp: module
        {
          $$=$1;
        }
      | class
        {
          $$=$1;
        }
      ;

module: MODIMP { file_kind=F_modimp;} id {  mod_name=$3; } COLON defs
          {
            $$=$6;
            MODUL_NAME($$)=mod_name;
            MODUL_FILETYPE($$)=file_kind;
          }
	;

class: CLASSIMP { file_kind=F_classimp;} id {  mod_name=$3; } COLON 
       CLASSTYPE type SEMIC defs
          { 
            $$=$9;
            MODUL_NAME($$)=mod_name;
            MODUL_FILETYPE($$)=file_kind;
            MODUL_CLASSTYPE($$)=$7;
          }
	;

prg: defs { $$=$1;
            MODUL_NAME($$)=mod_name;
            MODUL_FILETYPE($$)=F_prog;
          }
        ;

typedefs: typedef typedefs {$$=$1;
                            $1->node[0]=$2;
                           }
	| typedef {$$=$1; }
	;

typedef: TYPEDEF type id SEMIC 
          { $$=MakeNode(N_typedef);
            $$->info.types=$2;
            $$->info.types->id=$3;
            $$->info.types->id_mod=mod_name;
            /*
            if ((file_kind==F_classimp) && (!strcmp((char*) $3, mod_name)))
               $$->info.types->attrib=ST_unique;
            */
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }

objdefs: objdef objdefs {$$=$1;
                         $$->node[0]=$2;
                           }
	| objdef {$$=$1; }
	;

objdef: OBJDEF type id LET expr SEMIC 
          { $$=MakeNode(N_objdef);
            $$->info.types=$2;
            $$->info.types->id=$3;
            $$->info.types->id_mod=mod_name;
            
            $$->node[1]=$5;
            
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        $$->info.types, $$->info.types->id));
          }
      ;



fundefs: fundef fundefs { $$=$1;
                          $$->node[1]=$2;
                        }
	| fundef {$$=$1;}
	| main {$$=$1;}
	;

fundef: returntypes fun_name BRACKET_L fundef2 
        {  $$=$4;
           $$->info.types=$1;          		/*  result type(s) */
           $$->info.types->id=$2;      		/*  function name  */
           $$->info.types->id_mod=mod_name;     /*  module name    */
           $$->flag=0;                          /*  inline flag    */

           DBUG_PRINT("GENTREE",
                      ("%s: %s:%s "P_FORMAT,
                       mdb_nodetype[ $$->nodetype ],
                       $$->info.types->id_mod,
                       $$->info.types->id,
                       $$->info.types->id));

        }
      | INLINE returntypes fun_name BRACKET_L fundef2 
        {  $$=$5;
           $$->info.types=$2;          		/*  result type(s) */
           $$->info.types->id=$3;      		/*  function name  */
           $$->info.types->id_mod=mod_name;     /*  module name    */
           $$->flag=1;                          /*  inline flag    */

           DBUG_PRINT("GENTREE",
                      ("%s: %s:%s "P_FORMAT,
                       mdb_nodetype[ $$->nodetype ],
                       $$->info.types->id_mod,
                       $$->info.types->id,
                       $$->info.types->id));

        }
      ;

fundef2: args BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>3;
            $$->node[0]=$4;             /* Funktionsrumpf  */
            $$->node[2]=$1;             /* Funktionsargumente */
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT", Id: %s"P_FORMAT" %s," P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, 
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0],
                        mdb_nodetype[ $$->node[2]->nodetype ], $$->node[2]));
          }

       | BRACKET_R  {$$=MakeNode(N_fundef);}   exprblock
          { 
            $$=$<node>2;
            $$->node[0]=$3;             /* Funktionsrumpf  */
         
            DBUG_PRINT("GENTREE",
                       ("%s:"P_FORMAT" %s"P_FORMAT,
                        mdb_nodetype[ $$->nodetype ], $$, 
                        mdb_nodetype[ $$->node[0]->nodetype ], $$->node[0]));
         }
        ;


args:   arg COMMA args  {$1->node[0]=$3;
                         $$=$1;
                        }
      | arg {$$=$1;}
      ;

varargs: arg COMMA varargs
         {
           $1->node[0]=$3;
           $$=$1;
                           }
       | arg COMMA TYPE_DOTS
           {
             if((F_extmoddec != file_kind) && (F_extclassdec != file_kind))
             {
               strcpy(yytext,"...");
               yyerror("syntax error");
             }
             else
             {
               $$=$1;
               $$->node[0]=MakeNode(N_arg);
               $$->node[0]->info.types=MakeTypes(T_dots);
               
              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Id: ..., Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));
             } 
           }
       | arg
         {
           $$=$1;
         }
      ;

arg: type id {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->id=$2;     /* Argumentname */
              $$->info.types->next=NULL; /* keine weiteren Argumente */

              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Id: %s, Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->id, $$->info.types->attrib));
             }
     |
     type AMPERS id {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->id=$3;     /* Argumentname */
              $$->info.types->next=NULL; /* keine weiteren Argumente */
              $$->info.types->attrib=ST_reference;
                     
              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Id: %s, Attrib: %d ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->id, $$->info.types->attrib));
             }
     ;

argtypes: argtype COMMA argtypes
          {
            $1->node[0]=$3;
            $$=$1;
          }
        | argtype
          {
            $$=$1;
          }
      ;

varargtypes: argtype COMMA varargtypes
             {
               $1->node[0]=$3;
               $$=$1;
             }
       | argtype COMMA TYPE_DOTS
           {
             if((F_extmoddec != file_kind) && (F_extclassdec != file_kind))
             {
               strcpy(yytext,"...");
               yyerror("syntax error");
             }
             else
             {
               $$=$1;
               $$->node[0]=MakeNode(N_arg);
               $$->node[0]->info.types=MakeTypes(T_dots);

              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));

             }
           }
      | argtype {$$=$1;}
      ;

argtype: type {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->next=NULL; /* keine weiteren Argumente */

              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Attrib: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));
             }
     |
     type AMPERS {$$=MakeNode(N_arg); 
              $$->info.types=$1;         /* Argumenttyp */
              $$->info.types->next=NULL; /* keine weiteren Argumente */
              $$->info.types->attrib=ST_reference;
                     
              DBUG_PRINT("GENTREE",
                         ("%s: "P_FORMAT", Attrib: %d ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          $$->info.types->attrib));
             }
     ;

fun_name : id { $$=$1; }
         | prf_name { $$=$1; }
         ;

prf_name : AND { $$=$1; }
         | OR { $$=$1; }
         | EQ { $$=$1; }
         | NEQ { $$=$1; }
         | NOT { $$=$1; }
         | LE { $$=$1; }
         | LT { $$=$1; }
         | GE { $$=$1; }
         | GT { $$=$1; }
         | ABS { $$=$1; }
         | PLUS { $$=$1; }
         | MINUS { $$=$1; }
         | DIV { $$=$1; }
         | MUL { $$=$1; }
         | PRF_MOD { $$=$1; }
         | RESHAPE { $$=$1; }
         | SHAPE { $$=$1; }
         | TAKE { $$=$1; }
         | DROP { $$=$1; }
         | DIM { $$=$1; }
         | ROTATE { $$=$1; }
         | CAT { $$=$1; }
         | PSI { $$=$1; }
         | GENARRAY { $$=$1; }
         | MODARRAY { $$=$1; }
         | TOI { $$=$1; }
         | TOF { $$=$1; }
         | TOD { $$=$1; }
         | I2F { $$=$1; }
         | I2D { $$=$1; }
         | F2I { $$=$1; }
         | F2D { $$=$1; }
         | D2I { $$=$1; }
         | D2F { $$=$1; }
         | PRF_MIN { $$=$1; }
         | PRF_MAX { $$=$1; }
        ;

main: TYPE_INT K_MAIN BRACKET_L BRACKET_R {$$=MakeNode(N_fundef);} exprblock 
       {
        $$=$<node>5;     /* $$=$5 */
        $$->node[0]=$6;                 /* Funktionsrumpf */

        $$->info.types=MakeTypes(T_int);  /* Knoten fu"r Typinformation */ 
        $$->info.types->id=(char *)Malloc(sizeof(char)*5); 
        strcpy($$->info.types->id, "main");   /* Funktionsnamen eintragen */

        DBUG_PRINT("GENTREE",("%s:"P_FORMAT", main "P_FORMAT
                              "  %s (" P_FORMAT ") ",
                              mdb_nodetype[$$->nodetype], $$, 
                              $$->info.types->id,
                              mdb_nodetype[ $$->node[0]->nodetype], 
                              $$->node[0]));
        
       }
      ;


/* BRUSH BEGIN */


/*
 *********************************************************************
 *
 *  rules for expression blocks
 *
 *********************************************************************
 */


exprblock: BRACE_L exprblock2 { $$=$2; }
         ;

exprblock2: typeNOudt_arr ids SEMIC exprblock2 
            { node *vardec_ptr;
              ids   *ids_ptr=$2;
 
              /*
               * Insert the actual vardec(s) before the ones that
               * are already attached to the N_block node of $4!
               * This reverses the order of var-decs!
               * The reason for doing so is feasablilty only...
               * In regard to semantics, there should be no difference...
               */
              vardec_ptr=BLOCK_VARDEC( $4);

              DBUG_ASSERT( ($2 != NULL),
                           "non-terminal ids should not return NULL ptr!");
                                               
              /*
               * In the AST, each variable has it's own declaration.
               * Therefore, for each ID in ids, we have to generate 
               * it's own N_vardec node with it's own copy of the
               * types-structure from $1!
               */
              while( IDS_NEXT($2) != NULL) { /* at least 2 vardecs! */
                vardec_ptr=MakeVardec(
                             IDS_NAME($2),
                               DuplicateTypes( $1, 1),
                                 vardec_ptr);
                /* 
                 * Now, we want to "push" $2 one IDS further
                 * and we want to FREE the current IDS structure.
                 * Since we have recycled the IDS_ID, we can NOT
                 * use a given function from free.c such as 'FreeOneIds'
                 * or 'FreeAllIds'.
                 * Therefore, we introduce a temporary ids_ptr, which holds
                 * the ptr to the "next $2" and manually free the current $2.
                 */
                ids_ptr=IDS_NEXT( $2);
                FREE( $2);
                $2=ids_ptr;
              }
              /*
               * When we reach this point, all but one vardec is constructed!
               * Therefore, we can recycle the types node from $1 instead of
               * duplicating it as done in the loop above!
               */
              $$=$4;
              BLOCK_VARDEC($$)=MakeVardec(
                                 IDS_NAME($2),
                                   $1,
                                     vardec_ptr);
              FREE( $2); /* Finally, we free the last IDS-node! */

            }
          | assignsOPTret BRACE_R { $$=MakeBlock( $1, NULL); }
          ;


optassignblock: assignblock {$$ = $1;}
              | {$$ = NULL;}
              ;

assignblock: SEMIC     
             { $$=MAKE_EMPTY_BLOCK( );
             }
           | BRACE_L {$<cint>$=linenum;} assigns BRACE_R 
             { if($3==NULL) {
                 $$=MAKE_EMPTY_BLOCK();
               }
               else {
                 $$=MakeBlock( $3, NULL);
                 NODE_LINE($$)=$<cint>2;
               }
             }
           | assign  
             { $$=MakeBlock( MakeAssign( $1, NULL), NULL);
             } 
           ;


assignsOPTret: /*
                * Although this rule is very similar to the "assigns"
                * rule we keep both of them since this allows the 
                * N_assign -> N_return     node to be
                * inserted directly in the N_assign chain.
                * Otherwise, we would have to append that node
                * to a given N_assigns chain as generated by 
                * "assigns".
                */
               /* empty */      
               { $$=MakeAssign( MakeReturn(NULL), NULL);
               }
             | RETURN BRACKET_L {$<cint>$=linenum;} exprs BRACKET_R SEMIC
               { $$=MakeAssign( MakeReturn( $4), NULL);
                 NODE_LINE($$)=$<cint>3;
               }
             | assign assignsOPTret
               { if( NODE_TYPE($1)==N_assign){
                   /*
                    * The only situation where "assign" returns an N_assign
                    * node is when a for-loop had to be parsed.
                    * In this case we get a tree of the form:
                    *
                    * N_assign -> N_let
                    *    \
                    *   N_assign -> N_while
                    *      \
                    *     NULL
                    */
                   DBUG_ASSERT( (NODE_TYPE( ASSIGN_NEXT($1))==N_while)
                                 && (ASSIGN_NEXT( ASSIGN_NEXT($1))==NULL),
                                "corrupted node returned for \"assign\"!");
                   $$=$1;
                   ASSIGN_NEXT( ASSIGN_NEXT($$))=$2;
                 }
                 else {
                   $$=MakeAssign( $1, $2);
                 }
               }
             ;

assigns: /* empty */  { $$=NULL; }
       | assign assigns 
         { if( NODE_TYPE($1)==N_assign){
             /*
              * The only situation where "assign" returns an N_assign
              * node is when a for-loop had to be parsed.
              * In this case we get a tree of the form:
              *
              * N_assign -> N_let
              *    \
              *   N_assign -> N_while
              *      \
              *     NULL
              */
             DBUG_ASSERT( (NODE_TYPE( ASSIGN_NEXT($1))==N_while)
                           && (ASSIGN_NEXT( ASSIGN_NEXT($1))==NULL),
                          "corrupted node returned for \"assign\"!");
             $$=$1;
             ASSIGN_NEXT( ASSIGN_NEXT($$))=$2;
           }
           else {
             $$=MakeAssign( $1, $2);
           }
         }
       ;

assign: letassign SEMIC { $$=$1; }
      | selassign       { $$=$1; }
      | forassign       { $$=$1; }
      ;

letassign: ids LET expr 
           { $$=MakeLet($3, $1);
           }
         | id SQBR_L exprNOnum SQBR_R LET expr
           { $$=MakeLet( MakePrf( F_modarray,
                           MakeExprs( MakeId( $1, NULL, ST_regular) ,
                             MakeExprs( $3,
                               MakeExprs($6,
                                 NULL))) ),
                         MakeIds($1, NULL, ST_regular) );
           }
         | expr_ap 
           { $$=MakeLet( $1, NULL);
           }

/* left for later BRUSHING BEGIN */
         | id unaryop 
            { $$=MakeNode(N_post);
              $$->info.ids=MakeIds($1, NULL, ST_regular);
              $$->node[0]=$2;
              
              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT,
                          mdb_nodetype[$$->nodetype], $$, $$->info.ids->id,
                          mdb_nodetype[$$->node[0]->nodetype] ));
           }
         | unaryop id
            {  $$=MakeNode(N_pre);
               $$->info.ids=MakeIds($2, NULL, ST_regular);    
               $$->node[0]=$1;

               DBUG_PRINT("GENTREE",
                          ("%s "P_FORMAT": %s "P_FORMAT,
                           mdb_nodetype[$$->nodetype], $$, $$->info.ids->id,
                           mdb_nodetype[$$->node[0]->nodetype] )); 
            }
/* left for later BRUSHING END */

         | id ADDON expr { $$=MAKE_OPON_LET($1,$3,F_add); }
         | id SUBON expr { $$=MAKE_OPON_LET($1,$3,F_sub); }
         | id MULON expr { $$=MAKE_OPON_LET($1,$3,F_mul); }
         | id DIVON expr { $$=MAKE_OPON_LET($1,$3,F_div); }
         | id MODON expr { $$=MAKE_OPON_LET($1,$3,F_mod); }
         ;

selassign: IF {$<cint>$=linenum;} BRACKET_L exprNOar BRACKET_R assignblock 
           optelse
           { $$=MakeCond( $4, $6, $7);
             NODE_LINE($$)=$<cint>2;
           }
         ;

optelse: ELSE assignblock                  { $$ = $2;                  }
       | /* empty*/  %prec LOWER_THAN_ELSE { $$ = MAKE_EMPTY_BLOCK( ); } 
       ;

forassign: DO {$<cint>$=linenum;} assignblock
           WHILE BRACKET_L exprNOar BRACKET_R SEMIC
           { $$=MakeDo( $6, $3);
             NODE_LINE($$)=$<cint>2;
           }
         | WHILE {$<cint>$=linenum;} BRACKET_L exprNOar BRACKET_R assignblock 
           { $$=MakeWhile( $4, $6);
             NODE_LINE($$)=$<cint>2;
           }
         | FOR {$<cint>$=linenum;} BRACKET_L assign exprNOar SEMIC 
           letassign BRACKET_R assignblock
           { /*
              * for( x=e1; e2; y=e3) AssBlock
              * is transformed into
              * x=e1;
              * while( e2) { AssBlock; y=e3;}
              */
             $$=$4; 
             ASSIGN_NEXT($$)=MakeAssign( MakeWhile( $5, Append($9,$7)), NULL);
             NODE_LINE( ASSIGN_INSTR( ASSIGN_NEXT($$)))=$<cint>2;
           } 
         ;

exprs: expr COMMA exprs { $$=MakeExprs($1, $3);   }
     | expr             { $$=MakeExprs($1, NULL); }
     ;

exprsNOar: exprNOar COMMA exprsNOar { $$=MakeExprs($1, $3);   }
         | exprNOar                 { $$=MakeExprs($1, NULL); }
         ;

exprNOdot: expr %prec GENERATOR { $$ = $1; }
         ;

exprORdot: expr %prec GENERATOR { $$ = $1;   }
         | DOT                  { $$ = NULL; }
         ;

exprNOar: expr_main {$$ = $1;}
        | expr_ap   {$$ = $1;}
        | expr_num  {$$ = $1;}
        ;
 
exprNOnum: expr_main {$$ = $1;}
         | expr_ar   {$$ = $1;}
         | expr_ap   {$$ = $1;}
         ;
 
expr: expr_main {$$ = $1;}
    | expr_ar   {$$ = $1;}
    | expr_ap   {$$ = $1;}
    | expr_num  {$$ = $1;}
    ;
 
expr_num: NUM {$$=MakeNum( yylval.cint);}
        ;

expr_ar: SQBR_L {$<cint>$=linenum;} exprsNOar SQBR_R
         { $$=MakeArray( $3);
           NODE_LINE($$)=$<cint>2;
         }
       ;

expr_ap: id BRACKET_L {$<cint>$=linenum;} exprs BRACKET_R
         { $$=MakeAp( $1, NULL, $4);
           NODE_LINE($$)=$<cint>3;
         }
       | id BRACKET_L BRACKET_R
         { $$=MakeAp($1, NULL, NULL);
         }
       | id COLON id  BRACKET_L {$<cint>$=linenum;} exprs BRACKET_R
          { $$=MakeAp( $3, $1, $6);
            NODE_LINE($$)=$<cint>5;
          }
       | id COLON id BRACKET_L BRACKET_R
         { $$=MakeAp($3, $1, NULL);
         }
       ;


expr_main: id  { $$=MakeId( $1, NULL, ST_regular); }
         | id COLON id  { $$=MakeId( $3, $1, ST_regular); }
         | MINUS id %prec UMINUS
           { /*
              * substitute unary minus by multiplication
              * with -1!
              */
             $$=MAKE_BIN_PRF( F_mul, 
                                MakeNum( -1),
                                  MakeId($2, NULL, ST_regular) );
           }
         | BRACKET_L COLON type BRACKET_R expr %prec CAST
           { $$=MakeCast( $5, $3);
           }
         | NOT expr
           { MakePrf( F_not,
               MakeExprs( $2,
                 NULL));
           }
         | BRACKET_L expr BRACKET_R { $$=$2; }
         | expr SQBR_L exprNOnum SQBR_R
           { $$ = MAKE_BIN_PRF( F_psi, $3, $1);
             NODE_LINE($$) = NODE_LINE( $1);
           }
         | expr SQBR_L expr_num SQBR_R
           { $$ = MakePrf( F_psi,
                    MakeExprs( MakeArray( MakeExprs( $3, NULL)),
                      MakeExprs( $1,
                        NULL)) );
             NODE_LINE($$) = NODE_LINE( $1);
           }
         | expr SQBR_L expr COMMA exprs SQBR_R
           { $$ = MakePrf( F_psi,
                    MakeExprs( MakeArray( MakeExprs( $3, $5)), 
                      MakeExprs( $1,
                        NULL)) );
             NODE_LINE($$) = NODE_LINE( $1);
           }
         | monop BRACKET_L expr BRACKET_R 
           { $$=MakePrf( $1,
                  MakeExprs( $3,
                    NULL));
           }
         | binop BRACKET_L expr COMMA expr BRACKET_R 
           { $$=MAKE_BIN_PRF( $1, $3, $5);
           }
         | triop BRACKET_L expr COMMA expr COMMA expr BRACKET_R 
           { $$=MakePrf( $1, 
                  MakeExprs( $3,
                    MakeExprs( $5,
                      MakeExprs( $7,
                        NULL))));
           }
         | expr AND expr     { $$=MAKE_BIN_PRF( F_and,$1,$3); }
         | expr OR expr      { $$=MAKE_BIN_PRF( F_or ,$1,$3); }
         | expr EQ expr      { $$=MAKE_BIN_PRF( F_eq ,$1,$3); }
         | expr NEQ expr     { $$=MAKE_BIN_PRF( F_neq,$1,$3); }
         | expr LE expr      { $$=MAKE_BIN_PRF( F_le ,$1,$3); }
         | expr LT expr      { $$=MAKE_BIN_PRF( F_lt ,$1,$3); }
         | expr GE expr      { $$=MAKE_BIN_PRF( F_ge ,$1,$3); }
         | expr GT expr      { $$=MAKE_BIN_PRF( F_gt ,$1,$3); }
         | expr PLUS expr    { $$=MAKE_BIN_PRF( F_add,$1,$3); }
         | expr MINUS expr   { $$=MAKE_BIN_PRF( F_sub,$1,$3); }
         | expr DIV expr     { $$=MAKE_BIN_PRF( F_div,$1,$3); }
         | expr MUL expr     { $$=MAKE_BIN_PRF( F_mul,$1,$3); }
         | expr PRF_MOD expr { $$=MAKE_BIN_PRF( F_mod,$1,$3); }
         | MINUS NUM %prec UMINUS    { $$=MakeNum( -yylval.cint);  }
         | PLUS  NUM %prec UMINUS    { $$=MakeNum( yylval.cint);   }
         | CHAR                      { $$=MakeChar(yylval.cchar);  }
         | FLOAT                     { $$=MakeFloat( $1);          }
         | MINUS FLOAT %prec UMINUS  { $$=MakeFloat( -$2);         }
         | PLUS FLOAT %prec UMINUS   { $$=MakeFloat( $2);          }
         | DOUBLE                    { $$=MakeDouble( $1);         }
         | MINUS DOUBLE %prec UMINUS { $$=MakeDouble( -$2);        }
         | PLUS DOUBLE %prec UMINUS  { $$=MakeDouble( $2);         }
         | TRUE                      { $$=MakeBool( 1);            }
         | FALSE                     { $$=MakeBool( 0);            }
         | STR                       { $$=string2array2string($1); }
         | NWITH {$<cint>$=linenum;} BRACKET_L Ngenerator BRACKET_R
           optassignblock Nwithop 
           { /*
              * the tricky part about this rule is that $7 (an N_Nwithop node)
              * carries the goal-expression of the With-Loop, i.e., the "N-expr"
              * node which belongs into the N_Ncode node!!!
              * The reason for this is that an exclusion of the goal expression
              * from the non-terminal Nwithop would lead to a shift/reduce
              * conflict in that rule!
              */
             $$=MakeNWith( $4, MakeNCode( $6, NWITHOP_EXPR($7)), $7);
             NODE_LINE($$)= $<cint>2;

             /*
              * Finally, we generate the connection between the 
              * (only) partition and the (only) code!
              */
             NPART_CODE( NWITH_PART($$)) = NWITH_CODE($$);
           }
         | WITH {$<cint>$=linenum;} BRACKET_L generator  BRACKET_R conexpr 
           { $$=MakeWith( $4, $6);
           }
         ;


Ngenerator: exprORdot Ngenop Ngenidx Ngenop exprORdot
            Nsteps Nwidth
            { $$=MakeNPart( $3, 
                   MakeNGenerator($1, $5, $2,  $4, $6, $7 ));
            }
          ;

Nsteps: /* empty */ { $$ = NULL; }
      | STEP expr   { $$ = $2;}
      ;

Nwidth: /* empty */ { $$ = NULL; }
      | WIDTH expr  { $$ = $2;}
      ;

Ngenidx: id { $$ = MakeNWithid(MakeIds($1, NULL, ST_regular), NULL); }
       | SQBR_L ids SQBR_R { $$ = MakeNWithid( NULL, $2); }
       ;

Ngenop:   LT {$$=F_lt;}
	| LE {$$=F_le;}
        ;

Nwithop: GENARRAY BRACKET_L expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_genarray);
           NWITHOP_SHAPE($$) = $3;
           NWITHOP_EXPR($$) = $5;
         }
       | MODARRAY BRACKET_L expr COMMA id COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_modarray);
           NWITHOP_ARRAY($$) = $3;
           NWITHOP_EXPR($$) = $7;
         }
       | FOLD BRACKET_L foldop COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldprf);
           NWITHOP_PRF($$) = $3;
           NWITHOP_EXPR($$) = $5;
         }
       | FOLD BRACKET_L foldop COMMA expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldprf);
           NWITHOP_PRF($$) = $3;
           NWITHOP_NEUTRAL($$) = $5;
           NWITHOP_EXPR($$) = $7;
         }
       | FOLD BRACKET_L id COMMA expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldfun);
           NWITHOP_FUN($$) = $3;
           NWITHOP_NEUTRAL($$) = $5;
           NWITHOP_EXPR($$) = $7;
         }
       | FOLD BRACKET_L id COLON id COMMA expr COMMA expr BRACKET_R
         { $$ = MakeNWithOp( WO_foldfun);
           NWITHOP_FUN($$) = $5;
           NWITHOP_MOD($$) = $3;
           NWITHOP_NEUTRAL($$) = $7;
           NWITHOP_EXPR($$) = $9;
         }
       ;

/* NOT BRUSHED BEGIN */

generator: exprNOdot  LE id LE exprNOdot
            { $$=MakeNode(N_generator);
              $$->node[0]=$1;        /* left border  */
              $$->node[1]=$5;        /* right border */
              $$->info.ids=MakeIds($3, NULL, ST_regular); /*index-variable  */

              DBUG_PRINT("GENTREE",
                         ("%s "P_FORMAT": %s "P_FORMAT", ID: %s, %s "P_FORMAT ,
                          mdb_nodetype[$$->nodetype], $$,
                          mdb_nodetype[$$->node[0]->nodetype], $$->node[0],
                          $$->info.ids->id,
                          mdb_nodetype[$$->node[1]->nodetype], $$->node[1]));
            }
        ;

conexpr: assignblock GENARRAY BRACKET_L expr COMMA {$$=MakeGenarray($4, NULL);}
         expr BRACKET_R
           { node *ret;

             $$=$<node>6;
             ret = MakeReturn( MakeExprs($7, NULL) );
             RETURN_INWITH(ret)=1;
             
             GENARRAY_BODY($$)=Append( $1, ret);
             
           }
         | GENARRAY BRACKET_L expr COMMA {$$=MakeGenarray($3, NULL);}
           expr BRACKET_R
           { node *ret;

             ret=MakeReturn( MakeExprs($6, NULL) );
             RETURN_INWITH(ret)=1;
             
             $$=$<node>5;
             GENARRAY_BODY($$)=MakeBlock(
                                 MakeAssign( ret, NULL),
                                 NULL );

           }


         | assignblock MODARRAY BRACKET_L expr COMMA id COMMA
           {$$=MakeModarray($4, NULL);} expr BRACKET_R
           { node *ret;

             $$=$<node>8;
             ret = MakeReturn( MakeExprs($9, NULL) );
             RETURN_INWITH(ret)=1;

             MODARRAY_BODY($$)=Append( $1, ret);
             MODARRAY_ID($$)=$6;
             
           }
         | MODARRAY BRACKET_L expr COMMA id COMMA {$$=MakeModarray($3, NULL);}
           expr BRACKET_R
           { node *ret;

             $$=$<node>7;
             ret=MakeReturn( MakeExprs($8, NULL) );
             RETURN_INWITH(ret)=1;
             
             MODARRAY_BODY($$)=MakeBlock(
                                 MakeAssign( ret, NULL),
                                 NULL );
             MODARRAY_ID($$)=$5;
           }

         | assignblock FOLD BRACKET_L foldop COMMA expr BRACKET_R
           { node *ret;

             $$=MakeFoldprf($4, NULL, NULL);
             ret = MakeReturn( MakeExprs($6, NULL) );
             RETURN_INWITH(ret)=1;
             FOLDPRF_BODY($$)=Append( $1, ret);

           }
         | FOLD BRACKET_L foldop COMMA expr BRACKET_R
           { node *ret;

             ret=MakeReturn( MakeExprs($5, NULL) );
             RETURN_INWITH(ret)=1;
             
             $$=MakeFoldprf($3,
                            MakeBlock( MakeAssign( ret, NULL),
                                       NULL ),
                            NULL);

           }

         | assignblock FOLD BRACKET_L foldop COMMA expr COMMA expr
           BRACKET_R
           { node *ret;

             $$=MakeFoldprf($4,NULL,$6);
             ret = MakeReturn( MakeExprs($8, NULL) );
             RETURN_INWITH(ret)=1;
             FOLDPRF_BODY($$)=Append($1, ret);
           }
         | FOLD BRACKET_L foldop COMMA expr COMMA expr BRACKET_R
           { node *ret;

             ret=MakeReturn( MakeExprs($7, NULL) );
             RETURN_INWITH(ret)=1;
             
             $$=MakeFoldprf($3,
                            MakeBlock( MakeAssign( ret, NULL),
                                       NULL ),
                            $5);

           }

         | assignblock FOLD BRACKET_L foldfun expr COMMA expr 
           BRACKET_R
           { node *ret;

              $$=$<node>4;
              ret = MakeReturn( MakeExprs($7, NULL) );
              RETURN_INWITH(ret)=1;
              FOLDFUN_BODY($$)=Append( $1, ret);
              FOLDFUN_NEUTRAL($$)= $5;

           }
         | FOLD BRACKET_L foldfun expr COMMA expr BRACKET_R
           { node *ret;

             $$=$<node>3;
             ret=MakeReturn( MakeExprs($6, NULL) );
             RETURN_INWITH(ret)=1;
             
             FOLDFUN_BODY($$)= MakeBlock( MakeAssign( ret, NULL),
                                       NULL );
             FOLDFUN_NEUTRAL($$)= $4;

           }

         ;


foldfun:  id COMMA 
     {
        $$=MakeNode(N_foldfun);
        $$->info.fun_name.id=$1;
     }
    | id COLON id COMMA
     {
        $$=MakeNode(N_foldfun);
        $$->info.fun_name.id=$3;
        $$->info.fun_name.id_mod=$1;
     }
    ;
     
/* NOT BRUSHED END  */
 
foldop: PLUS    {$$=F_add;}
      | MINUS   {$$=F_sub;}
      | DIV     {$$=F_div;}
      | MUL     {$$=F_mul;}
      | PRF_MOD {$$=F_mod;}
      | PRF_MIN {$$=F_min;}
      | PRF_MAX {$$=F_max;}
      | AND     {$$=F_and;}
      | OR      {$$=F_or; }
      | EQ      {$$=F_eq; }
      | NEQ     {$$=F_neq;}
      ;

monop: ABS   { $$=F_abs;   }
     | DIM   { $$=F_dim;   }
     | SHAPE { $$=F_shape; }
     | TOI   { $$=F_toi;   }
     | TOF   { $$=F_tof;   }
     | TOD   { $$=F_tod;   }
     | F2I   { $$=F_ftoi;  }
     | F2D   { $$=F_ftod;  }
     | I2F   { $$=F_itof;  }
     | I2D   { $$=F_itod;  }
     | D2I   { $$=F_dtoi;  }
     | D2F   { $$=F_dtof;  }
     ;

/* left for later BRUSHING BEGIN */

unaryop: INC
          { $$=MakeNode(N_inc);
          }

   
       | DEC
          { $$=MakeNode(N_dec);
          }
           ;

/* left for later BRUSHING END */

binop: PSI      { $$=F_psi;      }
     | TAKE     { $$=F_take;     }
     | DROP     { $$=F_drop;     }
     | RESHAPE  { $$=F_reshape;  }
     | GENARRAY { $$=F_genarray; }
     | PRF_MIN  { $$=F_min;      }
     | PRF_MAX  { $$=F_max;      }
     ;


triop: ROTATE   { $$=F_rotate;   }
     | CAT      { $$=F_cat;      }
     | MODARRAY { $$=F_modarray; }
     ;

ids: id COMMA ids 
     { $$=MakeIds($1, NULL, ST_regular);
       IDS_NEXT($$)=$3;
     }
   | id 
     { $$=MakeIds($1, NULL, ST_regular);
     }
   ;

nums: NUM COMMA nums { $$=MakeNums($1, $3);   }
    | NUM            { $$=MakeNums($1, NULL); }
    ; 

dots: DOT COMMA dots { $$=$3-1;                }
    | DOT            { $$=KNOWN_DIM_OFFSET -1; }
    ;

returntypes: TYPE_VOID { $$=MakeTypes(T_void); }
           | types     { $$=$1;                }
           ;

types: type COMMA types
       { $$=$1;
         TYPES_NEXT($$)=$3; 
       }
     | type { $$=$1; }
     ;

varreturntypes: TYPE_VOID { $$=MakeTypes(T_void); }
              | vartypes  { $$=$1;                }
              ;

vartypes: type COMMA vartypes 
          { $$=$1;
            TYPES_NEXT($$)=$3;
          }
        | type { $$=$1; }
        | TYPE_DOTS
          { if ((F_extmoddec != file_kind) 
                   && (F_extclassdec != file_kind)
                     && (F_sib != file_kind)) {
              strcpy(yytext,"...");
              yyerror("syntax error");
            }
            else {
              $$=MakeTypes(T_dots);
            }
          }
        ;

typeNOudt_arr: localtype_main {$$ = $1;}
             | id COLON localtype
               { $$=$3;
                 TYPES_MOD($$)=$1;
               }
             ;

type: localtype { $$=$1; }
      | id COLON localtype
        { $$=$3;
          TYPES_MOD($$)=$1;
        }
      ;

localtype: localtype_main     {$$=$1;}
         | local_type_udt_arr {$$=$1;}
         ;

localtype_main: simpletype {$$ = $1; }
              | simpletype_main SQBR_L nums SQBR_R  
                { $$=GenComplexType($1,$3); }
              | simpletype_main  SQBR_L SQBR_R  
                { $$ = $1;
                  TYPES_DIM($$) = -1;
                }
              | simpletype_main SQBR_L dots SQBR_R
                { $$ = $1;
                  TYPES_DIM($$) = $3;
                }
              | id SQBR_L SQBR_R
                { $$=MakeTypes(T_user);
                  TYPES_NAME($$) = $1;
                  TYPES_DIM($$)  = -1;
                }
              | id SQBR_L dots SQBR_R
                { $$=MakeTypes(T_user);
                  TYPES_NAME($$) = $1;
                  TYPES_DIM($$)  = $3;
                }
             ;

local_type_udt_arr: id SQBR_L nums SQBR_R
                    { $$=MakeTypes(T_user);
                      TYPES_NAME($$) = $1;
                      $$ = GenComplexType( $$, $3);
                    }
                  ;

simpletype: simpletype_main { $$ = $1; }
          | id 
            { $$=MakeTypes(T_user);
              TYPES_NAME($$) = $1;
            }
          ;

simpletype_main: TYPE_INT   { $$=MakeTypes(T_int);    }
               | TYPE_FLOAT { $$=MakeTypes(T_float);  }
               | TYPE_BOOL  { $$=MakeTypes(T_bool);   }
               | TYPE_CHAR  { $$=MakeTypes(T_char);   }
               | TYPE_DBL   { $$=MakeTypes(T_double); }
               ;

/*       BRUSH END    */


/*
 *********************************************************************
 *
 *  rules for SAC Information Blocks (SIB)
 *
 *********************************************************************
 */


sib: sibheader siblinkwith sibtypes
       {
         $$ = $3;
         SIB_LINKSTYLE($$) = $1;
         SIB_LINKWITH($$)  = $2;

         DBUG_PRINT("GENSIB",("%s"P_FORMAT,
                             mdb_nodetype[$$->nodetype],$$));
       }
   ;

sibheader: LT id GT
             {
               mod_name=$2;
               file_kind=F_sib;
               $$=0;
             }
         | LT id COLON ALL GT
             {
               mod_name=$2;
               file_kind=F_sib;
               $$=1;
             }
         ;

siblinkwith: LINKWITH siblinklist {$$=$2;}
             | {$$=NULL;}
	          ;

siblinklist: siblinkliststatus STR sibsublinklist COMMA siblinklist
             {
               $$=MakeDeps($2, NULL, NULL, $1, $3, $5);
             }
           | siblinkliststatus STR sibsublinklist 
             {
               $$=MakeDeps($2, NULL, NULL, $1, $3, NULL);
             }
           ;

siblinkliststatus: EXTERN 
             {
               $$=ST_external;
             }
	        | LINKWITH
             {
               $$=ST_system;
             }
	        |
             {
               $$=ST_sac;
             }
	        ;

sibsublinklist: BRACE_L siblinklist BRACE_R
             {
               $$=$2;
             }
           | 
             {
               $$=NULL;
             }
           ;


sibtypes: sibtype sibtypes
            {
              $$=$2;
              TYPEDEF_NEXT($1)=SIB_TYPES($2);
              SIB_TYPES($$) = $1;
            }
        | sibobjs
            {
              $$=$1;
            } 
          ;

sibtype: sibevclass TYPEDEF type id SEMIC sibpragmas
           {
             $$=MakeTypedef($4, NULL, $3, $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$6;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
       | sibevclass TYPEDEF type id COLON id SEMIC sibpragmas
           {
             $$=MakeTypedef($6, $4, $3, $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$8;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
       | sibevclass TYPEDEF IMPLICIT id SEMIC sibpragmas
           {
             $$=MakeTypedef($4, NULL,
                            MakeType(T_hidden, 0, NULL, NULL, NULL),
                            $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$6;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
       | sibevclass TYPEDEF IMPLICIT id COLON id SEMIC sibpragmas
           {
             $$=MakeTypedef($6, $4,
                            MakeType(T_hidden, 0, NULL, NULL, NULL),
                            $1, NULL);
             TYPEDEF_STATUS($$)=ST_imported;
             TYPEDEF_PRAGMA($$)=$8;

            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        TYPEDEF_TYPE($$), ItemName($$)));
           }
        ;

sibevclass: CLASSTYPE {$$=ST_unique;}
          |           {$$=ST_regular;}

sibobjs: sibobj sibobjs
            {
              $$=$2;
              OBJDEF_NEXT($1)=SIB_OBJS($2);
              SIB_OBJS($2)=$1;
            }
        | sibfuns
            {
              $$=$1;
            } 
          ;

sibobj: OBJDEF type id SEMIC sibpragmas
          {
            $$=MakeObjdef($3, NULL, $2, NULL, NULL);
            OBJDEF_PRAGMA($$)=$5;
            OBJDEF_STATUS($$)=ST_imported;
            
            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        OBJDEF_TYPE($$), ItemName($$)));
          }
      | OBJDEF type id COLON id SEMIC sibpragmas
          {
            $$=MakeObjdef($5, $3, $2, NULL, NULL);
            OBJDEF_PRAGMA($$)=$7;
            OBJDEF_STATUS($$)=ST_imported;
            
            DBUG_PRINT("GENSIB",
                       ("%s:"P_FORMAT","P_FORMAT", Id: class %s",
                        mdb_nodetype[ $$->nodetype ], $$, 
                        OBJDEF_TYPE($$), ItemName($$)));
          }
       ;

sibfuns: sibfun sibfuns
            {
              $$ = $2;
              FUNDEF_NEXT($1)=SIB_FUNS($2);
              SIB_FUNS($$) = $1;
            }
       | 
            {
              $$ = MakeSib( mod_name, 0, NULL, NULL, NULL, NULL);
            }
       ;

sibfun: sibevmarker varreturntypes fun_name 
        BRACKET_L sibarglist BRACKET_R sibfunbody sibpragmas
          {
            $$=MakeFundef($3, NULL, $2, $5, $7, NULL);
            switch ($1)
            {
            case 0:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=0;
              break;
            case 1:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=1;
              break;
            case 2:
              FUNDEF_STATUS($$)=ST_classfun;
              FUNDEF_INLINE($$)=0;
              break;
            }
            FUNDEF_PRAGMA($$)=$8;

           DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibFun %s",
                               mdb_nodetype[$$->nodetype],$$,
                               ItemName($$)));
          }
      | sibevmarker varreturntypes id COLON fun_name 
        BRACKET_L sibarglist BRACKET_R sibfunbody sibpragmas
          {
            $$=MakeFundef($5, $3, $2, $7, $9, NULL);
            switch ($1)
            {
            case 0:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=0;
              break;
            case 1:
              FUNDEF_STATUS($$)=ST_imported;
              FUNDEF_INLINE($$)=1;
              break;
            case 2:
              FUNDEF_STATUS($$)=ST_classfun;
              FUNDEF_INLINE($$)=0;
              break;
            }
            FUNDEF_PRAGMA($$)=$10;

           DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibFun %s",
                               mdb_nodetype[$$->nodetype],$$,
                               ItemName($$)));

          }
        ;

sibevmarker: INLINE    {$$=1;} 
        | CLASSTYPE {$$=2;}
        |           {$$=0;}
        ;

sibarglist: sibargs {$$=$1;}
          |         {$$=NULL;}
          ;

sibargs: sibarg COMMA sibargs
           {
             $$=$1;
             ARG_NEXT($$)=$3;
           }
       | sibarg
           {
             $$=$1;
           }
       ;

sibarg: type sibreference sibparam
          {
            $$=MakeArg($3, $1, ST_regular, $2, NULL);

              DBUG_PRINT("GENSIB",
                         ("%s: "P_FORMAT", Id: %s, Attrib: %d, Status: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          CHECK_NULL( ARG_NAME($$)), ARG_ATTRIB($$), ARG_STATUS($$)));
          }
      | TYPE_DOTS
          {
            $$=MakeArg(NULL, MakeTypes(T_dots), ST_regular, ST_regular, NULL); 

              DBUG_PRINT("GENSIB",
                         ("%s: "P_FORMAT", ... , Attrib: %d, Status: %d  ",
                          mdb_nodetype[ $$->nodetype ], $$, 
                          ARG_ATTRIB($$), ARG_STATUS($$)));
          }
      ;

sibparam: id
            {
              $$=$1;
            }
        |
            {
              $$=NULL;
            }
        ;


sibreference: BRACKET_L AMPERS BRACKET_R
                 {
                   $$=ST_readonly_reference;
                 }
             | AMPERS
                 {
                   $$=ST_reference;
                 }
             |
                 {
                   $$=ST_regular;
                 }
             ;

sibfunbody: exprblock
              {
                $$=$1;
              }
          | SEMIC
              {
                $$=NULL;
              }
          ;


sibpragmas: sibpragmalist
            {
              $$=store_pragma;
              store_pragma=NULL;
            }
          |
            {
              $$=NULL;
            }
          ;

sibpragmalist: sibpragmalist sibpragma
             | sibpragma
             ;

sibpragma: pragma
         | PRAGMA TYPES modnames
           {
             if (store_pragma==NULL) store_pragma=MakePragma();
             PRAGMA_NEEDTYPES(store_pragma)=$3;
           }
         | PRAGMA FUNS sibfunlist
           {
             if (store_pragma==NULL) store_pragma=MakePragma();
             PRAGMA_NEEDFUNS(store_pragma)=$3; 
           }
         | PRAGMA EXTERN id
           {
             if (store_pragma==NULL) store_pragma=MakePragma();
             PRAGMA_LINKMOD(store_pragma)=$3; 
           }
         ;

sibfunlist: sibfunlistentry COMMA sibfunlist
            {
              $$=$1;
              FUNDEF_NEXT($$)=$3;
            }
          | sibfunlistentry
            {
              $$=$1;
            }
          ;

sibfunlistentry: id BRACKET_L sibarglist BRACKET_R
                 {
                   $$=MakeFundef($1, NULL,
                                 MakeType(T_unknown, 0, NULL, NULL, NULL),
                                 $3, NULL, NULL);
                   FUNDEF_STATUS($$)=ST_imported;
                   
                   DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibNeedFun %s",
                                       mdb_nodetype[$$->nodetype],$$,
                                       ItemName($$)));
                }
               | id COLON fun_name BRACKET_L sibarglist BRACKET_R
                 {
                   $$=MakeFundef($3, $1,
                                 MakeType(T_unknown, 0, NULL, NULL, NULL),
                                 $5, NULL, NULL);

                   FUNDEF_STATUS($$)=ST_imported;
                   
                   DBUG_PRINT("GENSIB",("%s"P_FORMAT"SibNeedFun %s",
                                       mdb_nodetype[$$->nodetype],$$,
                                       ItemName($$)));
                 }
               ;

     

%%



/*
 *********************************************************************
 *
 *  functions 
 *
 *********************************************************************
 */


int yyerror(char *errname)
{
  ABORT(linenum,("%s at '%s`", errname, yytext));
}



/***
 ***  string2array
 ***/

node *string2array2string(char *str)
{
  node *new_exprs;
  int i, cnt;
  char *funname;
  char *funmod;
  node *len_exprs;
  
  DBUG_ENTER("string2array2string");
  
  new_exprs=MakeExprs(MakeChar('\0'), NULL);

  cnt=0;
  
  for (i=strlen(str)-1; i>=0; i--)
  {
    if ((i>0) && (str[i-1]=='\\'))
    {
      switch (str[i])
      {
      case 'n':
        new_exprs=MakeExprs(MakeChar('\n'), new_exprs);
        i-=1;
        break;
      case 't':
        new_exprs=MakeExprs(MakeChar('\t'), new_exprs);
        i-=1;
        break;
      case 'v':
        new_exprs=MakeExprs(MakeChar('\v'), new_exprs);
        i-=1;
        break;
      case 'b':
        new_exprs=MakeExprs(MakeChar('\b'), new_exprs);
        i-=1;
        break;
      case 'r':
        new_exprs=MakeExprs(MakeChar('\r'), new_exprs);
        i-=1;
        break;
      case 'f':
        new_exprs=MakeExprs(MakeChar('\f'), new_exprs);
        i-=1;
        break;
      case 'a':
        new_exprs=MakeExprs(MakeChar('\a'), new_exprs);
        i-=1;
        break;
      case '"':
        new_exprs=MakeExprs(MakeChar('"'), new_exprs);
        i-=1;
        break;
      default:
        new_exprs=MakeExprs(MakeChar(str[i]), new_exprs);
        break;
      }
    }
    else
    {
      new_exprs=MakeExprs(MakeChar(str[i]), new_exprs);
    }
    
    cnt+=1;
  }

  FREE(str);

  
  funname=(char*)Malloc(10);
  funmod=(char*)Malloc(11);
  len_exprs;
          
  strcpy(funname, "to_string");
  strcpy(funmod, "StringBase");
          
  len_exprs=MakeExprs(MakeNum(cnt), NULL);
          
  DBUG_RETURN(MakeAp(funname, NULL, 
                     MakeExprs(MakeArray(new_exprs), len_exprs))); 
}


/*
 *   Altes aus sac.21
 *
 */

types *GenComplexType( types *types, nums *numsp)
{
  int *destptr;
  nums *tmp;

  DBUG_ENTER("GenComplexType");

  types->shpseg=GEN_NODE(shpseg);
  destptr=types->shpseg->shp;
  do {
    types->dim++;
    *destptr++=numsp->num;
    DBUG_PRINT("GENTREE",("shape-element: %d",numsp->num));
    tmp=numsp;
    numsp=numsp->next;
    FREE(tmp);
  } while (numsp != NULL);
  DBUG_RETURN(types);
}

node *GenVardec( types *type, ids *ids_p)
{
  node *tmp, *vardec_p;
  ids *tmp2;
  types *type_p;

  DBUG_ENTER("GenVardec");

  vardec_p=NULL;
  do
   {
     tmp=MakeNode(N_vardec);
     tmp->node[0]=vardec_p;
     vardec_p=tmp;
     type_p=MakeType(T_int, 0, NULL, NULL, NULL);
     type_p=(types*) memcpy((void*)type_p, (void*)type, sizeof(types)) ;
     vardec_p->info.types=type_p;
     vardec_p->info.types->id=ids_p->id;
     tmp2=ids_p;
     ids_p=ids_p->next;
     FREE(tmp2);
  } while (ids_p != NULL);
  DBUG_RETURN(vardec_p);
}

/*
 *
 *  functionname  : Append
 *  arguments     : 1) node to append to; nodetype: N_assign or N_block
 *                  2) node to append
 *  description   : appends node 2) to node 1) and creates a new N_assign node
 *                  if necessary
 *  global vars   : ---
 *  internal funs : MakeNode
 *  external funs : ---
 *  macros        : DBUG..., P_FORMAT
 *
 *  remarks       :
 *
 */
node *Append(node *target_node, node *append_node)
{
   node *tmp;
   
   DBUG_ENTER("Append");
   if (N_block == target_node->nodetype)
      tmp=target_node->node[0];
   else
      tmp=target_node;
   if (N_assign == tmp->nodetype) 
   {
      while( tmp->node[1] )      /* look for last N_assign node */
        tmp=tmp->node[1];
   
      if (N_assign != append_node->nodetype)
      {
         tmp->node[1]=MakeNode(N_assign);
         tmp->node[1]->node[0]=append_node;
      }
      else
         tmp->node[1]=append_node;
      
      DBUG_PRINT("GENTREE",("%s"P_FORMAT": %s"P_FORMAT" %s"P_FORMAT,
                            mdb_nodetype[tmp->nodetype],tmp,
                            mdb_nodetype[tmp->node[0]->nodetype],tmp->node[0],
                            mdb_nodetype[tmp->node[1]->nodetype],tmp->node[1]));
   }
   else
   {  /* target_node has type N_empty */

      FREE(tmp);     /* delete node of type N_empty */
      if (N_assign != append_node->nodetype)
      {
         tmp=MakeNode(N_assign);  
         tmp->node[0]=append_node;
         DBUG_PRINT("GENTREE",("%s"P_FORMAT": %s"P_FORMAT,
                               mdb_nodetype[tmp->nodetype],tmp,
                               mdb_nodetype[tmp->node[0]->nodetype],
                               tmp->node[0]));
      } 
      else
      {
         tmp=append_node;
         
         DBUG_PRINT("GENTREE",("%s"P_FORMAT,
                               mdb_nodetype[tmp->nodetype],tmp));
      }
   }
      
   DBUG_RETURN(target_node);
}


