/*
 *
 * $Log$
 * Revision 1.7  1996/01/22 18:35:31  cg
 * added new pragmas for global objects: effect, initfun
 *
 * Revision 1.6  1996/01/07  16:59:29  cg
 * pragmas copyfun, freefun, linkname, effect, touch and readonly
 * are now immediately resolved
 *
 * Revision 1.5  1996/01/05  12:39:26  cg
 * Now, SIB information is retrieved from SAC library files.
 * The existence of all necessary module/class implementations
 * is checked and archive files are extracted from library files.
 *
 * Revision 1.4  1996/01/02  17:48:07  cg
 * Typedefs in SIBs which are again based on user-defined types are now resolved.
 *
 * Revision 1.3  1996/01/02  16:09:21  cg
 * some bugs fixed
 *
 * Revision 1.2  1995/12/29  10:41:52  cg
 * first running revision for new SIBs
 *
 * Revision 1.1  1995/12/23  17:28:39  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "traverse.h"

#include "import.h"
#include "scnprs.h"
#include "filemgr.h"

/*
 *  global variables :
 */

static node *sibs = NULL; /* start of list of N_sib nodes storing parsed
                             SAC Information Blocks    */

/*
 *  forward declarations
 */

extern nodelist *EnsureExistTypes (ids *type, node *modul, node *sib);

/*
 *
 *  functionname  : ReadSib
 *  arguments     : 1) syntax tree
 *  description   : retrieves information about types, functions, and objects
 *                  from SAC Information Blocks
 *  global vars   : act_tab, readsib_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : ---
 *
 *  remarks       :
 *
 */

node *
ReadSib (node *syntax_tree)
{
    DBUG_ENTER ("ReadSib");

    act_tab = readsib_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  : CreateArchive
 *  arguments     : 1) module/class name
 *  description   : opens archive file in storedir and copies yyin to it.
 *  global vars   : yyin, storedirname
 *  internal funs : ---
 *  external funs : WriteOpen, fscanf, fprintf, fclose, feof, SystemCall
 *  macros        :
 *
 *  remarks       : is called in sac.y when parsing end of SIB
 *
 */

void
CreateArchive (char *name)
{
    char tmp;
    FILE *archive;

    DBUG_ENTER ("CreateArchive");

    archive = WriteOpen ("%s%s.a", store_dirname, name);

    while (!feof (yyin)) {
        fscanf (yyin, "%c", &tmp);
        fprintf (archive, "%c", tmp);
    }

    fclose (archive);

    SystemCall ("ranlib %s%s.a", store_dirname, name);

    /*
     *  ranlib must be rerun here because otherwise the date of the
     *  archive's symbol table is older than the archive file itself
     *  which causes error messages by the C-compiler.
     */

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : FindSib
 *  arguments     : 1) name of module/class
 *  description   : checks in the list of sibs if this sib has already been
 *                  read. In this case, a pointer to the N_sib node is
 *                  returned. Otherwise the sib is parsed, added to the list
 *                  of sibs and a pointer to the new N_sib node is
 *                  returned.
 *  global vars   : sibs
 *  internal funs : ---
 *  external funs : strcmp, strcpy, strcat, fopen, fclose
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
FindSib (char *name)
{
    static char buffer[MAX_FILE_NAME];
    node *tmp;
    char *pathname;

    DBUG_ENTER ("FindSib");

    DBUG_ASSERT (name != NULL, "called FindSib with name==NULL");

    tmp = sibs;

    while ((tmp != NULL) && (0 != strcmp (name, SIB_NAME (tmp)))) {
        tmp = SIB_NEXT (tmp);
    }

    if (tmp == NULL) {
        strcpy (buffer, name);
        strcat (buffer, ".lib");

        NOTE (("Searching for implementation of SAC module/class '%s` ...", name));

        pathname = FindFile (MODIMP_PATH, buffer);

        if (pathname == NULL) {
            SYSABORT (("Unable to find implementation of SAC module/class '%s`", name));
        }

        NOTE (("  Evaluating SAC library \"%s\" !", pathname));

        yyin = fopen (pathname, "r");
        linenum = 1;
        start_token = PARSE_SIB;

        yyparse ();

        fclose (yyin);

        SIB_NEXT (sib_tree) = sibs;
        sibs = sib_tree;

        tmp = sibs;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : FindSibEntry
 *  arguments     : 1) pointer to N_fundef, N_typedef, or N_objdef node
 *                  2) N_sib node where to look in
 *  description   : finds the respective node in the
 *                  sib-tree that provides additional information about
 *                  the given node.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : CMP_FUNDEF, CMP_TYPEDEF, CMP_OBJDEF
 *
 *  remarks       :
 *
 */

