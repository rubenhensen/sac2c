/*
 *
 * $Log$
 * Revision 1.1  1995/12/23 17:28:39  cg
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
 *  functionname  : FindSibEntry
 *  arguments     : 1) pointer to N_fundef or N_typedef node
 *  description   : finds the respective N_fundef or N_typedef node in the
 *                  sib-tree that provides additional information about
 *                  the given node.
 *  global vars   : ---
 *  internal funs : FindModul
 *  external funs : ---
 *  macros        : CMP_FUNDEF, CMP_TYPEDEF
 *
 *  remarks       :
 *
 */

node *
FindSibEntry (node *orig)
{
    char *mod_name;
    node *sib_entry = NULL;
    mod *mod;

    DBUG_ENTER ("FindSibEntry");

    mod_name = orig->ID_MOD;
    if (mod_name != NULL) {
        mod = FindModul (mod_name);
        if (mod != NULL) {
            sib_entry = mod->sib;
            if (sib_entry != NULL) {
                if (orig->nodetype == N_typedef) {
                    sib_entry = sib_entry->node[0];
                    while ((sib_entry != NULL) && (!CMP_TYPEDEF (sib_entry, orig))) {
                        sib_entry = sib_entry->node[0];
                    }
                } else {
                    sib_entry = sib_entry->node[1];
                    while ((sib_entry != NULL) && (!CMP_FUNDEF (sib_entry, orig))) {
                        sib_entry = sib_entry->node[1];
                    }
                }
            }
        }
    }

    if (sib_entry == NULL) {
        DBUG_PRINT ("READSIB", ("NO SIB-entry found for %s:%s", orig->ID_MOD, orig->ID));
    } else {
        DBUG_PRINT ("READSIB", ("SIB-entry found for %s:%s", orig->ID_MOD, orig->ID));
    }

    DBUG_RETURN (sib_entry);
}

/*
 *
 *  functionname  : AddSymbol
 *  arguments     : 1) name of symbol
 *                  2) module of symbol
 *                  3) type of symbol
 *  description   : adds implicitly imported symbol to global mod_tab if
 *                  it is yet unknown
 *  global vars   : ---
 *  internal funs : FindModul
 *  external funs : Malloc
 *  macros        :
 *
 *  remarks       :
 *
 */

void
AddSymbol (char *name, char *module, int symbkind)
{
    mod *tmp;
    int i;
    syms *sym;

    DBUG_ENTER ("AddSymbol");

    tmp = FindModul (module);
    if (tmp == NULL) {
        tmp = (mod *)Malloc (sizeof (mod));
        tmp->name = name;
        tmp->prefix = name;
        tmp->flag = 0;
        tmp->allflag = 0;
        tmp->moddec = NULL;
        tmp->sib = NULL;
        for (i = 0; i < 4; i++) {
            tmp->syms[i] = NULL;
        }

        tmp->syms[symbkind] = (syms *)Malloc (sizeof (syms));
        tmp->syms[symbkind]->id = name;
        tmp->syms[symbkind]->next = NULL;
        tmp->syms[symbkind]->flag = 0;
    } else {
        sym = tmp->syms[symbkind];
        while ((sym != NULL) && (strcmp (sym->id, name) != 0)) {
            sym = sym->next;
        }
        if (sym == NULL) {
            sym = (syms *)Malloc (sizeof (syms));
            sym->id = name;
            sym->flag = 0;
            sym->next = tmp->syms[symbkind];
            tmp->syms[symbkind] = sym;
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : EnsureExistObjects
 *  arguments     : 1) objdef node from sib, contains an object that is
 *                     implicitly used by one of the sib-functions
 *                  2) modul node of program
 *  description   : ensures that the object is declared in the syntax tree.
 *                  If the object is yet unknown, the objdef node
 *                  is inserted. A nodelist of all objects needed by the
 *                  particular function is generated and returned.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist
 *  macros        : CMP_OBJDEF
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistObjects (node *object, node *modul)
{
    node *find_obj, *next;
    nodelist *objlist = NULL;
    statustype keep_attrib;

    DBUG_ENTER ("EnsureExistObjects");

    while (object != NULL) {
        find_obj = modul->node[3];
        keep_attrib = object->info.types->attrib;
        object->info.types->attrib = ST_regular;

        while ((find_obj != NULL) && (!CMP_OBJDEF (object, find_obj))) {
            find_obj = find_obj->node[0];
        }

        next = object->node[0];

        if (find_obj == NULL) /* the object does not yet exist */
        {
            find_obj = object;
            find_obj->node[0] = modul->node[3];
            find_obj->nnode = (modul->node[3] == NULL) ? 0 : 1;

            modul->node[3] = find_obj;

            /*
                  if (FindModul(object->ID_MOD) == NULL)
                  {
                    AddToLinkerTab(object->ID_MOD);
                  }
            */
            if (object->ID_MOD != NULL) {
                AddSymbol (object->ID, object->ID_MOD, 3);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            DBUG_PRINT ("READSIB", ("Implicitly used object %s:%s inserted.",
                                    find_obj->ID_MOD, find_obj->ID));

        } else {
            free (object);

            DBUG_PRINT ("READSIB", ("Implicitly used object %s:%s already exists.",
                                    find_obj->ID_MOD, find_obj->ID));
        }

        objlist = MakeNodelist (find_obj, ST_regular, objlist);

        object = next;
    }

    DBUG_RETURN (objlist);
}

/*
 *
 *  functionname  : EnsureExistTypes
 *  arguments     : 1) typedef node from sib, contains a type that is
 *                     implicitly used by one of the sib-functions
 *                  2) modul node of program
 *  description   : ensures that the type is declared in the syntax tree.
 *                  If the type is yet unknown, the typedef node
 *                  is inserted. A nodelist of all types needed by the
 *                  particular function is generated and returned.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist
 *  macros        : CMP_TYPEDEF
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistTypes (node *type, node *modul)
{
    node *find_type, *next;
    nodelist *typelist = NULL;

    DBUG_ENTER ("EnsureExistTypes");

    while (type != NULL) {
        find_type = modul->node[1];
        while ((find_type != NULL) && (!CMP_TYPEDEF (type, find_type))) {
            find_type = find_type->node[0];
        }

        next = type->node[0];

        if (find_type == NULL) /* the type does not yet exist */
        {
            find_type = type;

            find_type->node[0] = modul->node[1];
            find_type->nnode = (modul->node[1] == NULL) ? 0 : 1;
            modul->node[1] = find_type;

            if (type->ID_MOD != NULL) {
                AddSymbol (type->ID, type->ID_MOD, 1);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            DBUG_PRINT ("READSIB",
                        ("Implicitly used type %s:%s inserted.", type->ID_MOD, type->ID));

        } else {
            DBUG_PRINT ("READSIB", ("Implicitly used type %s:%s already exists.",
                                    type->ID_MOD, type->ID));

            free (type);
        }

        typelist = MakeNodelist (find_type, ST_regular, typelist);

        type = next;
    }

    DBUG_RETURN (typelist);
}

/*
 *
 *  functionname  : EnsureExistFuns
 *  arguments     : 1) fundef node from sib, contains a function that is
 *                     implicitly used by one of the sib-functions
 *                  2) modul node of program
 *  description   : ensures that the function is declared in the syntax tree.
 *                  If the function is yet unknown, the fundef node
 *                  is inserted. A nodelist of all functions needed by the
 *                  particular function is generated and returned.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : MakeNodelist
 *  macros        : CMP_FUNDEF
 *
 *  remarks       :
 *
 */

nodelist *
EnsureExistFuns (node *fundef, node *modul)
{
    node *find_fun, *next, *last;
    nodelist *funlist = NULL;

    DBUG_ENTER ("EnsureExistFuns");

    while (fundef != NULL) /* search function */
    {
        find_fun = modul->node[2];
        last = find_fun;

        while ((find_fun != NULL) && (!CMP_FUNDEF (fundef, find_fun))) {
            last = find_fun;
            find_fun = find_fun->node[1];
        }

        next = fundef->node[1];

        if (find_fun == NULL) /* the function does not yet exist */
        {
            find_fun = fundef;

            find_fun->node[1] = NULL;
            find_fun->nnode = 1;

            FUNDEF_NEEDOBJS (find_fun) = EnsureExistObjects (find_fun->node[4], modul);

            if (modul->node[2] == NULL) {
                modul->node[2] = find_fun;
            } else {
                last->nnode += 1;
                last->node[1] = find_fun;
            }

            /*
                  if (FindModul(fundef->ID_MOD) == NULL)
                  {
                    AddToLinkerTab(fundef->ID_MOD);
                  }
            */
            if (fundef->ID_MOD != NULL) {
                AddSymbol (fundef->ID, fundef->ID_MOD, 2);
            }

            /* new symbol is added to mod_tab if it's a sac-symbol */

            DBUG_PRINT ("READSIB", ("Implicitly used function %s:%s inserted.",
                                    fundef->ID_MOD, fundef->ID));

        } else {
            DBUG_PRINT ("READSIB", ("Implicitly used function %s:%s already exists.",
                                    fundef->ID_MOD, fundef->ID));

            free (fundef);
        }

        funlist = MakeNodelist (find_fun, ST_regular, funlist);

        fundef = next;
    }

    DBUG_RETURN (funlist);
}

/*
 *
 *  functionname  : IMfundef
 *  arguments     : 1) pointer to N_fundef node
 *                  2) pointer to N_modul node of respective program
 *  description   : retrieves information from SIB for respective function.
 *                  Implicitly used types, objects, and other functions
 *                  are imported if necessary. Inline information is
 *                  stored as regular function body.
 *  global vars   : ---
 *  internal funs : FindSibEntry, MakeArgList, EnsureExistObjects,
 *                  EnsureExistTypes, EnsureExistFuns
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IMfundef (node *arg_node, node *arg_info)
{
    node *sib_entry;

    DBUG_ENTER ("IMfundef");

    if (arg_node->ID_MOD != NULL) {
        sib_entry = FindSibEntry (arg_node);

        if (sib_entry != NULL) /* SIB information available */
        {
            if (sib_entry->node[0] == NULL) /* only implicit object information */
            {
                FUNDEF_NEEDOBJS (arg_node)
                  = EnsureExistObjects (sib_entry->node[4], arg_info);

                DBUG_PRINT ("READSIB",
                            ("Adding implicit object information to function %s:%s",
                             arg_node->info.types->id_mod, arg_node->info.types->id));

            } else /* function inlining information */
            {
                arg_node->node[0] = sib_entry->node[0]; /* take body from SIB */
                arg_node->node[2] = sib_entry->node[2]; /* take args from SIB */

                FUNDEF_NEEDOBJS (arg_node)
                  = EnsureExistObjects (sib_entry->node[4], arg_info);

                FUNDEF_NEEDTYPES (arg_node)
                  = EnsureExistTypes (sib_entry->node[3], arg_info);

                FUNDEF_NEEDFUNS (arg_node)
                  = EnsureExistFuns (sib_entry->node[5], arg_info);

                arg_node->flag = 1; /* inline flag */

                DBUG_PRINT ("READSIB", ("Adding inline information to function %s:%s",
                                        arg_node->ID_MOD, arg_node->ID));
            }
        }
    }

    if (arg_node->node[1] != NULL) {
        Trav (arg_node->node[1], arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IMtypedef
 *  arguments     : 1) pointer to N_typedef node
 *                  2) unused, necessary for traversal mechanism
 *  description   : retrieves information from sib for specific type
 *                  definition. The additional types-structure containing
 *                  the implementation of a T_hidden type is added as a
 *                  second types-structure reusing the "next" entry.
 *  global vars   : ---
 *  internal funs : FindSibEntry
 *  external funs : Trav
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
IMtypedef (node *arg_node, node *arg_info)
{
    node *sib_entry;

    DBUG_ENTER ("IMtypedef");

    if (arg_node->info.types->simpletype == T_hidden) {
        sib_entry = FindSibEntry (arg_node);
        if (sib_entry != NULL) {
            arg_node->info.types->next = sib_entry->info.types;

            DBUG_PRINT ("READSIB",
                        ("adding implementation of hidden type %s:%s",
                         MOD (arg_node->info.types->id_mod), arg_node->info.types->id));
        }
    }

    if (arg_node->node[0] != NULL) {
        Trav (arg_node->node[0], NULL);
    }

    DBUG_RETURN (arg_node);
}