node *
FindSibEntry (node *item, node *sib)
{
    node *tmp;

    DBUG_ENTER ("FindSibEntry");

    switch (NODE_TYPE (item)) {
    case N_typedef:
        tmp = SIB_TYPES (sib);
        while ((tmp != NULL) && (!CMP_TYPEDEF (item, tmp))) {
            tmp = TYPEDEF_NEXT (tmp);
        }
        break;

    case N_objdef:
        tmp = SIB_OBJS (sib);
        while ((tmp != NULL) && (!CMP_OBJDEF (item, tmp))) {
            tmp = OBJDEF_NEXT (tmp);
        }
        break;

    case N_fundef:
        tmp = SIB_FUNS (sib);
        while ((tmp != NULL) && (!CMP_FUNDEF (item, tmp))) {
            tmp = FUNDEF_NEXT (tmp);
        }
        break;

    default:
        DBUG_ASSERT (0, ("Wrong node type in call of function FindSibEntry"));
    }

    if (tmp == NULL) {
        DBUG_PRINT ("READSIB", ("No SIB entry for %s", ItemName (item)));
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : ExtractTypeFromSib
 *  arguments     : 1) N_typedef node of sib to be extracted
 *                  2) N_sib where to extract from
 *                  3) N_modul where to insert extracted N_typedef node
 *  description   : 1) is removed from the chain of typedefs in 2) and
 *                  added to the beginning of the chain of typedefs in 3)
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : InitGenericFuns
 *  macros        :
 *
 *  remarks       : compare ExtractObjFromSib
 *                  Different functions are used for types and objects
 *                  with respect to different access macros in the new
 *                  virtual syntax tree.
 *
 */

void
ExtractTypeFromSib (node *type, node *sib, node *modul)
{
    node *tmp;

    DBUG_ENTER ("ExtractTypeFromSib");

    if (SIB_TYPES (sib) == type) {
        SIB_TYPES (sib) = TYPEDEF_NEXT (type);
    } else {
        tmp = SIB_TYPES (sib);
        while (TYPEDEF_NEXT (tmp) != type) {
            tmp = TYPEDEF_NEXT (tmp);
        }
        TYPEDEF_NEXT (tmp) = TYPEDEF_NEXT (type);
    }

    type = InitGenericFuns (type, TYPEDEF_PRAGMA (type));

    if (MODUL_TYPES (modul) == NULL) {
        MODUL_TYPES (modul) = type;
        TYPEDEF_NEXT (type) = NULL;

        /******************************************************/
#ifndef NEWTREE
        type->nnode = 0;
#endif
        /******************************************************/

    } else {
        TYPEDEF_NEXT (type) = MODUL_TYPES (modul);
        MODUL_TYPES (modul) = type;

        /******************************************************/
#ifndef NEWTREE
        type->nnode = 1;
#endif
        /******************************************************/
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ExtractObjFromSib
 *  arguments     : 1) N_objdef node of sib to be extracted
 *                  2) N_sib where to extract from
 *                  3) N_modul where to insert extracted N_objdef node
 *  description   : 1) is removed from the chain of objdefs in 2) and
 *                  added to the beginning of the chain of objdefs in 3)
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        :
 *
 *  remarks       : compare ExtractTypeFromSib
 *                  Different functions are used for types and objects
 *                  with respect to different access macros in the new
 *                  virtual syntax tree.
 *
 */

void
ExtractObjFromSib (node *obj, node *sib, node *modul)
{
    node *tmp;

    DBUG_ENTER ("ExtractObjFromSib");

    if (SIB_OBJS (sib) == obj) {
        SIB_OBJS (sib) = OBJDEF_NEXT (obj);
    } else {
        tmp = SIB_OBJS (sib);

        while (OBJDEF_NEXT (tmp) != obj) {
            tmp = OBJDEF_NEXT (tmp);
        }

        OBJDEF_NEXT (tmp) = OBJDEF_NEXT (obj);
    }

    OBJDEF_LINKMOD (obj) = SIB_NAME (sib);

    if (MODUL_OBJS (modul) == NULL) {
        MODUL_OBJS (modul) = obj;
    } else {
        tmp = MODUL_OBJS (modul);

        while (OBJDEF_NEXT (tmp) != NULL) {
            tmp = OBJDEF_NEXT (tmp);
        }

        OBJDEF_NEXT (tmp) = obj;
    }

    OBJDEF_NEXT (obj) = NULL;

    /******************************************************/
#ifndef NEWTREE
    obj->nnode = 1;
#endif
    /******************************************************/

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AddFunToModul
 *  arguments     : 1) N_fundef node to add to syntax tree
 *                  2) N_modul node of current program
 *  description   : N_fundef node from funlist in pragma functions
 *                  (used in SIBs) is added at the end of the chain of
 *                  functions in 2)
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        :
 *
 *  remarks       : 1) need not to be extracted from the funlist because
 *                  this is traversed exactly once.
 *
 */

void
AddFunToModul (node *fun, node *modul)
{
    node *tmp;

    DBUG_ENTER ("AddFunToModul");

    tmp = MODUL_FUNS (modul);

    while (FUNDEF_NEXT (tmp) != NULL) {
        tmp = FUNDEF_NEXT (tmp);
    }

    FUNDEF_NEXT (tmp) = fun;
    FUNDEF_NEXT (fun) = NULL;

    /******************************************************/
#ifndef NEWTREE
    fun->nnode = 1;
    tmp->nnode = 2;
#endif
    /******************************************************/

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : EnsureExistObjects
 *  arguments     : 1) global object from pragma touch or effect
 *                  2) modul node of program
 *                  3) N_sib node to extract object definition
 *                  4) attrib of global object
 *                     (effect->ST_reference, touch->ST_readonly_reference)
 *  description   : checks if the global objects mentioned in touch or
 *                  effect pragmas alreaddy exist in the current context.
 *                  If not, the correct N_objdef node is extracted from
 *                  the SIB and inserted into the syntax tree.
 *                  A node list of needed objects for this particular
 *                  function is returned.
 *  global vars   : ---
 *  internal funs : ExtractObjFromSib
 *  external funs : MakeNodelist, SearchObjdef, AddSymbol
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistObjects (ids *object, node *modul, node *sib, statustype attrib)
{
    node *find;
    nodelist *objlist = NULL;
    ids *objtype;

    DBUG_ENTER ("EnsureExistObjects");

    while (object != NULL) {
        find = SearchObjdef (IDS_NAME (object), IDS_MOD (object), MODUL_OBJS (modul));

        if (find == NULL) {
            find = SearchObjdef (IDS_NAME (object), IDS_MOD (object), SIB_OBJS (sib));

            DBUG_PRINT ("READSIB", ("Looking for implicitly used object %s",
                                    ModName (IDS_MOD (object), IDS_NAME (object))));

            DBUG_ASSERT (find != NULL, "No info about object in SIB");

            ExtractObjFromSib (find, sib, modul);

            if (IDS_MOD (object) != NULL) {
                AddSymbol (IDS_NAME (object), IDS_MOD (object), 3);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            DBUG_PRINT ("READSIB",
                        ("Implicitly used object %s inserted.", ItemName (find)));

            objtype = MakeIds (StringCopy (OBJDEF_TNAME (find)), OBJDEF_TMOD (find),
                               ST_regular);
            EnsureExistTypes (objtype, modul, sib);

            OBJDEF_SIB (find) = sib;
        } else {
            DBUG_PRINT ("READSIB",
                        ("Implicitly used object %s already exists.", ItemName (find)));
        }

        objlist = MakeNodelist (find, ST_regular, objlist);
        NODELIST_ATTRIB (objlist) = attrib;

        object = IDS_NEXT (object);
    }

    DBUG_RETURN (objlist);
}

/*
 *
 *  functionname  : EnsureExistTypes
 *  arguments     : 1) type from pragma types (used in SIBs)
 *                  2) modul node of program
 *                  3) N_sib node to extract object definition
 *  description   : checks if the types mentioned in the pragma
 *                  already exist in the current context.
 *                  If not, the correct N_typedef node is extracted from
 *                  the SIB and inserted into the syntax tree.
 *                  A node list of needed types for this particular
 *                  function is returned.
 *  global vars   : ---
 *  internal funs : ExtractTypeFromSib
 *  external funs : MakeNodelist, AddSymbol, SearchTypedef, FreeOneIds
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistTypes (ids *type, node *modul, node *sib)
{
    node *find;
    nodelist *typelist = NULL;

    DBUG_ENTER ("EnsureExistTypes");

    while (type != NULL) {
        find = SearchTypedef (IDS_NAME (type), IDS_MOD (type), MODUL_TYPES (modul));

        if (find == NULL) {
            find = SearchTypedef (IDS_NAME (type), IDS_MOD (type), SIB_TYPES (sib));

            DBUG_PRINT ("READSIB", ("Looking for implicitly used type %s",
                                    ModName (IDS_MOD (type), IDS_NAME (type))));

            DBUG_ASSERT (find != NULL, "No info about type in SIB ");

            ExtractTypeFromSib (find, sib, modul);

            if (IDS_MOD (type) != NULL) {
                AddSymbol (IDS_NAME (type), IDS_MOD (type), 1);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            if (TYPEDEF_BASETYPE (find) == T_user) {
                EnsureExistTypes (MakeIds (TYPEDEF_TNAME (find), TYPEDEF_TMOD (find),
                                           ST_regular),
                                  modul, sib);
            }

            /*
             *  If the definition of the type is again user-defined, its existence
             *  must be guaranteed, too.
             */

            DBUG_PRINT ("READSIB",
                        ("Implicitly used type %s inserted.", ItemName (find)));
        } else {
            DBUG_PRINT ("READSIB",
                        ("Implicitly used type %s already exists.", ItemName (find)));
        }

        typelist = MakeNodelist (find, ST_regular, typelist);

        type = FreeOneIds (type);
    }

    DBUG_RETURN (typelist);
}

/*
 *
 *  functionname  : EnsureExistFuns
 *  arguments     : 1) N_fundef node from pragma functions (used in SIBs)
 *                  2) modul node of program
 *                  3) N_sib node to extract object definition
 *  description   : checks if the functions mentioned in the pragma
 *                  already exist in the current context.
 *                  If not, 1) is extracted from
 *                  the SIB and inserted into the syntax tree.
 *                  By inserting it at the end of the function list,
 *                  it is guaranteed that this function will still be
 *                  traversed by RSIBfundef.
 *                  A node list of needed types for this particular
 *                  function is returned.
 *  global vars   : ---
 *  internal funs : AddFunToModul
 *  external funs : MakeNodelist, FreeNode, strcmp, SearchFundef
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistFuns (node *fundef, node *modul, node *sib)
{
    node *find, *next;
    nodelist *funlist = NULL;

    DBUG_ENTER ("EnsureExistFuns");

    while (fundef != NULL) /* search function */
    {
        find = SearchFundef (fundef, MODUL_FUNS (modul));

        if (find == NULL) {
            DBUG_PRINT ("READSIB",
                        ("Looking for implicitly used function %s", ItemName (fundef)));

            next = FUNDEF_NEXT (fundef);

            FUNDEF_LINKMOD (fundef) = SIB_NAME (sib);

            FUNDEF_SIB (fundef) = sib;

            AddFunToModul (fundef, modul);

            find = fundef;
            fundef = next;

            DBUG_PRINT ("READSIB",
                        ("Implicitly used function %s inserted.", ItemName (find)));
        } else {
            fundef = FreeNode (fundef);

            DBUG_PRINT ("READSIB",
                        ("Implicitly used function %s already exists.", ItemName (find)));
        }

        funlist = MakeNodelist (find, ST_regular, funlist);
    }

    DBUG_RETURN (funlist);
}

/*
 *
 *  functionname  : RSIBfundef
 *  arguments     : 1) pointer to N_fundef node
 *                  2) pointer to N_modul node of respective program
 *  description   : retrieves information from SIB for respective function.
 *                  Implicitly used types, objects, and other functions
 *                  are imported if necessary. Inline information is
 *                  stored as regular function body.
 *                  The pragmas effect and touch of external functions
 *                  are converted to node lists of the respective
 *                  N_objdef nodes.
 *  global vars   : ---
 *  internal funs : FindSib, FindSibEntry, MakeArgList, EnsureExistObjects,
 *                  EnsureExistTypes, EnsureExistFuns, CheckExistObjects,
 *                  CheckExternalImplementation
 *  external funs : Trav, ConcatNodelist, CountFunctionParams
 *                  Nums2BoolArray, Nums2IntArray
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RSIBfundef (node *arg_node, node *arg_info)
{
    node *sib_entry, *sib = NULL, *pragma;
    int count_params;

    DBUG_ENTER ("RSIBfundef");

    if (FUNDEF_SIB (arg_node) != NULL) {
        sib = FUNDEF_SIB (arg_node);
        FUNDEF_SIB (arg_node) = NULL;
    } else {
        if ((FUNDEF_STATUS (arg_node) == ST_imported)
            && (FUNDEF_MOD (arg_node) != NULL)) {
            sib = FindSib (FUNDEF_MOD (arg_node));
        }
    }

    if (sib != NULL) {
        sib_entry = FindSibEntry (arg_node, sib);

        if (sib_entry != NULL) /* SIB information available */
        {
            DBUG_PRINT ("READSIB", ("Copying body, args, return types, and inline flag"
                                    "from SIB to function %s",
                                    ItemName (arg_node)));

            FUNDEF_BODY (arg_node) = FUNDEF_BODY (sib_entry);
            FUNDEF_INLINE (arg_node) = FUNDEF_INLINE (sib_entry);
            FUNDEF_ARGS (arg_node) = FUNDEF_ARGS (sib_entry);
            FUNDEF_TYPES (arg_node) = FUNDEF_TYPES (sib_entry);
            FUNDEF_LINKMOD (arg_node) = SIB_NAME (sib);

            if (FUNDEF_PRAGMA (sib_entry) != NULL) {
                pragma = FUNDEF_PRAGMA (sib_entry);

                count_params = CountFunctionParams (arg_node);

                PRAGMA_NUMPARAMS (pragma) = count_params;

                if (PRAGMA_LINKSIGNNUMS (pragma) != NULL) {
                    DBUG_PRINT ("READSIB", ("Converting pragma linksign"));

                    PRAGMA_LINKSIGN (pragma)
                      = Nums2IntArray (NODE_LINE (arg_node), count_params,
                                       PRAGMA_LINKSIGNNUMS (pragma));
                }

                if (PRAGMA_REFCOUNTINGNUMS (pragma) != NULL) {
                    DBUG_PRINT ("READSIB", ("Converting pragma refcounting"));

                    PRAGMA_REFCOUNTING (pragma)
                      = Nums2BoolArray (NODE_LINE (arg_node), count_params,
                                        PRAGMA_REFCOUNTINGNUMS (pragma));
                }

                if (PRAGMA_READONLYNUMS (pragma) != NULL) {
                    arg_node = ResolvePragmaReadonly (arg_node, pragma, count_params);
                }

                DBUG_PRINT ("READSIB", ("Resolving touched objects of function %s",
                                        ItemName (arg_node)));

                FUNDEF_NEEDOBJS (arg_node)
                  = EnsureExistObjects (PRAGMA_TOUCH (pragma), arg_info, sib,
                                        ST_readonly_reference);
                PRAGMA_TOUCH (pragma) = NULL;

                DBUG_PRINT ("READSIB", ("Resolving effected objects of function %s",
                                        ItemName (arg_node)));

                FUNDEF_NEEDOBJS (arg_node)
                  = ConcatNodelist (EnsureExistObjects (PRAGMA_EFFECT (pragma), arg_info,
                                                        sib, ST_reference),
                                    FUNDEF_NEEDOBJS (arg_node));
                PRAGMA_EFFECT (pragma) = NULL;

                DBUG_PRINT ("READSIB", ("Resolving needed types of function %s",
                                        ItemName (arg_node)));

                FUNDEF_NEEDTYPES (arg_node)
                  = EnsureExistTypes (PRAGMA_NEEDTYPES (pragma), arg_info, sib);
                PRAGMA_NEEDTYPES (pragma) = NULL;

                DBUG_PRINT ("READSIB", ("Resolving needed functions of function %s",
                                        ItemName (arg_node)));

                FUNDEF_NEEDFUNS (arg_node)
                  = EnsureExistFuns (PRAGMA_NEEDFUNS (pragma), arg_info, sib);
                PRAGMA_NEEDFUNS (pragma) = NULL;
            }
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RSIBobjdef
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RSIBobjdef (node *arg_node, node *arg_info)
{
    node *sib_entry, *sib = NULL;

    DBUG_ENTER ("RSIBobjdef");

    if (OBJDEF_SIB (arg_node) != NULL) {
        sib = OBJDEF_SIB (arg_node);
        OBJDEF_SIB (arg_node) = NULL;
    } else {
        if ((OBJDEF_STATUS (arg_node) == ST_imported)
            && (OBJDEF_MOD (arg_node) != NULL)) {
            sib = FindSib (OBJDEF_MOD (arg_node));
        }
    }

    if (sib != NULL) {
        sib_entry = FindSibEntry (arg_node, sib);

        if ((sib_entry != NULL) && (OBJDEF_PRAGMA (sib_entry) != NULL)) {
            OBJDEF_NEEDOBJS (arg_node) = EnsureExistObjects (OBJDEF_EFFECT (sib_entry),
                                                             arg_info, sib, ST_reference);
        }
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RSIBtypedef
 *  arguments     : 1) pointer to N_typedef node
 *                  2) arg_info unused
 *  description   : retrieves information from sib about implementation
 *                  of a hidden SAC-types
 *  global vars   : ---
 *  internal funs : FindSibEntry, FindSib
 *  external funs : Trav
 *  macros        : DBUG, TREE, ERROR
 *
 *  remarks       :
 *
 */

node *
RSIBtypedef (node *arg_node, node *arg_info)
{
    node *sib_entry;

    DBUG_ENTER ("RSIBtypedef");

    if ((TYPEDEF_BASETYPE (arg_node) == T_hidden) && (TYPEDEF_MOD (arg_node) != NULL)) {
        sib_entry = FindSibEntry (arg_node, FindSib (TYPEDEF_MOD (arg_node)));

        if (sib_entry != NULL) {
            TYPEDEF_IMPL (arg_node) = TYPEDEF_TYPE (sib_entry);

            DBUG_PRINT ("READSIB",
                        ("Adding implementation of hidden type %s", ItemName (arg_node)));
        } else {
            SYSERROR (("No implementation for hidden type %s", ItemName (arg_node)));
        }
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RSIBmodul
 *  arguments     : 1) N_modul node of current program
 *                  2) arg_info unused
 *  description   : starts traversals of the functions, objects and types
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
RSIBmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RSIBmodul");

    /*
     *  searching SIB-information about functions
     */

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_node);
    }

    /*
     *  searching SIB-information about implicit types
     */

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    /*
     *  The objects must be traversed in order to guarantee that all
     *  relevant module/class implementations are copied to store_dirname.
     */

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}
