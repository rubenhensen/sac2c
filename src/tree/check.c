
#define NEW_INFO
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "check_lib.h"

struct INFO {
};

static info *
MakeInfo ()
{
    info *result;
    DBUG_ENTER ("MakeInfo");
    result = Malloc (sizeof (info));
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");
    info = Free (info);
    DBUG_RETURN (info);
}

node *
CHKannotate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKannotate");

    // this attribute is mandatory = yes
    CHKExistAttribute (ANNOTATE_FUN (arg_node), arg_node,
                       "mandatory attribute ANNOTATE_FUN is NULL");

    CHKRightType (ANNOTATE_FUN (arg_node), arg_node, "LINK",
                  "attribute ANNOTATE_FUN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ANNOTATE_FUNAPNUMBER (arg_node), arg_node,
                       "mandatory attribute ANNOTATE_FUNAPNUMBER is NULL");

    CHKRightType (ANNOTATE_FUNAPNUMBER (arg_node), arg_node, "INTEGER",
                  "attribute ANNOTATE_FUNAPNUMBER hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ANNOTATE_FUNNUMBER (arg_node), arg_node,
                       "mandatory attribute ANNOTATE_FUNNUMBER is NULL");

    CHKRightType (ANNOTATE_FUNNUMBER (arg_node), arg_node, "INTEGER",
                  "attribute ANNOTATE_FUNNUMBER hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ANNOTATE_TAG (arg_node), arg_node,
                       "mandatory attribute ANNOTATE_TAG is NULL");

    CHKRightType (ANNOTATE_TAG (arg_node), arg_node, "INTEGER",
                  "attribute ANNOTATE_TAG hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKap");

    // this attribute is mandatory = yes
    CHKExistAttribute (AP_ARGTAB (arg_node), arg_node,
                       "mandatory attribute AP_ARGTAB is NULL");

    CHKRightType (AP_ARGTAB (arg_node), arg_node, "ARGTAB",
                  "attribute AP_ARGTAB hasnt the right type");

    CHKRightType (AP_ATFLAG (arg_node), arg_node, "INTEGER",
                  "attribute AP_ATFLAG hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AP_FUNDEF (arg_node), arg_node,
                       "mandatory attribute AP_FUNDEF is NULL");

    CHKRightType (AP_FUNDEF (arg_node), arg_node, "EXTLINK",
                  "attribute AP_FUNDEF hasnt the right type");

    CHKRightType (AP_MOD (arg_node), arg_node, "SHAREDSTRING",
                  "attribute AP_MOD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AP_NAME (arg_node), arg_node,
                       "mandatory attribute AP_NAME is NULL");

    CHKRightType (AP_NAME (arg_node), arg_node, "STRING",
                  "attribute AP_NAME hasnt the right type");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKarg");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_ACTCHN (arg_node), arg_node,
                       "mandatory attribute ARG_ACTCHN is NULL");

    CHKRightType (ARG_ACTCHN (arg_node), arg_node, "LINK",
                  "attribute ARG_ACTCHN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_ALIAS (arg_node), arg_node,
                       "mandatory attribute ARG_ALIAS is NULL");

    CHKRightType (ARG_ALIAS (arg_node), arg_node, "BOOL",
                  "attribute ARG_ALIAS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_ATTRIB (arg_node), arg_node,
                       "mandatory attribute ARG_ATTRIB is NULL");

    CHKRightType (ARG_ATTRIB (arg_node), arg_node, "STATUSTYPE",
                  "attribute ARG_ATTRIB hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_AVIS (arg_node), arg_node,
                       "mandatory attribute ARG_AVIS is NULL");

    CHKRightType (ARG_AVIS (arg_node), arg_node, "NODE",
                  "attribute ARG_AVIS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_COLCHN (arg_node), arg_node,
                       "mandatory attribute ARG_COLCHN is NULL");

    CHKRightType (ARG_COLCHN (arg_node), arg_node, "LINK",
                  "attribute ARG_COLCHN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_FUNDEF (arg_node), arg_node,
                       "mandatory attribute ARG_FUNDEF is NULL");

    CHKRightType (ARG_FUNDEF (arg_node), arg_node, "LINK",
                  "attribute ARG_FUNDEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_NAIVE_REFCNT (arg_node), arg_node,
                       "mandatory attribute ARG_NAIVE_REFCNT is NULL");

    CHKRightType (ARG_NAIVE_REFCNT (arg_node), arg_node, "INTEGER",
                  "attribute ARG_NAIVE_REFCNT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_NAME (arg_node), arg_node,
                       "mandatory attribute ARG_NAME is NULL");

    CHKRightType (ARG_NAME (arg_node), arg_node, "STRING",
                  "attribute ARG_NAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_OBJDEF (arg_node), arg_node,
                       "mandatory attribute ARG_OBJDEF is NULL");

    CHKRightType (ARG_OBJDEF (arg_node), arg_node, "LINK",
                  "attribute ARG_OBJDEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_PADDED (arg_node), arg_node,
                       "mandatory attribute ARG_PADDED is NULL");

    CHKRightType (ARG_PADDED (arg_node), arg_node, "BOOL",
                  "attribute ARG_PADDED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_REFCNT (arg_node), arg_node,
                       "mandatory attribute ARG_REFCNT is NULL");

    CHKRightType (ARG_REFCNT (arg_node), arg_node, "INTEGER",
                  "attribute ARG_REFCNT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_STATUS (arg_node), arg_node,
                       "mandatory attribute ARG_STATUS is NULL");

    CHKRightType (ARG_STATUS (arg_node), arg_node, "STATUSTYPE",
                  "attribute ARG_STATUS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_TYPE (arg_node), arg_node,
                       "mandatory attribute ARG_TYPE is NULL");

    CHKRightType (ARG_TYPE (arg_node), arg_node, "OLDTYPE",
                  "attribute ARG_TYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_TYPESTRING (arg_node), arg_node,
                       "mandatory attribute ARG_TYPESTRING is NULL");

    CHKRightType (ARG_TYPESTRING (arg_node), arg_node, "STRING",
                  "attribute ARG_TYPESTRING hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARG_VARNO (arg_node), arg_node,
                       "mandatory attribute ARG_VARNO is NULL");

    CHKRightType (ARG_VARNO (arg_node), arg_node, "INTEGER",
                  "attribute ARG_VARNO hasnt the right type");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKarray");

    // this son is mandatory = yes
    CHKExistChild (ARRAY_AELEMS (arg_node), arg_node,
                   "mandatory son ARRAY_AELEMS is NULL");

    CHKRightType (ARRAY_CONSTVEC (arg_node), arg_node, "CONSTVECPOINTER",
                  "attribute ARRAY_CONSTVEC hasnt the right type");

    CHKRightType (ARRAY_ISCONST (arg_node), arg_node, "BOOL",
                  "attribute ARRAY_ISCONST hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARRAY_NTYPE (arg_node), arg_node,
                       "mandatory attribute ARRAY_NTYPE is NULL");

    CHKRightType (ARRAY_NTYPE (arg_node), arg_node, "NEWTYPE",
                  "attribute ARRAY_NTYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARRAY_SHAPE (arg_node), arg_node,
                       "mandatory attribute ARRAY_SHAPE is NULL");

    CHKRightType (ARRAY_SHAPE (arg_node), arg_node, "SHAPE",
                  "attribute ARRAY_SHAPE hasnt the right type");

    CHKRightType (ARRAY_STRING (arg_node), arg_node, "STRING",
                  "attribute ARRAY_STRING hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ARRAY_TYPE (arg_node), arg_node,
                       "mandatory attribute ARRAY_TYPE is NULL");

    CHKRightType (ARRAY_TYPE (arg_node), arg_node, "OLDTYPE",
                  "attribute ARRAY_TYPE hasnt the right type");

    CHKRightType (ARRAY_VECLEN (arg_node), arg_node, "INTEGER",
                  "attribute ARRAY_VECLEN hasnt the right type");

    CHKRightType (ARRAY_VECTYPE (arg_node), arg_node, "SIMPLETYPE",
                  "attribute ARRAY_VECTYPE hasnt the right type");

    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKassign");

    // this son is mandatory = yes
    CHKExistChild (ASSIGN_INSTR (arg_node), arg_node,
                   "mandatory son ASSIGN_INSTR is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (ASSIGN_CELLID (arg_node), arg_node,
                       "mandatory attribute ASSIGN_CELLID is NULL");

    CHKRightType (ASSIGN_CELLID (arg_node), arg_node, "INTEGER",
                  "attribute ASSIGN_CELLID hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ASSIGN_CF (arg_node), arg_node,
                       "mandatory attribute ASSIGN_CF is NULL");

    CHKRightType (ASSIGN_CF (arg_node), arg_node, "LINK",
                  "attribute ASSIGN_CF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ASSIGN_CSE (arg_node), arg_node,
                       "mandatory attribute ASSIGN_CSE is NULL");

    CHKRightType (ASSIGN_CSE (arg_node), arg_node, "LINK",
                  "attribute ASSIGN_CSE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ASSIGN_EXECMODE (arg_node), arg_node,
                       "mandatory attribute ASSIGN_EXECMODE is NULL");

    CHKRightType (ASSIGN_EXECMODE (arg_node), arg_node, "INTEGER",
                  "attribute ASSIGN_EXECMODE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ASSIGN_INDENT (arg_node), arg_node,
                       "mandatory attribute ASSIGN_INDENT is NULL");

    CHKRightType (ASSIGN_INDENT (arg_node), arg_node, "INTEGER",
                  "attribute ASSIGN_INDENT hasnt the right type");

    CHKRightType (ASSIGN_INDEX (arg_node), arg_node, "INDEXPOINTER",
                  "attribute ASSIGN_INDEX hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ASSIGN_LEVEL (arg_node), arg_node,
                       "mandatory attribute ASSIGN_LEVEL is NULL");

    CHKRightType (ASSIGN_LEVEL (arg_node), arg_node, "INTEGER",
                  "attribute ASSIGN_LEVEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ASSIGN_STATUS (arg_node), arg_node,
                       "mandatory attribute ASSIGN_STATUS is NULL");

    CHKRightType (ASSIGN_STATUS (arg_node), arg_node, "INTEGER",
                  "attribute ASSIGN_STATUS hasnt the right type");

    CHKRightType (ASSIGN_TAG (arg_node), arg_node, "LINK",
                  "attribute ASSIGN_TAG hasnt the right type");

    CHKRightType (ASSIGN_VISITED_WITH (arg_node), arg_node, "LINK",
                  "attribute ASSIGN_VISITED_WITH hasnt the right type");

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKavis");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_ALIAS (arg_node), arg_node,
                       "mandatory attribute AVIS_ALIAS is NULL");

    CHKRightType (AVIS_ALIAS (arg_node), arg_node, "BOOL",
                  "attribute AVIS_ALIAS hasnt the right type");

    CHKRightType (AVIS_ALIASMASK (arg_node), arg_node, "DFMMASK",
                  "attribute AVIS_ALIASMASK hasnt the right type");

    CHKRightType (AVIS_DEFDEPTH (arg_node), arg_node, "INTEGER",
                  "attribute AVIS_DEFDEPTH hasnt the right type");

    CHKRightType (AVIS_EMRC_COUNTER (arg_node), arg_node, "RCCOUNTER",
                  "attribute AVIS_EMRC_COUNTER hasnt the right type");

    CHKRightType (AVIS_EMRC_COUNTER2 (arg_node), arg_node, "RCCOUNTER",
                  "attribute AVIS_EMRC_COUNTER2 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_EMRC_DEFLEVEL (arg_node), arg_node,
                       "mandatory attribute AVIS_EMRC_DEFLEVEL is NULL");

    CHKRightType (AVIS_EMRC_DEFLEVEL (arg_node), arg_node, "INTEGER",
                  "attribute AVIS_EMRC_DEFLEVEL hasnt the right type");

    CHKRightType (AVIS_EXPRESULT (arg_node), arg_node, "BOOL",
                  "attribute AVIS_EXPRESULT hasnt the right type");

    CHKRightType (AVIS_LIRMOVE (arg_node), arg_node, "BITFIELD",
                  "attribute AVIS_LIRMOVE hasnt the right type");

    CHKRightType (AVIS_NEEDCOUNT (arg_node), arg_node, "BOOL",
                  "attribute AVIS_NEEDCOUNT hasnt the right type");

    CHKRightType (AVIS_SELPROP (arg_node), arg_node, "INTEGER",
                  "attribute AVIS_SELPROP hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_SSAASSIGN (arg_node), arg_node,
                       "mandatory attribute AVIS_SSAASSIGN is NULL");

    CHKRightType (AVIS_SSAASSIGN (arg_node), arg_node, "DOWNLINK",
                  "attribute AVIS_SSAASSIGN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_SSAASSIGN2 (arg_node), arg_node,
                       "mandatory attribute AVIS_SSAASSIGN2 is NULL");

    CHKRightType (AVIS_SSAASSIGN2 (arg_node), arg_node, "DOWNLINK",
                  "attribute AVIS_SSAASSIGN2 hasnt the right type");

    CHKRightType (AVIS_SSACONST (arg_node), arg_node, "CONSTANT",
                  "attribute AVIS_SSACONST hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_SSACOUNT (arg_node), arg_node,
                       "mandatory attribute AVIS_SSACOUNT is NULL");

    CHKRightType (AVIS_SSACOUNT (arg_node), arg_node, "LINK",
                  "attribute AVIS_SSACOUNT hasnt the right type");

    CHKRightType (AVIS_SSADEFINED (arg_node), arg_node, "BOOL",
                  "attribute AVIS_SSADEFINED hasnt the right type");

    CHKRightType (AVIS_SSAELSE (arg_node), arg_node, "DOWNLINK",
                  "attribute AVIS_SSAELSE hasnt the right type");

    CHKRightType (AVIS_SSALPINV (arg_node), arg_node, "BOOL",
                  "attribute AVIS_SSALPINV hasnt the right type");

    CHKRightType (AVIS_SSAPHITARGET (arg_node), arg_node, "SSAPHI",
                  "attribute AVIS_SSAPHITARGET hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_SSASTACK (arg_node), arg_node,
                       "mandatory attribute AVIS_SSASTACK is NULL");

    CHKRightType (AVIS_SSASTACK (arg_node), arg_node, "NODE",
                  "attribute AVIS_SSASTACK hasnt the right type");

    CHKRightType (AVIS_SSATHEN (arg_node), arg_node, "DOWNLINK",
                  "attribute AVIS_SSATHEN hasnt the right type");

    CHKRightType (AVIS_SSAUNDOFLAG (arg_node), arg_node, "BOOL",
                  "attribute AVIS_SSAUNDOFLAG hasnt the right type");

    CHKRightType (AVIS_SUBST (arg_node), arg_node, "DOWNLINK",
                  "attribute AVIS_SUBST hasnt the right type");

    CHKRightType (AVIS_SUBSTUSSA (arg_node), arg_node, "DOWNLINK",
                  "attribute AVIS_SUBSTUSSA hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_TYPE (arg_node), arg_node,
                       "mandatory attribute AVIS_TYPE is NULL");

    CHKRightType (AVIS_TYPE (arg_node), arg_node, "NEWTYPE",
                  "attribute AVIS_TYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (AVIS_VARDECORARG (arg_node), arg_node,
                       "mandatory attribute AVIS_VARDECORARG is NULL");

    CHKRightType (AVIS_VARDECORARG (arg_node), arg_node, "LINK",
                  "attribute AVIS_VARDECORARG hasnt the right type");

    CHKRightType (AVIS_WITHID (arg_node), arg_node, "LINK",
                  "attribute AVIS_WITHID hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKblock");

    // this son is mandatory = yes
    CHKExistChild (BLOCK_INSTR (arg_node), arg_node, "mandatory son BLOCK_INSTR is NULL");

    // this son is mandatory = yes
    CHKExistChild (BLOCK_VARDEC (arg_node), arg_node,
                   "mandatory son BLOCK_VARDEC is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_CACHESIM (arg_node), arg_node,
                       "mandatory attribute BLOCK_CACHESIM is NULL");

    CHKRightType (BLOCK_CACHESIM (arg_node), arg_node, "STRING",
                  "attribute BLOCK_CACHESIM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_NEEDFUNS (arg_node), arg_node,
                       "mandatory attribute BLOCK_NEEDFUNS is NULL");

    CHKRightType (BLOCK_NEEDFUNS (arg_node), arg_node, "NODELIST",
                  "attribute BLOCK_NEEDFUNS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_NEEDTYPES (arg_node), arg_node,
                       "mandatory attribute BLOCK_NEEDTYPES is NULL");

    CHKRightType (BLOCK_NEEDTYPES (arg_node), arg_node, "NODELIST",
                  "attribute BLOCK_NEEDTYPES hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_SCHEDULER_INIT (arg_node), arg_node,
                       "mandatory attribute BLOCK_SCHEDULER_INIT is NULL");

    CHKRightType (BLOCK_SCHEDULER_INIT (arg_node), arg_node, "LINK",
                  "attribute BLOCK_SCHEDULER_INIT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_SPMD_PROLOG_ICMS (arg_node), arg_node,
                       "mandatory attribute BLOCK_SPMD_PROLOG_ICMS is NULL");

    CHKRightType (BLOCK_SPMD_PROLOG_ICMS (arg_node), arg_node, "LINK",
                  "attribute BLOCK_SPMD_PROLOG_ICMS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_SPMD_SETUP_ARGS (arg_node), arg_node,
                       "mandatory attribute BLOCK_SPMD_SETUP_ARGS is NULL");

    CHKRightType (BLOCK_SPMD_SETUP_ARGS (arg_node), arg_node, "LINK",
                  "attribute BLOCK_SPMD_SETUP_ARGS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_SSACOUNTER (arg_node), arg_node,
                       "mandatory attribute BLOCK_SSACOUNTER is NULL");

    CHKRightType (BLOCK_SSACOUNTER (arg_node), arg_node, "NODE",
                  "attribute BLOCK_SSACOUNTER hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (BLOCK_VARNO (arg_node), arg_node,
                       "mandatory attribute BLOCK_VARNO is NULL");

    CHKRightType (BLOCK_VARNO (arg_node), arg_node, "INTEGER",
                  "attribute BLOCK_VARNO hasnt the right type");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKbool (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKbool");

    // this attribute is mandatory = yes
    CHKExistAttribute (BOOL_VAL (arg_node), arg_node,
                       "mandatory attribute BOOL_VAL is NULL");

    CHKRightType (BOOL_VAL (arg_node), arg_node, "BOOL",
                  "attribute BOOL_VAL hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKcast");

    // this son is mandatory = yes
    CHKExistChild (CAST_EXPR (arg_node), arg_node, "mandatory son CAST_EXPR is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (CAST_TYPE (arg_node), arg_node,
                       "mandatory attribute CAST_TYPE is NULL");

    CHKRightType (CAST_TYPE (arg_node), arg_node, "OLDTYPE",
                  "attribute CAST_TYPE hasnt the right type");

    if (CAST_EXPR (arg_node) != NULL) {
        CAST_EXPR (arg_node) = Trav (CAST_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKchar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKchar");

    // this attribute is mandatory = yes
    CHKExistAttribute (CHAR_VAL (arg_node), arg_node,
                       "mandatory attribute CHAR_VAL is NULL");

    CHKRightType (CHAR_VAL (arg_node), arg_node, "CHAR",
                  "attribute CHAR_VAL hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKcond");

    // this son is mandatory = yes
    CHKExistChild (COND_COND (arg_node), arg_node, "mandatory son COND_COND is NULL");

    // this son is mandatory = yes
    CHKExistChild (COND_ELSE (arg_node), arg_node, "mandatory son COND_ELSE is NULL");

    // this son is mandatory = yes
    CHKExistChild (COND_THEN (arg_node), arg_node, "mandatory son COND_THEN is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (COND_ELSEVARS (arg_node), arg_node,
                       "mandatory attribute COND_ELSEVARS is NULL");

    CHKRightType (COND_ELSEVARS (arg_node), arg_node, "IDS",
                  "attribute COND_ELSEVARS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (COND_IN_MASK (arg_node), arg_node,
                       "mandatory attribute COND_IN_MASK is NULL");

    CHKRightType (COND_IN_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute COND_IN_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (COND_LOCAL_MASK (arg_node), arg_node,
                       "mandatory attribute COND_LOCAL_MASK is NULL");

    CHKRightType (COND_LOCAL_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute COND_LOCAL_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (COND_OUT_MASK (arg_node), arg_node,
                       "mandatory attribute COND_OUT_MASK is NULL");

    CHKRightType (COND_OUT_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute COND_OUT_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (COND_THENVARS (arg_node), arg_node,
                       "mandatory attribute COND_THENVARS is NULL");

    CHKRightType (COND_THENVARS (arg_node), arg_node, "IDS",
                  "attribute COND_THENVARS hasnt the right type");

    if (COND_COND (arg_node) != NULL) {
        COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKcseinfo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKcseinfo");

    // this attribute is mandatory = yes
    CHKExistAttribute (CSEINFO_LAYER (arg_node), arg_node,
                       "mandatory attribute CSEINFO_LAYER is NULL");

    CHKRightType (CSEINFO_LAYER (arg_node), arg_node, "LINK",
                  "attribute CSEINFO_LAYER hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (CSEINFO_LET (arg_node), arg_node,
                       "mandatory attribute CSEINFO_LET is NULL");

    CHKRightType (CSEINFO_LET (arg_node), arg_node, "LINK",
                  "attribute CSEINFO_LET hasnt the right type");

    if (CSEINFO_NEXT (arg_node) != NULL) {
        CSEINFO_NEXT (arg_node) = Trav (CSEINFO_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKcwrapper (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKcwrapper");

    // this attribute is mandatory = yes
    CHKExistAttribute (CWRAPPER_ARGCOUNT (arg_node), arg_node,
                       "mandatory attribute CWRAPPER_ARGCOUNT is NULL");

    CHKRightType (CWRAPPER_ARGCOUNT (arg_node), arg_node, "INTEGER",
                  "attribute CWRAPPER_ARGCOUNT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (CWRAPPER_FUNS (arg_node), arg_node,
                       "mandatory attribute CWRAPPER_FUNS is NULL");

    CHKRightType (CWRAPPER_FUNS (arg_node), arg_node, "NODELIST",
                  "attribute CWRAPPER_FUNS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (CWRAPPER_MOD (arg_node), arg_node,
                       "mandatory attribute CWRAPPER_MOD is NULL");

    CHKRightType (CWRAPPER_MOD (arg_node), arg_node, "SHAREDSTRING",
                  "attribute CWRAPPER_MOD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (CWRAPPER_NAME (arg_node), arg_node,
                       "mandatory attribute CWRAPPER_NAME is NULL");

    CHKRightType (CWRAPPER_NAME (arg_node), arg_node, "STRING",
                  "attribute CWRAPPER_NAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (CWRAPPER_RESCOUNT (arg_node), arg_node,
                       "mandatory attribute CWRAPPER_RESCOUNT is NULL");

    CHKRightType (CWRAPPER_RESCOUNT (arg_node), arg_node, "INTEGER",
                  "attribute CWRAPPER_RESCOUNT hasnt the right type");

    if (CWRAPPER_NEXT (arg_node) != NULL) {
        CWRAPPER_NEXT (arg_node) = Trav (CWRAPPER_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKdataflowgraph (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKdataflowgraph");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWGRAPH_MEMBERS (arg_node), arg_node,
                       "mandatory attribute DATAFLOWGRAPH_MEMBERS is NULL");

    CHKRightType (DATAFLOWGRAPH_MEMBERS (arg_node), arg_node, "NODELIST",
                  "attribute DATAFLOWGRAPH_MEMBERS hasnt the right type");

    CHKRightType (DATAFLOWGRAPH_MYHOMEDFN (arg_node), arg_node, "NODE",
                  "attribute DATAFLOWGRAPH_MYHOMEDFN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWGRAPH_SINK (arg_node), arg_node,
                       "mandatory attribute DATAFLOWGRAPH_SINK is NULL");

    CHKRightType (DATAFLOWGRAPH_SINK (arg_node), arg_node, "NODE",
                  "attribute DATAFLOWGRAPH_SINK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWGRAPH_SOURCE (arg_node), arg_node,
                       "mandatory attribute DATAFLOWGRAPH_SOURCE is NULL");

    CHKRightType (DATAFLOWGRAPH_SOURCE (arg_node), arg_node, "NODE",
                  "attribute DATAFLOWGRAPH_SOURCE hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKdataflownode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKdataflownode");

    CHKRightType (DATAFLOWNODE_ASSIGN (arg_node), arg_node, "LINK",
                  "attribute DATAFLOWNODE_ASSIGN hasnt the right type");

    CHKRightType (DATAFLOWNODE_DEPENDENT (arg_node), arg_node, "NODELIST",
                  "attribute DATAFLOWNODE_DEPENDENT hasnt the right type");

    CHKRightType (DATAFLOWNODE_DFGELSE (arg_node), arg_node, "LINK",
                  "attribute DATAFLOWNODE_DFGELSE hasnt the right type");

    CHKRightType (DATAFLOWNODE_DFGTHEN (arg_node), arg_node, "LINK",
                  "attribute DATAFLOWNODE_DFGTHEN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWNODE_EXECMODE (arg_node), arg_node,
                       "mandatory attribute DATAFLOWNODE_EXECMODE is NULL");

    CHKRightType (DATAFLOWNODE_EXECMODE (arg_node), arg_node, "MTEXECMODE",
                  "attribute DATAFLOWNODE_EXECMODE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWNODE_GRAPH (arg_node), arg_node,
                       "mandatory attribute DATAFLOWNODE_GRAPH is NULL");

    CHKRightType (DATAFLOWNODE_GRAPH (arg_node), arg_node, "LINK",
                  "attribute DATAFLOWNODE_GRAPH hasnt the right type");

    CHKRightType (DATAFLOWNODE_NAME (arg_node), arg_node, "STRING",
                  "attribute DATAFLOWNODE_NAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWNODE_REFCOUNT (arg_node), arg_node,
                       "mandatory attribute DATAFLOWNODE_REFCOUNT is NULL");

    CHKRightType (DATAFLOWNODE_REFCOUNT (arg_node), arg_node, "INTEGER",
                  "attribute DATAFLOWNODE_REFCOUNT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWNODE_REFLEFT (arg_node), arg_node,
                       "mandatory attribute DATAFLOWNODE_REFLEFT is NULL");

    CHKRightType (DATAFLOWNODE_REFLEFT (arg_node), arg_node, "INTEGER",
                  "attribute DATAFLOWNODE_REFLEFT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DATAFLOWNODE_USED (arg_node), arg_node,
                       "mandatory attribute DATAFLOWNODE_USED is NULL");

    CHKRightType (DATAFLOWNODE_USED (arg_node), arg_node, "BOOL",
                  "attribute DATAFLOWNODE_USED hasnt the right type");

    CHKRightType (DATAFLOWNODE_USEDNODES (arg_node), arg_node, "NODELIST",
                  "attribute DATAFLOWNODE_USEDNODES hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKdo");

    // this son is mandatory = yes
    CHKExistChild (DO_BODY (arg_node), arg_node, "mandatory son DO_BODY is NULL");

    // this son is mandatory = yes
    CHKExistChild (DO_COND (arg_node), arg_node, "mandatory son DO_COND is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (DO_DEFVARS (arg_node), arg_node,
                       "mandatory attribute DO_DEFVARS is NULL");

    CHKRightType (DO_DEFVARS (arg_node), arg_node, "IDS",
                  "attribute DO_DEFVARS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DO_IN_MASK (arg_node), arg_node,
                       "mandatory attribute DO_IN_MASK is NULL");

    CHKRightType (DO_IN_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute DO_IN_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DO_LABEL (arg_node), arg_node,
                       "mandatory attribute DO_LABEL is NULL");

    CHKRightType (DO_LABEL (arg_node), arg_node, "STRING",
                  "attribute DO_LABEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DO_LOCAL_MASK (arg_node), arg_node,
                       "mandatory attribute DO_LOCAL_MASK is NULL");

    CHKRightType (DO_LOCAL_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute DO_LOCAL_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DO_OUT_MASK (arg_node), arg_node,
                       "mandatory attribute DO_OUT_MASK is NULL");

    CHKRightType (DO_OUT_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute DO_OUT_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DO_SKIP (arg_node), arg_node,
                       "mandatory attribute DO_SKIP is NULL");

    CHKRightType (DO_SKIP (arg_node), arg_node, "LINK",
                  "attribute DO_SKIP hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (DO_USEVARS (arg_node), arg_node,
                       "mandatory attribute DO_USEVARS is NULL");

    CHKRightType (DO_USEVARS (arg_node), arg_node, "IDS",
                  "attribute DO_USEVARS hasnt the right type");

    if (DO_BODY (arg_node) != NULL) {
        DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);
    }

    if (DO_COND (arg_node) != NULL) {
        DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKdot (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKdot");

    // this attribute is mandatory = yes
    CHKExistAttribute (DOT_NUM (arg_node), arg_node,
                       "mandatory attribute DOT_NUM is NULL");

    CHKRightType (DOT_NUM (arg_node), arg_node, "INTEGER",
                  "attribute DOT_NUM hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKdouble (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKdouble");

    // this attribute is mandatory = yes
    CHKExistAttribute (DOUBLE_VAL (arg_node), arg_node,
                       "mandatory attribute DOUBLE_VAL is NULL");

    CHKRightType (DOUBLE_VAL (arg_node), arg_node, "DOUBLE",
                  "attribute DOUBLE_VAL hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKempty (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKempty");

    DBUG_RETURN (arg_node);
}

node *
CHKex (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKex");

    // this son is mandatory = yes
    CHKExistChild (EX_REGION (arg_node), arg_node, "mandatory son EX_REGION is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (EX_DEFMASK (arg_node), arg_node,
                       "mandatory attribute EX_DEFMASK is NULL");

    CHKRightType (EX_DEFMASK (arg_node), arg_node, "DFMMASK",
                  "attribute EX_DEFMASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (EX_NEEDLATER (arg_node), arg_node,
                       "mandatory attribute EX_NEEDLATER is NULL");

    CHKRightType (EX_NEEDLATER (arg_node), arg_node, "DFMMASK",
                  "attribute EX_NEEDLATER hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (EX_USEMASK (arg_node), arg_node,
                       "mandatory attribute EX_USEMASK is NULL");

    CHKRightType (EX_USEMASK (arg_node), arg_node, "DFMMASK",
                  "attribute EX_USEMASK hasnt the right type");

    if (EX_REGION (arg_node) != NULL) {
        EX_REGION (arg_node) = Trav (EX_REGION (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKexport");

    // this attribute is mandatory = yes
    CHKExistAttribute (EXPORT_ALL (arg_node), arg_node,
                       "mandatory attribute EXPORT_ALL is NULL");

    CHKRightType (EXPORT_ALL (arg_node), arg_node, "BOOL",
                  "attribute EXPORT_ALL hasnt the right type");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = Trav (EXPORT_NEXT (arg_node), arg_info);
    }

    if (EXPORT_SYMBOL (arg_node) != NULL) {
        EXPORT_SYMBOL (arg_node) = Trav (EXPORT_SYMBOL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKexprs");

    // this son is mandatory = yes
    CHKExistChild (EXPRS_EXPR (arg_node), arg_node, "mandatory son EXPRS_EXPR is NULL");

    if (EXPRS_EXPR (arg_node) != NULL) {
        EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);
    }

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKfloat (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKfloat");

    // this attribute is mandatory = yes
    CHKExistAttribute (FLOAT_VAL (arg_node), arg_node,
                       "mandatory attribute FLOAT_VAL is NULL");

    CHKRightType (FLOAT_VAL (arg_node), arg_node, "FLOAT",
                  "attribute FLOAT_VAL hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKfuncond");

    // this son is mandatory = yes
    CHKExistChild (FUNCOND_ELSE (arg_node), arg_node,
                   "mandatory son FUNCOND_ELSE is NULL");

    // this son is mandatory = yes
    CHKExistChild (FUNCOND_IF (arg_node), arg_node, "mandatory son FUNCOND_IF is NULL");

    // this son is mandatory = yes
    CHKExistChild (FUNCOND_THEN (arg_node), arg_node,
                   "mandatory son FUNCOND_THEN is NULL");

    if (FUNCOND_ELSE (arg_node) != NULL) {
        FUNCOND_ELSE (arg_node) = Trav (FUNCOND_ELSE (arg_node), arg_info);
    }

    if (FUNCOND_IF (arg_node) != NULL) {
        FUNCOND_IF (arg_node) = Trav (FUNCOND_IF (arg_node), arg_info);
    }

    if (FUNCOND_THEN (arg_node) != NULL) {
        FUNCOND_THEN (arg_node) = Trav (FUNCOND_THEN (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKfundef");

    // this son is mandatory = yes
    CHKExistChild (FUNDEF_BODY (arg_node), arg_node, "mandatory son FUNDEF_BODY is NULL");

    // this son is mandatory = yes
    CHKExistChild (FUNDEF_NEXT (arg_node), arg_node, "mandatory son FUNDEF_NEXT is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_ARGTAB (arg_node), arg_node,
                       "mandatory attribute FUNDEF_ARGTAB is NULL");

    CHKRightType (FUNDEF_ARGTAB (arg_node), arg_node, "ARGTAB",
                  "attribute FUNDEF_ARGTAB hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_ATTRIB (arg_node), arg_node,
                       "mandatory attribute FUNDEF_ATTRIB is NULL");

    CHKRightType (FUNDEF_ATTRIB (arg_node), arg_node, "STATUSTYPE",
                  "attribute FUNDEF_ATTRIB hasnt the right type");

    CHKRightType (FUNDEF_COMPANION (arg_node), arg_node, "LINK",
                  "attribute FUNDEF_COMPANION hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_DFM_BASE (arg_node), arg_node,
                       "mandatory attribute FUNDEF_DFM_BASE is NULL");

    CHKRightType (FUNDEF_DFM_BASE (arg_node), arg_node, "DFMMASKBASE",
                  "attribute FUNDEF_DFM_BASE hasnt the right type");

    CHKRightType (FUNDEF_EXECMODE (arg_node), arg_node, "MTEXECMODE",
                  "attribute FUNDEF_EXECMODE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_EXPORT (arg_node), arg_node,
                       "mandatory attribute FUNDEF_EXPORT is NULL");

    CHKRightType (FUNDEF_EXPORT (arg_node), arg_node, "BOOL",
                  "attribute FUNDEF_EXPORT hasnt the right type");

    CHKRightType (FUNDEF_EXT_ASSIGNS (arg_node), arg_node, "NODELIST",
                  "attribute FUNDEF_EXT_ASSIGNS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_FUNNO (arg_node), arg_node,
                       "mandatory attribute FUNDEF_FUNNO is NULL");

    CHKRightType (FUNDEF_FUNNO (arg_node), arg_node, "INTEGER",
                  "attribute FUNDEF_FUNNO hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_ICM (arg_node), arg_node,
                       "mandatory attribute FUNDEF_ICM is NULL");

    CHKRightType (FUNDEF_ICM (arg_node), arg_node, "NODE",
                  "attribute FUNDEF_ICM hasnt the right type");

    CHKRightType (FUNDEF_IMPL (arg_node), arg_node, "EXTLINK",
                  "attribute FUNDEF_IMPL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_INFIX (arg_node), arg_node,
                       "mandatory attribute FUNDEF_INFIX is NULL");

    CHKRightType (FUNDEF_INFIX (arg_node), arg_node, "BOOL",
                  "attribute FUNDEF_INFIX hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_INLINE (arg_node), arg_node,
                       "mandatory attribute FUNDEF_INLINE is NULL");

    CHKRightType (FUNDEF_INLINE (arg_node), arg_node, "BOOL",
                  "attribute FUNDEF_INLINE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_INLREC (arg_node), arg_node,
                       "mandatory attribute FUNDEF_INLREC is NULL");

    CHKRightType (FUNDEF_INLREC (arg_node), arg_node, "INTEGER",
                  "attribute FUNDEF_INLREC hasnt the right type");

    CHKRightType (FUNDEF_INT_ASSIGN (arg_node), arg_node, "LINK",
                  "attribute FUNDEF_INT_ASSIGN hasnt the right type");

    CHKRightType (FUNDEF_LIFTEDFROM (arg_node), arg_node, "LINK",
                  "attribute FUNDEF_LIFTEDFROM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_MOD (arg_node), arg_node,
                       "mandatory attribute FUNDEF_MOD is NULL");

    CHKRightType (FUNDEF_MOD (arg_node), arg_node, "SHAREDSTRING",
                  "attribute FUNDEF_MOD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_NAME (arg_node), arg_node,
                       "mandatory attribute FUNDEF_NAME is NULL");

    CHKRightType (FUNDEF_NAME (arg_node), arg_node, "STRING",
                  "attribute FUNDEF_NAME hasnt the right type");

    CHKRightType (FUNDEF_NEEDOBJS (arg_node), arg_node, "NODELIST",
                  "attribute FUNDEF_NEEDOBJS hasnt the right type");

    CHKRightType (FUNDEF_PRAGMA (arg_node), arg_node, "NODE",
                  "attribute FUNDEF_PRAGMA hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_RET_TYPE (arg_node), arg_node,
                       "mandatory attribute FUNDEF_RET_TYPE is NULL");

    CHKRightType (FUNDEF_RET_TYPE (arg_node), arg_node, "NEWTYPE",
                  "attribute FUNDEF_RET_TYPE hasnt the right type");

    CHKRightType (FUNDEF_RETALIAS (arg_node), arg_node, "NODELIST",
                  "attribute FUNDEF_RETALIAS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_RETURN (arg_node), arg_node,
                       "mandatory attribute FUNDEF_RETURN is NULL");

    CHKRightType (FUNDEF_RETURN (arg_node), arg_node, "DOWNLINK",
                  "attribute FUNDEF_RETURN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_SIB (arg_node), arg_node,
                       "mandatory attribute FUNDEF_SIB is NULL");

    CHKRightType (FUNDEF_SIB (arg_node), arg_node, "NODE",
                  "attribute FUNDEF_SIB hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_SPECS (arg_node), arg_node,
                       "mandatory attribute FUNDEF_SPECS is NULL");

    CHKRightType (FUNDEF_SPECS (arg_node), arg_node, "INTEGER",
                  "attribute FUNDEF_SPECS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_STATUS (arg_node), arg_node,
                       "mandatory attribute FUNDEF_STATUS is NULL");

    CHKRightType (FUNDEF_STATUS (arg_node), arg_node, "STATUSTYPE",
                  "attribute FUNDEF_STATUS hasnt the right type");

    CHKRightType (FUNDEF_SYMBOLNAME (arg_node), arg_node, "STRING",
                  "attribute FUNDEF_SYMBOLNAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_TCSTAT (arg_node), arg_node,
                       "mandatory attribute FUNDEF_TCSTAT is NULL");

    CHKRightType (FUNDEF_TCSTAT (arg_node), arg_node, "INTEGER",
                  "attribute FUNDEF_TCSTAT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_TYPE (arg_node), arg_node,
                       "mandatory attribute FUNDEF_TYPE is NULL");

    CHKRightType (FUNDEF_TYPE (arg_node), arg_node, "NEWTYPE",
                  "attribute FUNDEF_TYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_TYPES (arg_node), arg_node,
                       "mandatory attribute FUNDEF_TYPES is NULL");

    CHKRightType (FUNDEF_TYPES (arg_node), arg_node, "OLDTYPE",
                  "attribute FUNDEF_TYPES hasnt the right type");

    CHKRightType (FUNDEF_USED (arg_node), arg_node, "INTEGER",
                  "attribute FUNDEF_USED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (FUNDEF_VARNO (arg_node), arg_node,
                       "mandatory attribute FUNDEF_VARNO is NULL");

    CHKRightType (FUNDEF_VARNO (arg_node), arg_node, "INTEGER",
                  "attribute FUNDEF_VARNO hasnt the right type");

    CHKRightType (FUNDEF_WORKER (arg_node), arg_node, "LINK",
                  "attribute FUNDEF_WORKER hasnt the right type");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKicm (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKicm");

    // this attribute is mandatory = yes
    CHKExistAttribute (ICM_END_OF_STATEMENT (arg_node), arg_node,
                       "mandatory attribute ICM_END_OF_STATEMENT is NULL");

    CHKRightType (ICM_END_OF_STATEMENT (arg_node), arg_node, "BOOL",
                  "attribute ICM_END_OF_STATEMENT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ICM_FUNDEF (arg_node), arg_node,
                       "mandatory attribute ICM_FUNDEF is NULL");

    CHKRightType (ICM_FUNDEF (arg_node), arg_node, "LINK",
                  "attribute ICM_FUNDEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ICM_INDENT_AFTER (arg_node), arg_node,
                       "mandatory attribute ICM_INDENT_AFTER is NULL");

    CHKRightType (ICM_INDENT_AFTER (arg_node), arg_node, "INTEGER",
                  "attribute ICM_INDENT_AFTER hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ICM_INDENT_BEFORE (arg_node), arg_node,
                       "mandatory attribute ICM_INDENT_BEFORE is NULL");

    CHKRightType (ICM_INDENT_BEFORE (arg_node), arg_node, "INTEGER",
                  "attribute ICM_INDENT_BEFORE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ICM_NAME (arg_node), arg_node,
                       "mandatory attribute ICM_NAME is NULL");

    CHKRightType (ICM_NAME (arg_node), arg_node, "SHAREDSTRING",
                  "attribute ICM_NAME hasnt the right type");

    if (ICM_ARGS (arg_node) != NULL) {
        ICM_ARGS (arg_node) = Trav (ICM_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKid");

    CHKRightType (ID_CONSTVEC (arg_node), arg_node, "CONSTVECPOINTER",
                  "attribute ID_CONSTVEC hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ID_IDS (arg_node), arg_node, "mandatory attribute ID_IDS is NULL");

    CHKRightType (ID_IDS (arg_node), arg_node, "IDS",
                  "attribute ID_IDS hasnt the right type");

    CHKRightType (ID_ISCONST (arg_node), arg_node, "INTEGER",
                  "attribute ID_ISCONST hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ID_NT_TAG (arg_node), arg_node,
                       "mandatory attribute ID_NT_TAG is NULL");

    CHKRightType (ID_NT_TAG (arg_node), arg_node, "STRING",
                  "attribute ID_NT_TAG hasnt the right type");

    CHKRightType (ID_NUM (arg_node), arg_node, "INTEGER",
                  "attribute ID_NUM hasnt the right type");

    CHKRightType (ID_VECLEN (arg_node), arg_node, "INTEGER",
                  "attribute ID_VECLEN hasnt the right type");

    CHKRightType (ID_VECTYPE (arg_node), arg_node, "SIMPLETYPE",
                  "attribute ID_VECTYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ID_WL (arg_node), arg_node, "mandatory attribute ID_WL is NULL");

    CHKRightType (ID_WL (arg_node), arg_node, "LINK",
                  "attribute ID_WL hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKimport");

    // this attribute is mandatory = yes
    CHKExistAttribute (IMPORT_ALL (arg_node), arg_node,
                       "mandatory attribute IMPORT_ALL is NULL");

    CHKRightType (IMPORT_ALL (arg_node), arg_node, "BOOL",
                  "attribute IMPORT_ALL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (IMPORT_MOD (arg_node), arg_node,
                       "mandatory attribute IMPORT_MOD is NULL");

    CHKRightType (IMPORT_MOD (arg_node), arg_node, "STRING",
                  "attribute IMPORT_MOD hasnt the right type");

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = Trav (IMPORT_NEXT (arg_node), arg_info);
    }

    if (IMPORT_SYMBOL (arg_node) != NULL) {
        IMPORT_SYMBOL (arg_node) = Trav (IMPORT_SYMBOL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKlet");

    // this son is mandatory = yes
    CHKExistChild (LET_EXPR (arg_node), arg_node, "mandatory son LET_EXPR is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (LET_DEFMASK (arg_node), arg_node,
                       "mandatory attribute LET_DEFMASK is NULL");

    CHKRightType (LET_DEFMASK (arg_node), arg_node, "DFMMASK",
                  "attribute LET_DEFMASK hasnt the right type");

    CHKRightType (LET_IDS (arg_node), arg_node, "IDS",
                  "attribute LET_IDS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (LET_LIRFLAG (arg_node), arg_node,
                       "mandatory attribute LET_LIRFLAG is NULL");

    CHKRightType (LET_LIRFLAG (arg_node), arg_node, "INTEGER",
                  "attribute LET_LIRFLAG hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (LET_USEMASK (arg_node), arg_node,
                       "mandatory attribute LET_USEMASK is NULL");

    CHKRightType (LET_USEMASK (arg_node), arg_node, "DFMMASK",
                  "attribute LET_USEMASK hasnt the right type");

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKmodul");

    CHKRightType (MODUL_CLASSTYPE (arg_node), arg_node, "OLDTYPE",
                  "attribute MODUL_CLASSTYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MODUL_DECL (arg_node), arg_node,
                       "mandatory attribute MODUL_DECL is NULL");

    CHKRightType (MODUL_DECL (arg_node), arg_node, "LINK",
                  "attribute MODUL_DECL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MODUL_DEPENDENCIES (arg_node), arg_node,
                       "mandatory attribute MODUL_DEPENDENCIES is NULL");

    CHKRightType (MODUL_DEPENDENCIES (arg_node), arg_node, "STRINGSET",
                  "attribute MODUL_DEPENDENCIES hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MODUL_FILETYPE (arg_node), arg_node,
                       "mandatory attribute MODUL_FILETYPE is NULL");

    CHKRightType (MODUL_FILETYPE (arg_node), arg_node, "FILETYPE",
                  "attribute MODUL_FILETYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MODUL_FOLDFUNS (arg_node), arg_node,
                       "mandatory attribute MODUL_FOLDFUNS is NULL");

    CHKRightType (MODUL_FOLDFUNS (arg_node), arg_node, "LINK",
                  "attribute MODUL_FOLDFUNS hasnt the right type");

    CHKRightType (MODUL_NAME (arg_node), arg_node, "SHAREDSTRING",
                  "attribute MODUL_NAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MODUL_WRAPPERFUNS (arg_node), arg_node,
                       "mandatory attribute MODUL_WRAPPERFUNS is NULL");

    CHKRightType (MODUL_WRAPPERFUNS (arg_node), arg_node, "LUT",
                  "attribute MODUL_WRAPPERFUNS hasnt the right type");

    if (MODUL_CWRAPPER (arg_node) != NULL) {
        MODUL_CWRAPPER (arg_node) = Trav (MODUL_CWRAPPER (arg_node), arg_info);
    }

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    if (MODUL_IMPORTS (arg_node) != NULL) {
        MODUL_IMPORTS (arg_node) = Trav (MODUL_IMPORTS (arg_node), arg_info);
    }

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKmop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKmop");

    // this attribute is mandatory = yes
    CHKExistAttribute (MOP_FIX (arg_node), arg_node,
                       "mandatory attribute MOP_FIX is NULL");

    CHKRightType (MOP_FIX (arg_node), arg_node, "BOOL",
                  "attribute MOP_FIX hasnt the right type");

    CHKRightType (MOP_OPS (arg_node), arg_node, "IDS",
                  "attribute MOP_OPS hasnt the right type");

    if (MOP_EXPRS (arg_node) != NULL) {
        MOP_EXPRS (arg_node) = Trav (MOP_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKmt (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKmt");

    // this son is mandatory = yes
    CHKExistChild (MT_REGION (arg_node), arg_node, "mandatory son MT_REGION is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (MT_ALLOC (arg_node), arg_node,
                       "mandatory attribute MT_ALLOC is NULL");

    CHKRightType (MT_ALLOC (arg_node), arg_node, "DFMMASK",
                  "attribute MT_ALLOC hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MT_DEFMASK (arg_node), arg_node,
                       "mandatory attribute MT_DEFMASK is NULL");

    CHKRightType (MT_DEFMASK (arg_node), arg_node, "DFMMASK",
                  "attribute MT_DEFMASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MT_FUNDEF (arg_node), arg_node,
                       "mandatory attribute MT_FUNDEF is NULL");

    CHKRightType (MT_FUNDEF (arg_node), arg_node, "EXTLINK",
                  "attribute MT_FUNDEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MT_NEEDLATER (arg_node), arg_node,
                       "mandatory attribute MT_NEEDLATER is NULL");

    CHKRightType (MT_NEEDLATER (arg_node), arg_node, "DFMMASK",
                  "attribute MT_NEEDLATER hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MT_USEMASK (arg_node), arg_node,
                       "mandatory attribute MT_USEMASK is NULL");

    CHKRightType (MT_USEMASK (arg_node), arg_node, "DFMMASK",
                  "attribute MT_USEMASK hasnt the right type");

    if (MT_REGION (arg_node) != NULL) {
        MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKmtalloc (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKmtalloc");

    // this attribute is mandatory = yes
    CHKExistAttribute (MTALLOC_IDSET (arg_node), arg_node,
                       "mandatory attribute MTALLOC_IDSET is NULL");

    CHKRightType (MTALLOC_IDSET (arg_node), arg_node, "DFMMASK",
                  "attribute MTALLOC_IDSET hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKmtsignal (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKmtsignal");

    // this attribute is mandatory = yes
    CHKExistAttribute (MTSIGNAL_IDSET (arg_node), arg_node,
                       "mandatory attribute MTSIGNAL_IDSET is NULL");

    CHKRightType (MTSIGNAL_IDSET (arg_node), arg_node, "DFMMASK",
                  "attribute MTSIGNAL_IDSET hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKmtsync (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKmtsync");

    // this attribute is mandatory = yes
    CHKExistAttribute (MTSYNC_ALLOC (arg_node), arg_node,
                       "mandatory attribute MTSYNC_ALLOC is NULL");

    CHKRightType (MTSYNC_ALLOC (arg_node), arg_node, "DFMMASK",
                  "attribute MTSYNC_ALLOC hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MTSYNC_FOLD (arg_node), arg_node,
                       "mandatory attribute MTSYNC_FOLD is NULL");

    CHKRightType (MTSYNC_FOLD (arg_node), arg_node, "DFMFOLDMASK",
                  "attribute MTSYNC_FOLD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (MTSYNC_WAIT (arg_node), arg_node,
                       "mandatory attribute MTSYNC_WAIT is NULL");

    CHKRightType (MTSYNC_WAIT (arg_node), arg_node, "DFMMASK",
                  "attribute MTSYNC_WAIT hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKncode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKncode");

    // this son is mandatory = yes
    CHKExistChild (NCODE_CEXPRS (arg_node), arg_node,
                   "mandatory son NCODE_CEXPRS is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_AP_DUMMY_CODE (arg_node), arg_node,
                       "mandatory attribute NCODE_AP_DUMMY_CODE is NULL");

    CHKRightType (NCODE_AP_DUMMY_CODE (arg_node), arg_node, "BOOL",
                  "attribute NCODE_AP_DUMMY_CODE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_FLAG (arg_node), arg_node,
                       "mandatory attribute NCODE_FLAG is NULL");

    CHKRightType (NCODE_FLAG (arg_node), arg_node, "BOOL",
                  "attribute NCODE_FLAG hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_ID (arg_node), arg_node,
                       "mandatory attribute NCODE_ID is NULL");

    CHKRightType (NCODE_ID (arg_node), arg_node, "INTEGER",
                  "attribute NCODE_ID hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_INC_RC_IDS (arg_node), arg_node,
                       "mandatory attribute NCODE_INC_RC_IDS is NULL");

    CHKRightType (NCODE_INC_RC_IDS (arg_node), arg_node, "IDS",
                  "attribute NCODE_INC_RC_IDS hasnt the right type");

    CHKRightType (NCODE_RESOLVEABLE_DEPEND (arg_node), arg_node, "BOOL",
                  "attribute NCODE_RESOLVEABLE_DEPEND hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_TSI_TILESHP (arg_node), arg_node,
                       "mandatory attribute NCODE_TSI_TILESHP is NULL");

    CHKRightType (NCODE_TSI_TILESHP (arg_node), arg_node, "SHPSEG",
                  "attribute NCODE_TSI_TILESHP hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_USE (arg_node), arg_node,
                       "mandatory attribute NCODE_USE is NULL");

    CHKRightType (NCODE_USE (arg_node), arg_node, "LINK",
                  "attribute NCODE_USE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_USED (arg_node), arg_node,
                       "mandatory attribute NCODE_USED is NULL");

    CHKRightType (NCODE_USED (arg_node), arg_node, "INTEGER",
                  "attribute NCODE_USED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NCODE_WLAA_INFO (arg_node), arg_node,
                       "mandatory attribute NCODE_WLAA_INFO is NULL");

    CHKRightType (NCODE_WLAA_INFO (arg_node), arg_node, "ACCESSINFO",
                  "attribute NCODE_WLAA_INFO hasnt the right type");

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (NCODE_CEXPRS (arg_node) != NULL) {
        NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);
    }

    if (NCODE_EPILOGUE (arg_node) != NULL) {
        NCODE_EPILOGUE (arg_node) = Trav (NCODE_EPILOGUE (arg_node), arg_info);
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKngenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKngenerator");

    // this attribute is mandatory = yes
    CHKExistAttribute (NGENERATOR_OP1 (arg_node), arg_node,
                       "mandatory attribute NGENERATOR_OP1 is NULL");

    CHKRightType (NGENERATOR_OP1 (arg_node), arg_node, "PRF",
                  "attribute NGENERATOR_OP1 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NGENERATOR_OP1_ORIG (arg_node), arg_node,
                       "mandatory attribute NGENERATOR_OP1_ORIG is NULL");

    CHKRightType (NGENERATOR_OP1_ORIG (arg_node), arg_node, "PRF",
                  "attribute NGENERATOR_OP1_ORIG hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NGENERATOR_OP2 (arg_node), arg_node,
                       "mandatory attribute NGENERATOR_OP2 is NULL");

    CHKRightType (NGENERATOR_OP2 (arg_node), arg_node, "PRF",
                  "attribute NGENERATOR_OP2 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NGENERATOR_OP2_ORIG (arg_node), arg_node,
                       "mandatory attribute NGENERATOR_OP2_ORIG is NULL");

    CHKRightType (NGENERATOR_OP2_ORIG (arg_node), arg_node, "PRF",
                  "attribute NGENERATOR_OP2_ORIG hasnt the right type");

    if (NGENERATOR_BOUND1 (arg_node) != NULL) {
        NGENERATOR_BOUND1 (arg_node) = Trav (NGENERATOR_BOUND1 (arg_node), arg_info);
    }

    if (NGENERATOR_BOUND2 (arg_node) != NULL) {
        NGENERATOR_BOUND2 (arg_node) = Trav (NGENERATOR_BOUND2 (arg_node), arg_info);
    }

    if (NGENERATOR_STEP (arg_node) != NULL) {
        NGENERATOR_STEP (arg_node) = Trav (NGENERATOR_STEP (arg_node), arg_info);
    }

    if (NGENERATOR_WIDTH (arg_node) != NULL) {
        NGENERATOR_WIDTH (arg_node) = Trav (NGENERATOR_WIDTH (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKnpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKnpart");

    // this son is mandatory = yes
    CHKExistChild (NPART_GENERATOR (arg_node), arg_node,
                   "mandatory son NPART_GENERATOR is NULL");

    // this son is mandatory = yes
    CHKExistChild (NPART_WITHID (arg_node), arg_node,
                   "mandatory son NPART_WITHID is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (NPART_CODE (arg_node), arg_node,
                       "mandatory attribute NPART_CODE is NULL");

    CHKRightType (NPART_CODE (arg_node), arg_node, "LINK",
                  "attribute NPART_CODE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NPART_COPY (arg_node), arg_node,
                       "mandatory attribute NPART_COPY is NULL");

    CHKRightType (NPART_COPY (arg_node), arg_node, "BOOL",
                  "attribute NPART_COPY hasnt the right type");

    if (NPART_GENERATOR (arg_node) != NULL) {
        NPART_GENERATOR (arg_node) = Trav (NPART_GENERATOR (arg_node), arg_info);
    }

    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    if (NPART_WITHID (arg_node) != NULL) {
        NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKnum (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKnum");

    // this attribute is mandatory = yes
    CHKExistAttribute (NUM_VAL (arg_node), arg_node,
                       "mandatory attribute NUM_VAL is NULL");

    CHKRightType (NUM_VAL (arg_node), arg_node, "INTEGER",
                  "attribute NUM_VAL hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKnwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKnwith");

    // this son is mandatory = yes
    CHKExistChild (NWITH_CODE (arg_node), arg_node, "mandatory son NWITH_CODE is NULL");

    // this son is mandatory = yes
    CHKExistChild (NWITH_PART (arg_node), arg_node, "mandatory son NWITH_PART is NULL");

    // this son is mandatory = yes
    CHKExistChild (NWITH_WITHOP (arg_node), arg_node,
                   "mandatory son NWITH_WITHOP is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_DEC_RC_IDS (arg_node), arg_node,
                       "mandatory attribute NWITH_DEC_RC_IDS is NULL");

    CHKRightType (NWITH_DEC_RC_IDS (arg_node), arg_node, "IDS",
                  "attribute NWITH_DEC_RC_IDS hasnt the right type");

    CHKRightType (NWITH_DEPENDENT (arg_node), arg_node, "BOOL",
                  "attribute NWITH_DEPENDENT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_FOLDABLE (arg_node), arg_node,
                       "mandatory attribute NWITH_FOLDABLE is NULL");

    CHKRightType (NWITH_FOLDABLE (arg_node), arg_node, "BOOL",
                  "attribute NWITH_FOLDABLE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_IN_MASK (arg_node), arg_node,
                       "mandatory attribute NWITH_IN_MASK is NULL");

    CHKRightType (NWITH_IN_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute NWITH_IN_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_LOCAL_MASK (arg_node), arg_node,
                       "mandatory attribute NWITH_LOCAL_MASK is NULL");

    CHKRightType (NWITH_LOCAL_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute NWITH_LOCAL_MASK hasnt the right type");

    CHKRightType (NWITH_MTO_OFFSET_NEEDED (arg_node), arg_node, "BOOL",
                  "attribute NWITH_MTO_OFFSET_NEEDED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_NO_CHANCE (arg_node), arg_node,
                       "mandatory attribute NWITH_NO_CHANCE is NULL");

    CHKRightType (NWITH_NO_CHANCE (arg_node), arg_node, "BOOL",
                  "attribute NWITH_NO_CHANCE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_OUT_MASK (arg_node), arg_node,
                       "mandatory attribute NWITH_OUT_MASK is NULL");

    CHKRightType (NWITH_OUT_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute NWITH_OUT_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_PARTS (arg_node), arg_node,
                       "mandatory attribute NWITH_PARTS is NULL");

    CHKRightType (NWITH_PARTS (arg_node), arg_node, "INTEGER",
                  "attribute NWITH_PARTS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_PRAGMA (arg_node), arg_node,
                       "mandatory attribute NWITH_PRAGMA is NULL");

    CHKRightType (NWITH_PRAGMA (arg_node), arg_node, "NODE",
                  "attribute NWITH_PRAGMA hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_REFERENCED (arg_node), arg_node,
                       "mandatory attribute NWITH_REFERENCED is NULL");

    CHKRightType (NWITH_REFERENCED (arg_node), arg_node, "INTEGER",
                  "attribute NWITH_REFERENCED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_REFERENCED_FOLD (arg_node), arg_node,
                       "mandatory attribute NWITH_REFERENCED_FOLD is NULL");

    CHKRightType (NWITH_REFERENCED_FOLD (arg_node), arg_node, "INTEGER",
                  "attribute NWITH_REFERENCED_FOLD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH_REFERENCES_FOLDED (arg_node), arg_node,
                       "mandatory attribute NWITH_REFERENCES_FOLDED is NULL");

    CHKRightType (NWITH_REFERENCES_FOLDED (arg_node), arg_node, "INTEGER",
                  "attribute NWITH_REFERENCES_FOLDED hasnt the right type");

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKnwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKnwith2");

    // this son is mandatory = yes
    CHKExistChild (NWITH2_CODE (arg_node), arg_node, "mandatory son NWITH2_CODE is NULL");

    // this son is mandatory = yes
    CHKExistChild (NWITH2_SEGS (arg_node), arg_node, "mandatory son NWITH2_SEGS is NULL");

    // this son is mandatory = yes
    CHKExistChild (NWITH2_WITHID (arg_node), arg_node,
                   "mandatory son NWITH2_WITHID is NULL");

    // this son is mandatory = yes
    CHKExistChild (NWITH2_WITHOP (arg_node), arg_node,
                   "mandatory son NWITH2_WITHOP is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_CALCPARALLEL (arg_node), arg_node,
                       "mandatory attribute NWITH2_CALCPARALLEL is NULL");

    CHKRightType (NWITH2_CALCPARALLEL (arg_node), arg_node, "BOOL",
                  "attribute NWITH2_CALCPARALLEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_DEC_RC_IDS (arg_node), arg_node,
                       "mandatory attribute NWITH2_DEC_RC_IDS is NULL");

    CHKRightType (NWITH2_DEC_RC_IDS (arg_node), arg_node, "IDS",
                  "attribute NWITH2_DEC_RC_IDS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_DIMS (arg_node), arg_node,
                       "mandatory attribute NWITH2_DIMS is NULL");

    CHKRightType (NWITH2_DIMS (arg_node), arg_node, "INTEGER",
                  "attribute NWITH2_DIMS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_IN_MASK (arg_node), arg_node,
                       "mandatory attribute NWITH2_IN_MASK is NULL");

    CHKRightType (NWITH2_IN_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute NWITH2_IN_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_LOCAL_MASK (arg_node), arg_node,
                       "mandatory attribute NWITH2_LOCAL_MASK is NULL");

    CHKRightType (NWITH2_LOCAL_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute NWITH2_LOCAL_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_MT (arg_node), arg_node,
                       "mandatory attribute NWITH2_MT is NULL");

    CHKRightType (NWITH2_MT (arg_node), arg_node, "BOOL",
                  "attribute NWITH2_MT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_OFFSET_NEEDED (arg_node), arg_node,
                       "mandatory attribute NWITH2_OFFSET_NEEDED is NULL");

    CHKRightType (NWITH2_OFFSET_NEEDED (arg_node), arg_node, "BOOL",
                  "attribute NWITH2_OFFSET_NEEDED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_OUT_MASK (arg_node), arg_node,
                       "mandatory attribute NWITH2_OUT_MASK is NULL");

    CHKRightType (NWITH2_OUT_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute NWITH2_OUT_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_PRAGMA (arg_node), arg_node,
                       "mandatory attribute NWITH2_PRAGMA is NULL");

    CHKRightType (NWITH2_PRAGMA (arg_node), arg_node, "NODE",
                  "attribute NWITH2_PRAGMA hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITH2_REUSE (arg_node), arg_node,
                       "mandatory attribute NWITH2_REUSE is NULL");

    CHKRightType (NWITH2_REUSE (arg_node), arg_node, "DFMMASK",
                  "attribute NWITH2_REUSE hasnt the right type");

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    if (NWITH2_SEGS (arg_node) != NULL) {
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    }

    if (NWITH2_WITHID (arg_node) != NULL) {
        NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    }

    if (NWITH2_WITHOP (arg_node) != NULL) {
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKnwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKnwithid");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHID_IDS (arg_node), arg_node,
                       "mandatory attribute NWITHID_IDS is NULL");

    CHKRightType (NWITHID_IDS (arg_node), arg_node, "IDS",
                  "attribute NWITHID_IDS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHID_VEC (arg_node), arg_node,
                       "mandatory attribute NWITHID_VEC is NULL");

    CHKRightType (NWITHID_VEC (arg_node), arg_node, "IDS",
                  "attribute NWITHID_VEC hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHID_VECNEEDED (arg_node), arg_node,
                       "mandatory attribute NWITHID_VECNEEDED is NULL");

    CHKRightType (NWITHID_VECNEEDED (arg_node), arg_node, "BOOL",
                  "attribute NWITHID_VECNEEDED hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKnwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKnwithop");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHOP_EXPR (arg_node), arg_node,
                       "mandatory attribute NWITHOP_EXPR is NULL");

    CHKRightType (NWITHOP_EXPR (arg_node), arg_node, "LINK",
                  "attribute NWITHOP_EXPR hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHOP_FUN (arg_node), arg_node,
                       "mandatory attribute NWITHOP_FUN is NULL");

    CHKRightType (NWITHOP_FUN (arg_node), arg_node, "STRING",
                  "attribute NWITHOP_FUN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHOP_FUNDEF (arg_node), arg_node,
                       "mandatory attribute NWITHOP_FUNDEF is NULL");

    CHKRightType (NWITHOP_FUNDEF (arg_node), arg_node, "LINK",
                  "attribute NWITHOP_FUNDEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHOP_MOD (arg_node), arg_node,
                       "mandatory attribute NWITHOP_MOD is NULL");

    CHKRightType (NWITHOP_MOD (arg_node), arg_node, "SHAREDSTRING",
                  "attribute NWITHOP_MOD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHOP_OFFSET_NEEDED (arg_node), arg_node,
                       "mandatory attribute NWITHOP_OFFSET_NEEDED is NULL");

    CHKRightType (NWITHOP_OFFSET_NEEDED (arg_node), arg_node, "BOOL",
                  "attribute NWITHOP_OFFSET_NEEDED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHOP_PRF (arg_node), arg_node,
                       "mandatory attribute NWITHOP_PRF is NULL");

    CHKRightType (NWITHOP_PRF (arg_node), arg_node, "PRF",
                  "attribute NWITHOP_PRF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (NWITHOP_TYPE (arg_node), arg_node,
                       "mandatory attribute NWITHOP_TYPE is NULL");

    CHKRightType (NWITHOP_TYPE (arg_node), arg_node, "WITHOPTYPE",
                  "attribute NWITHOP_TYPE hasnt the right type");

    if (NWITHOP_DEFAULT (arg_node) != NULL) {
        NWITHOP_DEFAULT (arg_node) = Trav (NWITHOP_DEFAULT (arg_node), arg_info);
    }

    if (NWITHOP_MEM (arg_node) != NULL) {
        NWITHOP_MEM (arg_node) = Trav (NWITHOP_MEM (arg_node), arg_info);
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    if (NWITHOP_SHAPEARRAYNEUTRAL (arg_node) != NULL) {
        NWITHOP_SHAPEARRAYNEUTRAL (arg_node)
          = Trav (NWITHOP_SHAPEARRAYNEUTRAL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKobjdef");

    // this son is mandatory = yes
    CHKExistChild (OBJDEF_EXPR (arg_node), arg_node, "mandatory son OBJDEF_EXPR is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_ARG (arg_node), arg_node,
                       "mandatory attribute OBJDEF_ARG is NULL");

    CHKRightType (OBJDEF_ARG (arg_node), arg_node, "LINK",
                  "attribute OBJDEF_ARG hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_ATTRIB (arg_node), arg_node,
                       "mandatory attribute OBJDEF_ATTRIB is NULL");

    CHKRightType (OBJDEF_ATTRIB (arg_node), arg_node, "STATUSTYPE",
                  "attribute OBJDEF_ATTRIB hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_AVIS (arg_node), arg_node,
                       "mandatory attribute OBJDEF_AVIS is NULL");

    CHKRightType (OBJDEF_AVIS (arg_node), arg_node, "LINK",
                  "attribute OBJDEF_AVIS hasnt the right type");

    CHKRightType (OBJDEF_ICM (arg_node), arg_node, "LINK",
                  "attribute OBJDEF_ICM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_MOD (arg_node), arg_node,
                       "mandatory attribute OBJDEF_MOD is NULL");

    CHKRightType (OBJDEF_MOD (arg_node), arg_node, "SHAREDSTRING",
                  "attribute OBJDEF_MOD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_NAME (arg_node), arg_node,
                       "mandatory attribute OBJDEF_NAME is NULL");

    CHKRightType (OBJDEF_NAME (arg_node), arg_node, "STRING",
                  "attribute OBJDEF_NAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_NEEDOBJS (arg_node), arg_node,
                       "mandatory attribute OBJDEF_NEEDOBJS is NULL");

    CHKRightType (OBJDEF_NEEDOBJS (arg_node), arg_node, "SHAREDNODELIST",
                  "attribute OBJDEF_NEEDOBJS hasnt the right type");

    CHKRightType (OBJDEF_OBJDEC_DEF (arg_node), arg_node, "LINK",
                  "attribute OBJDEF_OBJDEC_DEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_PRAGMA (arg_node), arg_node,
                       "mandatory attribute OBJDEF_PRAGMA is NULL");

    CHKRightType (OBJDEF_PRAGMA (arg_node), arg_node, "NODE",
                  "attribute OBJDEF_PRAGMA hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_SIB (arg_node), arg_node,
                       "mandatory attribute OBJDEF_SIB is NULL");

    CHKRightType (OBJDEF_SIB (arg_node), arg_node, "LINK",
                  "attribute OBJDEF_SIB hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_STATUS (arg_node), arg_node,
                       "mandatory attribute OBJDEF_STATUS is NULL");

    CHKRightType (OBJDEF_STATUS (arg_node), arg_node, "STATUSTYPE",
                  "attribute OBJDEF_STATUS hasnt the right type");

    CHKRightType (OBJDEF_SYMBOLNAME (arg_node), arg_node, "STRING",
                  "attribute OBJDEF_SYMBOLNAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_TYPE (arg_node), arg_node,
                       "mandatory attribute OBJDEF_TYPE is NULL");

    CHKRightType (OBJDEF_TYPE (arg_node), arg_node, "OLDTYPE",
                  "attribute OBJDEF_TYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (OBJDEF_VARNAME (arg_node), arg_node,
                       "mandatory attribute OBJDEF_VARNAME is NULL");

    CHKRightType (OBJDEF_VARNAME (arg_node), arg_node, "STRING",
                  "attribute OBJDEF_VARNAME hasnt the right type");

    if (OBJDEF_EXPR (arg_node) != NULL) {
        OBJDEF_EXPR (arg_node) = Trav (OBJDEF_EXPR (arg_node), arg_info);
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKok (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKok");

    DBUG_RETURN (arg_node);
}

node *
CHKpragma (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKpragma");

    CHKRightType (PRAGMA_APL (arg_node), arg_node, "LINK",
                  "attribute PRAGMA_APL hasnt the right type");

    CHKRightType (PRAGMA_COPYFUN (arg_node), arg_node, "STRING",
                  "attribute PRAGMA_COPYFUN hasnt the right type");

    CHKRightType (PRAGMA_EFFECT (arg_node), arg_node, "IDS",
                  "attribute PRAGMA_EFFECT hasnt the right type");

    CHKRightType (PRAGMA_FREEFUN (arg_node), arg_node, "STRING",
                  "attribute PRAGMA_FREEFUN hasnt the right type");

    CHKRightType (PRAGMA_INITFUN (arg_node), arg_node, "STRING",
                  "attribute PRAGMA_INITFUN hasnt the right type");

    CHKRightType (PRAGMA_LINKMOD (arg_node), arg_node, "STRING",
                  "attribute PRAGMA_LINKMOD hasnt the right type");

    CHKRightType (PRAGMA_LINKNAME (arg_node), arg_node, "STRING",
                  "attribute PRAGMA_LINKNAME hasnt the right type");

    CHKRightType (PRAGMA_LINKOBJ (arg_node), arg_node, "STRING",
                  "attribute PRAGMA_LINKOBJ hasnt the right type");

    CHKRightType (PRAGMA_LINKSIGN (arg_node), arg_node, "INTEGERARRAY",
                  "attribute PRAGMA_LINKSIGN hasnt the right type");

    CHKRightType (PRAGMA_LINKSIGNNUMS (arg_node), arg_node, "NUMS",
                  "attribute PRAGMA_LINKSIGNNUMS hasnt the right type");

    CHKRightType (PRAGMA_NEEDFUNS (arg_node), arg_node, "LINK",
                  "attribute PRAGMA_NEEDFUNS hasnt the right type");

    CHKRightType (PRAGMA_NEEDTYPES (arg_node), arg_node, "IDS",
                  "attribute PRAGMA_NEEDTYPES hasnt the right type");

    CHKRightType (PRAGMA_NUMPARAMS (arg_node), arg_node, "INTEGER",
                  "attribute PRAGMA_NUMPARAMS hasnt the right type");

    CHKRightType (PRAGMA_READONLY (arg_node), arg_node, "INTEGERARRAY",
                  "attribute PRAGMA_READONLY hasnt the right type");

    CHKRightType (PRAGMA_READONLYNUMS (arg_node), arg_node, "NUMS",
                  "attribute PRAGMA_READONLYNUMS hasnt the right type");

    CHKRightType (PRAGMA_REFCOUNTING (arg_node), arg_node, "INTEGERARRAY",
                  "attribute PRAGMA_REFCOUNTING hasnt the right type");

    CHKRightType (PRAGMA_REFCOUNTINGNUMS (arg_node), arg_node, "NUMS",
                  "attribute PRAGMA_REFCOUNTINGNUMS hasnt the right type");

    CHKRightType (PRAGMA_TOUCH (arg_node), arg_node, "IDS",
                  "attribute PRAGMA_TOUCH hasnt the right type");

    CHKRightType (PRAGMA_WLCOMP_APS (arg_node), arg_node, "LINK",
                  "attribute PRAGMA_WLCOMP_APS hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKprf");

    // this son is mandatory = yes
    CHKExistChild (PRF_ARGS (arg_node), arg_node, "mandatory son PRF_ARGS is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (PRF_PRF (arg_node), arg_node,
                       "mandatory attribute PRF_PRF is NULL");

    CHKRightType (PRF_PRF (arg_node), arg_node, "PRF",
                  "attribute PRF_PRF hasnt the right type");

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKprovide");

    // this attribute is mandatory = yes
    CHKExistAttribute (PROVIDE_ALL (arg_node), arg_node,
                       "mandatory attribute PROVIDE_ALL is NULL");

    CHKRightType (PROVIDE_ALL (arg_node), arg_node, "BOOL",
                  "attribute PROVIDE_ALL hasnt the right type");

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = Trav (PROVIDE_NEXT (arg_node), arg_info);
    }

    if (PROVIDE_SYMBOL (arg_node) != NULL) {
        PROVIDE_SYMBOL (arg_node) = Trav (PROVIDE_SYMBOL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKreturn");

    // this son is mandatory = yes
    CHKExistChild (RETURN_EXPRS (arg_node), arg_node,
                   "mandatory son RETURN_EXPRS is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (RETURN_CRET (arg_node), arg_node,
                       "mandatory attribute RETURN_CRET is NULL");

    CHKRightType (RETURN_CRET (arg_node), arg_node, "LINK",
                  "attribute RETURN_CRET hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (RETURN_DEFMASK (arg_node), arg_node,
                       "mandatory attribute RETURN_DEFMASK is NULL");

    CHKRightType (RETURN_DEFMASK (arg_node), arg_node, "DFMMASK",
                  "attribute RETURN_DEFMASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (RETURN_REFERENCE (arg_node), arg_node,
                       "mandatory attribute RETURN_REFERENCE is NULL");

    CHKRightType (RETURN_REFERENCE (arg_node), arg_node, "LINK",
                  "attribute RETURN_REFERENCE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (RETURN_USEMASK (arg_node), arg_node,
                       "mandatory attribute RETURN_USEMASK is NULL");

    CHKRightType (RETURN_USEMASK (arg_node), arg_node, "DFMMASK",
                  "attribute RETURN_USEMASK hasnt the right type");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKsetwl");

    // this son is mandatory = yes
    CHKExistChild (SETWL_EXPR (arg_node), arg_node, "mandatory son SETWL_EXPR is NULL");

    // this son is mandatory = yes
    CHKExistChild (SETWL_IDS (arg_node), arg_node, "mandatory son SETWL_IDS is NULL");

    if (SETWL_EXPR (arg_node) != NULL) {
        SETWL_EXPR (arg_node) = Trav (SETWL_EXPR (arg_node), arg_info);
    }

    if (SETWL_IDS (arg_node) != NULL) {
        SETWL_IDS (arg_node) = Trav (SETWL_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKspmd (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKspmd");

    // this son is mandatory = yes
    CHKExistChild (SPMD_REGION (arg_node), arg_node, "mandatory son SPMD_REGION is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_FUNDEF (arg_node), arg_node,
                       "mandatory attribute SPMD_FUNDEF is NULL");

    CHKRightType (SPMD_FUNDEF (arg_node), arg_node, "EXTLINK",
                  "attribute SPMD_FUNDEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_ICM_ALTSEQ (arg_node), arg_node,
                       "mandatory attribute SPMD_ICM_ALTSEQ is NULL");

    CHKRightType (SPMD_ICM_ALTSEQ (arg_node), arg_node, "NODE",
                  "attribute SPMD_ICM_ALTSEQ hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_ICM_BEGIN (arg_node), arg_node,
                       "mandatory attribute SPMD_ICM_BEGIN is NULL");

    CHKRightType (SPMD_ICM_BEGIN (arg_node), arg_node, "NODE",
                  "attribute SPMD_ICM_BEGIN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_ICM_END (arg_node), arg_node,
                       "mandatory attribute SPMD_ICM_END is NULL");

    CHKRightType (SPMD_ICM_END (arg_node), arg_node, "NODE",
                  "attribute SPMD_ICM_END hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_ICM_PARALLEL (arg_node), arg_node,
                       "mandatory attribute SPMD_ICM_PARALLEL is NULL");

    CHKRightType (SPMD_ICM_PARALLEL (arg_node), arg_node, "NODE",
                  "attribute SPMD_ICM_PARALLEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_ICM_SEQUENTIAL (arg_node), arg_node,
                       "mandatory attribute SPMD_ICM_SEQUENTIAL is NULL");

    CHKRightType (SPMD_ICM_SEQUENTIAL (arg_node), arg_node, "NODE",
                  "attribute SPMD_ICM_SEQUENTIAL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_IN (arg_node), arg_node,
                       "mandatory attribute SPMD_IN is NULL");

    CHKRightType (SPMD_IN (arg_node), arg_node, "DFMMASK",
                  "attribute SPMD_IN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_INOUT (arg_node), arg_node,
                       "mandatory attribute SPMD_INOUT is NULL");

    CHKRightType (SPMD_INOUT (arg_node), arg_node, "DFMMASK",
                  "attribute SPMD_INOUT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_LOCAL (arg_node), arg_node,
                       "mandatory attribute SPMD_LOCAL is NULL");

    CHKRightType (SPMD_LOCAL (arg_node), arg_node, "DFMMASK",
                  "attribute SPMD_LOCAL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_OUT (arg_node), arg_node,
                       "mandatory attribute SPMD_OUT is NULL");

    CHKRightType (SPMD_OUT (arg_node), arg_node, "DFMMASK",
                  "attribute SPMD_OUT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_SHARED (arg_node), arg_node,
                       "mandatory attribute SPMD_SHARED is NULL");

    CHKRightType (SPMD_SHARED (arg_node), arg_node, "DFMMASK",
                  "attribute SPMD_SHARED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SPMD_STATIC (arg_node), arg_node,
                       "mandatory attribute SPMD_STATIC is NULL");

    CHKRightType (SPMD_STATIC (arg_node), arg_node, "INTEGER",
                  "attribute SPMD_STATIC hasnt the right type");

    if (SPMD_REGION (arg_node) != NULL) {
        SPMD_REGION (arg_node) = Trav (SPMD_REGION (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKssacnt (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKssacnt");

    // this attribute is mandatory = yes
    CHKExistAttribute (SSACNT_BASEID (arg_node), arg_node,
                       "mandatory attribute SSACNT_BASEID is NULL");

    CHKRightType (SSACNT_BASEID (arg_node), arg_node, "STRING",
                  "attribute SSACNT_BASEID hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SSACNT_COUNT (arg_node), arg_node,
                       "mandatory attribute SSACNT_COUNT is NULL");

    CHKRightType (SSACNT_COUNT (arg_node), arg_node, "INTEGER",
                  "attribute SSACNT_COUNT hasnt the right type");

    if (SSACNT_NEXT (arg_node) != NULL) {
        SSACNT_NEXT (arg_node) = Trav (SSACNT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKssastack (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKssastack");

    // this attribute is mandatory = yes
    CHKExistAttribute (SSASTACK_AVIS (arg_node), arg_node,
                       "mandatory attribute SSASTACK_AVIS is NULL");

    CHKRightType (SSASTACK_AVIS (arg_node), arg_node, "LINK",
                  "attribute SSASTACK_AVIS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SSASTACK_INUSE (arg_node), arg_node,
                       "mandatory attribute SSASTACK_INUSE is NULL");

    CHKRightType (SSASTACK_INUSE (arg_node), arg_node, "BOOL",
                  "attribute SSASTACK_INUSE hasnt the right type");

    if (SSASTACK_NEXT (arg_node) != NULL) {
        SSASTACK_NEXT (arg_node) = Trav (SSASTACK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKst (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKst");

    // this son is mandatory = yes
    CHKExistChild (ST_REGION (arg_node), arg_node, "mandatory son ST_REGION is NULL");

    CHKRightType (ST_ALLOC (arg_node), arg_node, "DFMMASK",
                  "attribute ST_ALLOC hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ST_DEFMASK (arg_node), arg_node,
                       "mandatory attribute ST_DEFMASK is NULL");

    CHKRightType (ST_DEFMASK (arg_node), arg_node, "DFMMASK",
                  "attribute ST_DEFMASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ST_NEEDLATER_MT (arg_node), arg_node,
                       "mandatory attribute ST_NEEDLATER_MT is NULL");

    CHKRightType (ST_NEEDLATER_MT (arg_node), arg_node, "DFMMASK",
                  "attribute ST_NEEDLATER_MT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ST_NEEDLATER_ST (arg_node), arg_node,
                       "mandatory attribute ST_NEEDLATER_ST is NULL");

    CHKRightType (ST_NEEDLATER_ST (arg_node), arg_node, "DFMMASK",
                  "attribute ST_NEEDLATER_ST hasnt the right type");

    CHKRightType (ST_SYNC (arg_node), arg_node, "DFMMASK",
                  "attribute ST_SYNC hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (ST_USEMASK (arg_node), arg_node,
                       "mandatory attribute ST_USEMASK is NULL");

    CHKRightType (ST_USEMASK (arg_node), arg_node, "DFMMASK",
                  "attribute ST_USEMASK hasnt the right type");

    if (ST_REGION (arg_node) != NULL) {
        ST_REGION (arg_node) = Trav (ST_REGION (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKstop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKstop");

    DBUG_RETURN (arg_node);
}

node *
CHKstr (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKstr");

    // this attribute is mandatory = yes
    CHKExistAttribute (STR_STRING (arg_node), arg_node,
                       "mandatory attribute STR_STRING is NULL");

    CHKRightType (STR_STRING (arg_node), arg_node, "STRING",
                  "attribute STR_STRING hasnt the right type");

    DBUG_RETURN (arg_node);
}

node *
CHKsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKsymbol");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYMBOL_ID (arg_node), arg_node,
                       "mandatory attribute SYMBOL_ID is NULL");

    CHKRightType (SYMBOL_ID (arg_node), arg_node, "STRING",
                  "attribute SYMBOL_ID hasnt the right type");

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = Trav (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKsync (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKsync");

    // this son is mandatory = yes
    CHKExistChild (SYNC_REGION (arg_node), arg_node, "mandatory son SYNC_REGION is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_FIRST (arg_node), arg_node,
                       "mandatory attribute SYNC_FIRST is NULL");

    CHKRightType (SYNC_FIRST (arg_node), arg_node, "INTEGER",
                  "attribute SYNC_FIRST hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_FOLDCOUNT (arg_node), arg_node,
                       "mandatory attribute SYNC_FOLDCOUNT is NULL");

    CHKRightType (SYNC_FOLDCOUNT (arg_node), arg_node, "INTEGER",
                  "attribute SYNC_FOLDCOUNT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_IN (arg_node), arg_node,
                       "mandatory attribute SYNC_IN is NULL");

    CHKRightType (SYNC_IN (arg_node), arg_node, "DFMMASK",
                  "attribute SYNC_IN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_INOUT (arg_node), arg_node,
                       "mandatory attribute SYNC_INOUT is NULL");

    CHKRightType (SYNC_INOUT (arg_node), arg_node, "DFMMASK",
                  "attribute SYNC_INOUT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_LAST (arg_node), arg_node,
                       "mandatory attribute SYNC_LAST is NULL");

    CHKRightType (SYNC_LAST (arg_node), arg_node, "INTEGER",
                  "attribute SYNC_LAST hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_LOCAL (arg_node), arg_node,
                       "mandatory attribute SYNC_LOCAL is NULL");

    CHKRightType (SYNC_LOCAL (arg_node), arg_node, "DFMMASK",
                  "attribute SYNC_LOCAL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_OUT (arg_node), arg_node,
                       "mandatory attribute SYNC_OUT is NULL");

    CHKRightType (SYNC_OUT (arg_node), arg_node, "DFMMASK",
                  "attribute SYNC_OUT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_OUTREP (arg_node), arg_node,
                       "mandatory attribute SYNC_OUTREP is NULL");

    CHKRightType (SYNC_OUTREP (arg_node), arg_node, "DFMMASK",
                  "attribute SYNC_OUTREP hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (SYNC_WITH_PTRS (arg_node), arg_node,
                       "mandatory attribute SYNC_WITH_PTRS is NULL");

    CHKRightType (SYNC_WITH_PTRS (arg_node), arg_node, "NODE",
                  "attribute SYNC_WITH_PTRS hasnt the right type");

    if (SYNC_REGION (arg_node) != NULL) {
        SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKtcfuninfo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKtcfuninfo");

    // this son is mandatory = yes
    CHKExistChild (TCFUNINFO_DEF (arg_node), arg_node,
                   "mandatory son TCFUNINFO_DEF is NULL");

    // this son is mandatory = yes
    CHKExistChild (TCFUNINFO_DOWN (arg_node), arg_node,
                   "mandatory son TCFUNINFO_DOWN is NULL");

    // this son is mandatory = yes
    CHKExistChild (TCFUNINFO_UP (arg_node), arg_node,
                   "mandatory son TCFUNINFO_UP is NULL");

    if (TCFUNINFO_DEF (arg_node) != NULL) {
        TCFUNINFO_DEF (arg_node) = Trav (TCFUNINFO_DEF (arg_node), arg_info);
    }

    if (TCFUNINFO_DOWN (arg_node) != NULL) {
        TCFUNINFO_DOWN (arg_node) = Trav (TCFUNINFO_DOWN (arg_node), arg_info);
    }

    if (TCFUNINFO_UP (arg_node) != NULL) {
        TCFUNINFO_UP (arg_node) = Trav (TCFUNINFO_UP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKtypedef");

    // this attribute is mandatory = yes
    CHKExistAttribute (TYPEDEF_ATTRIB (arg_node), arg_node,
                       "mandatory attribute TYPEDEF_ATTRIB is NULL");

    CHKRightType (TYPEDEF_ATTRIB (arg_node), arg_node, "STATUSTYPE",
                  "attribute TYPEDEF_ATTRIB hasnt the right type");

    CHKRightType (TYPEDEF_COPYFUN (arg_node), arg_node, "STRING",
                  "attribute TYPEDEF_COPYFUN hasnt the right type");

    CHKRightType (TYPEDEF_FREEFUN (arg_node), arg_node, "STRING",
                  "attribute TYPEDEF_FREEFUN hasnt the right type");

    CHKRightType (TYPEDEF_ICM (arg_node), arg_node, "NODE",
                  "attribute TYPEDEF_ICM hasnt the right type");

    CHKRightType (TYPEDEF_IMPL (arg_node), arg_node, "OLDTYPE",
                  "attribute TYPEDEF_IMPL hasnt the right type");

    CHKRightType (TYPEDEF_MOD (arg_node), arg_node, "SHAREDSTRING",
                  "attribute TYPEDEF_MOD hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (TYPEDEF_NAME (arg_node), arg_node,
                       "mandatory attribute TYPEDEF_NAME is NULL");

    CHKRightType (TYPEDEF_NAME (arg_node), arg_node, "STRING",
                  "attribute TYPEDEF_NAME hasnt the right type");

    CHKRightType (TYPEDEF_PRAGMA (arg_node), arg_node, "NODE",
                  "attribute TYPEDEF_PRAGMA hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (TYPEDEF_STATUS (arg_node), arg_node,
                       "mandatory attribute TYPEDEF_STATUS is NULL");

    CHKRightType (TYPEDEF_STATUS (arg_node), arg_node, "STATUSTYPE",
                  "attribute TYPEDEF_STATUS hasnt the right type");

    CHKRightType (TYPEDEF_SYMBOLNAME (arg_node), arg_node, "STRING",
                  "attribute TYPEDEF_SYMBOLNAME hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (TYPEDEF_TYPE (arg_node), arg_node,
                       "mandatory attribute TYPEDEF_TYPE is NULL");

    CHKRightType (TYPEDEF_TYPE (arg_node), arg_node, "OLDTYPE",
                  "attribute TYPEDEF_TYPE hasnt the right type");

    CHKRightType (TYPEDEF_TYPEDEC_DEF (arg_node), arg_node, "LINK",
                  "attribute TYPEDEF_TYPEDEC_DEF hasnt the right type");

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKuse");

    // this attribute is mandatory = yes
    CHKExistAttribute (USE_ALL (arg_node), arg_node,
                       "mandatory attribute USE_ALL is NULL");

    CHKRightType (USE_ALL (arg_node), arg_node, "BOOL",
                  "attribute USE_ALL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (USE_MOD (arg_node), arg_node,
                       "mandatory attribute USE_MOD is NULL");

    CHKRightType (USE_MOD (arg_node), arg_node, "STRING",
                  "attribute USE_MOD hasnt the right type");

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = Trav (USE_NEXT (arg_node), arg_info);
    }

    if (USE_SYMBOL (arg_node) != NULL) {
        USE_SYMBOL (arg_node) = Trav (USE_SYMBOL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKvardec");

    CHKRightType (VARDEC_ACTCHN (arg_node), arg_node, "NODE",
                  "attribute VARDEC_ACTCHN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_ATTRIB (arg_node), arg_node,
                       "mandatory attribute VARDEC_ATTRIB is NULL");

    CHKRightType (VARDEC_ATTRIB (arg_node), arg_node, "STATUSTYPE",
                  "attribute VARDEC_ATTRIB hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_AVIS (arg_node), arg_node,
                       "mandatory attribute VARDEC_AVIS is NULL");

    CHKRightType (VARDEC_AVIS (arg_node), arg_node, "NODE",
                  "attribute VARDEC_AVIS hasnt the right type");

    CHKRightType (VARDEC_COLCHN (arg_node), arg_node, "NODE",
                  "attribute VARDEC_COLCHN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_FLAG (arg_node), arg_node,
                       "mandatory attribute VARDEC_FLAG is NULL");

    CHKRightType (VARDEC_FLAG (arg_node), arg_node, "INTEGER",
                  "attribute VARDEC_FLAG hasnt the right type");

    CHKRightType (VARDEC_ICM (arg_node), arg_node, "NODE",
                  "attribute VARDEC_ICM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_NAIVE_REFCNT (arg_node), arg_node,
                       "mandatory attribute VARDEC_NAIVE_REFCNT is NULL");

    CHKRightType (VARDEC_NAIVE_REFCNT (arg_node), arg_node, "INTEGER",
                  "attribute VARDEC_NAIVE_REFCNT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_NAME (arg_node), arg_node,
                       "mandatory attribute VARDEC_NAME is NULL");

    CHKRightType (VARDEC_NAME (arg_node), arg_node, "STRING",
                  "attribute VARDEC_NAME hasnt the right type");

    CHKRightType (VARDEC_OBJDEF (arg_node), arg_node, "LINK",
                  "attribute VARDEC_OBJDEF hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_PADDED (arg_node), arg_node,
                       "mandatory attribute VARDEC_PADDED is NULL");

    CHKRightType (VARDEC_PADDED (arg_node), arg_node, "BOOL",
                  "attribute VARDEC_PADDED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_REFCNT (arg_node), arg_node,
                       "mandatory attribute VARDEC_REFCNT is NULL");

    CHKRightType (VARDEC_REFCNT (arg_node), arg_node, "INTEGER",
                  "attribute VARDEC_REFCNT hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_STATUS (arg_node), arg_node,
                       "mandatory attribute VARDEC_STATUS is NULL");

    CHKRightType (VARDEC_STATUS (arg_node), arg_node, "STATUSTYPE",
                  "attribute VARDEC_STATUS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_TYPE (arg_node), arg_node,
                       "mandatory attribute VARDEC_TYPE is NULL");

    CHKRightType (VARDEC_TYPE (arg_node), arg_node, "OLDTYPE",
                  "attribute VARDEC_TYPE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VARDEC_VARNO (arg_node), arg_node,
                       "mandatory attribute VARDEC_VARNO is NULL");

    CHKRightType (VARDEC_VARNO (arg_node), arg_node, "INTEGER",
                  "attribute VARDEC_VARNO hasnt the right type");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKvinfo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKvinfo");

    CHKRightType (VINFO_DOLLAR (arg_node), arg_node, "LINK",
                  "attribute VINFO_DOLLAR hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (VINFO_FLAG (arg_node), arg_node,
                       "mandatory attribute VINFO_FLAG is NULL");

    CHKRightType (VINFO_FLAG (arg_node), arg_node, "USEFLAG",
                  "attribute VINFO_FLAG hasnt the right type");

    CHKRightType (VINFO_SHAPE (arg_node), arg_node, "SHAPE",
                  "attribute VINFO_SHAPE hasnt the right type");

    CHKRightType (VINFO_VARDEC (arg_node), arg_node, "LINK",
                  "attribute VINFO_VARDEC hasnt the right type");

    if (VINFO_NEXT (arg_node) != NULL) {
        VINFO_NEXT (arg_node) = Trav (VINFO_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwhile");

    // this son is mandatory = yes
    CHKExistChild (WHILE_BODY (arg_node), arg_node, "mandatory son WHILE_BODY is NULL");

    // this son is mandatory = yes
    CHKExistChild (WHILE_COND (arg_node), arg_node, "mandatory son WHILE_COND is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (WHILE_DEFVARS (arg_node), arg_node,
                       "mandatory attribute WHILE_DEFVARS is NULL");

    CHKRightType (WHILE_DEFVARS (arg_node), arg_node, "IDS",
                  "attribute WHILE_DEFVARS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WHILE_IN_MASK (arg_node), arg_node,
                       "mandatory attribute WHILE_IN_MASK is NULL");

    CHKRightType (WHILE_IN_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute WHILE_IN_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WHILE_LOCAL_MASK (arg_node), arg_node,
                       "mandatory attribute WHILE_LOCAL_MASK is NULL");

    CHKRightType (WHILE_LOCAL_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute WHILE_LOCAL_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WHILE_OUT_MASK (arg_node), arg_node,
                       "mandatory attribute WHILE_OUT_MASK is NULL");

    CHKRightType (WHILE_OUT_MASK (arg_node), arg_node, "DFMMASK",
                  "attribute WHILE_OUT_MASK hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WHILE_USEVARS (arg_node), arg_node,
                       "mandatory attribute WHILE_USEVARS is NULL");

    CHKRightType (WHILE_USEVARS (arg_node), arg_node, "IDS",
                  "attribute WHILE_USEVARS hasnt the right type");

    if (WHILE_BODY (arg_node) != NULL) {
        WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);
    }

    if (WHILE_COND (arg_node) != NULL) {
        WHILE_COND (arg_node) = Trav (WHILE_COND (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlblock");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLBLOCK_BOUND1 (arg_node), arg_node,
                       "mandatory attribute WLBLOCK_BOUND1 is NULL");

    CHKRightType (WLBLOCK_BOUND1 (arg_node), arg_node, "INTEGER",
                  "attribute WLBLOCK_BOUND1 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLBLOCK_BOUND2 (arg_node), arg_node,
                       "mandatory attribute WLBLOCK_BOUND2 is NULL");

    CHKRightType (WLBLOCK_BOUND2 (arg_node), arg_node, "INTEGER",
                  "attribute WLBLOCK_BOUND2 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLBLOCK_DIM (arg_node), arg_node,
                       "mandatory attribute WLBLOCK_DIM is NULL");

    CHKRightType (WLBLOCK_DIM (arg_node), arg_node, "INTEGER",
                  "attribute WLBLOCK_DIM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLBLOCK_LEVEL (arg_node), arg_node,
                       "mandatory attribute WLBLOCK_LEVEL is NULL");

    CHKRightType (WLBLOCK_LEVEL (arg_node), arg_node, "INTEGER",
                  "attribute WLBLOCK_LEVEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLBLOCK_STEP (arg_node), arg_node,
                       "mandatory attribute WLBLOCK_STEP is NULL");

    CHKRightType (WLBLOCK_STEP (arg_node), arg_node, "INTEGER",
                  "attribute WLBLOCK_STEP hasnt the right type");

    if (WLBLOCK_CONTENTS (arg_node) != NULL) {
        WLBLOCK_CONTENTS (arg_node) = Trav (WLBLOCK_CONTENTS (arg_node), arg_info);
    }

    if (WLBLOCK_NEXT (arg_node) != NULL) {
        WLBLOCK_NEXT (arg_node) = Trav (WLBLOCK_NEXT (arg_node), arg_info);
    }

    if (WLBLOCK_NEXTDIM (arg_node) != NULL) {
        WLBLOCK_NEXTDIM (arg_node) = Trav (WLBLOCK_NEXTDIM (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlgrid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlgrid");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_BOUND1 (arg_node), arg_node,
                       "mandatory attribute WLGRID_BOUND1 is NULL");

    CHKRightType (WLGRID_BOUND1 (arg_node), arg_node, "INTEGER",
                  "attribute WLGRID_BOUND1 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_BOUND2 (arg_node), arg_node,
                       "mandatory attribute WLGRID_BOUND2 is NULL");

    CHKRightType (WLGRID_BOUND2 (arg_node), arg_node, "INTEGER",
                  "attribute WLGRID_BOUND2 hasnt the right type");

    CHKRightType (WLGRID_CODE (arg_node), arg_node, "LINK",
                  "attribute WLGRID_CODE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_DIM (arg_node), arg_node,
                       "mandatory attribute WLGRID_DIM is NULL");

    CHKRightType (WLGRID_DIM (arg_node), arg_node, "INTEGER",
                  "attribute WLGRID_DIM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_FITTED (arg_node), arg_node,
                       "mandatory attribute WLGRID_FITTED is NULL");

    CHKRightType (WLGRID_FITTED (arg_node), arg_node, "BOOL",
                  "attribute WLGRID_FITTED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_LEVEL (arg_node), arg_node,
                       "mandatory attribute WLGRID_LEVEL is NULL");

    CHKRightType (WLGRID_LEVEL (arg_node), arg_node, "INTEGER",
                  "attribute WLGRID_LEVEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_MODIFIED (arg_node), arg_node,
                       "mandatory attribute WLGRID_MODIFIED is NULL");

    CHKRightType (WLGRID_MODIFIED (arg_node), arg_node, "NODE",
                  "attribute WLGRID_MODIFIED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_NOOP (arg_node), arg_node,
                       "mandatory attribute WLGRID_NOOP is NULL");

    CHKRightType (WLGRID_NOOP (arg_node), arg_node, "BOOL",
                  "attribute WLGRID_NOOP hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRID_UNROLLING (arg_node), arg_node,
                       "mandatory attribute WLGRID_UNROLLING is NULL");

    CHKRightType (WLGRID_UNROLLING (arg_node), arg_node, "BOOL",
                  "attribute WLGRID_UNROLLING hasnt the right type");

    if (WLGRID_NEXT (arg_node) != NULL) {
        WLGRID_NEXT (arg_node) = Trav (WLGRID_NEXT (arg_node), arg_info);
    }

    if (WLGRID_NEXTDIM (arg_node) != NULL) {
        WLGRID_NEXTDIM (arg_node) = Trav (WLGRID_NEXTDIM (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlgridvar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlgridvar");

    // this son is mandatory = yes
    CHKExistChild (WLGRIDVAR_BOUND1 (arg_node), arg_node,
                   "mandatory son WLGRIDVAR_BOUND1 is NULL");

    // this son is mandatory = yes
    CHKExistChild (WLGRIDVAR_BOUND2 (arg_node), arg_node,
                   "mandatory son WLGRIDVAR_BOUND2 is NULL");

    CHKRightType (WLGRIDVAR_CODE (arg_node), arg_node, "LINK",
                  "attribute WLGRIDVAR_CODE hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRIDVAR_DIM (arg_node), arg_node,
                       "mandatory attribute WLGRIDVAR_DIM is NULL");

    CHKRightType (WLGRIDVAR_DIM (arg_node), arg_node, "INTEGER",
                  "attribute WLGRIDVAR_DIM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRIDVAR_FITTED (arg_node), arg_node,
                       "mandatory attribute WLGRIDVAR_FITTED is NULL");

    CHKRightType (WLGRIDVAR_FITTED (arg_node), arg_node, "BOOL",
                  "attribute WLGRIDVAR_FITTED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRIDVAR_LEVEL (arg_node), arg_node,
                       "mandatory attribute WLGRIDVAR_LEVEL is NULL");

    CHKRightType (WLGRIDVAR_LEVEL (arg_node), arg_node, "INTEGER",
                  "attribute WLGRIDVAR_LEVEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLGRIDVAR_NOOP (arg_node), arg_node,
                       "mandatory attribute WLGRIDVAR_NOOP is NULL");

    CHKRightType (WLGRIDVAR_NOOP (arg_node), arg_node, "BOOL",
                  "attribute WLGRIDVAR_NOOP hasnt the right type");

    if (WLGRIDVAR_BOUND1 (arg_node) != NULL) {
        WLGRIDVAR_BOUND1 (arg_node) = Trav (WLGRIDVAR_BOUND1 (arg_node), arg_info);
    }

    if (WLGRIDVAR_BOUND2 (arg_node) != NULL) {
        WLGRIDVAR_BOUND2 (arg_node) = Trav (WLGRIDVAR_BOUND2 (arg_node), arg_info);
    }

    if (WLGRIDVAR_NEXT (arg_node) != NULL) {
        WLGRIDVAR_NEXT (arg_node) = Trav (WLGRIDVAR_NEXT (arg_node), arg_info);
    }

    if (WLGRIDVAR_NEXTDIM (arg_node) != NULL) {
        WLGRIDVAR_NEXTDIM (arg_node) = Trav (WLGRIDVAR_NEXTDIM (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlseg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlseg");

    // this son is mandatory = yes
    CHKExistChild (WLSEG_CONTENTS (arg_node), arg_node,
                   "mandatory son WLSEG_CONTENTS is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_BLOCKS (arg_node), arg_node,
                       "mandatory attribute WLSEG_BLOCKS is NULL");

    CHKRightType (WLSEG_BLOCKS (arg_node), arg_node, "INTEGER",
                  "attribute WLSEG_BLOCKS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_BV (arg_node), arg_node,
                       "mandatory attribute WLSEG_BV is NULL");

    CHKRightType (WLSEG_BV (arg_node), arg_node, "INTEGERPOINTERARRAY",
                  "attribute WLSEG_BV hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_DIMS (arg_node), arg_node,
                       "mandatory attribute WLSEG_DIMS is NULL");

    CHKRightType (WLSEG_DIMS (arg_node), arg_node, "INTEGER",
                  "attribute WLSEG_DIMS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_HOMSV (arg_node), arg_node,
                       "mandatory attribute WLSEG_HOMSV is NULL");

    CHKRightType (WLSEG_HOMSV (arg_node), arg_node, "INTEGERPOINTER",
                  "attribute WLSEG_HOMSV hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_IDX_MAX (arg_node), arg_node,
                       "mandatory attribute WLSEG_IDX_MAX is NULL");

    CHKRightType (WLSEG_IDX_MAX (arg_node), arg_node, "INTEGERPOINTER",
                  "attribute WLSEG_IDX_MAX hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_IDX_MIN (arg_node), arg_node,
                       "mandatory attribute WLSEG_IDX_MIN is NULL");

    CHKRightType (WLSEG_IDX_MIN (arg_node), arg_node, "INTEGERPOINTER",
                  "attribute WLSEG_IDX_MIN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_SCHEDULING (arg_node), arg_node,
                       "mandatory attribute WLSEG_SCHEDULING is NULL");

    CHKRightType (WLSEG_SCHEDULING (arg_node), arg_node, "SCHEDULING",
                  "attribute WLSEG_SCHEDULING hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_SV (arg_node), arg_node,
                       "mandatory attribute WLSEG_SV is NULL");

    CHKRightType (WLSEG_SV (arg_node), arg_node, "INTEGERPOINTER",
                  "attribute WLSEG_SV hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_TASKSEL (arg_node), arg_node,
                       "mandatory attribute WLSEG_TASKSEL is NULL");

    CHKRightType (WLSEG_TASKSEL (arg_node), arg_node, "TASKSEL",
                  "attribute WLSEG_TASKSEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEG_UBV (arg_node), arg_node,
                       "mandatory attribute WLSEG_UBV is NULL");

    CHKRightType (WLSEG_UBV (arg_node), arg_node, "INTEGERPOINTER",
                  "attribute WLSEG_UBV hasnt the right type");

    if (WLSEG_CONTENTS (arg_node) != NULL) {
        WLSEG_CONTENTS (arg_node) = Trav (WLSEG_CONTENTS (arg_node), arg_info);
    }

    if (WLSEG_NEXT (arg_node) != NULL) {
        WLSEG_NEXT (arg_node) = Trav (WLSEG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlsegvar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlsegvar");

    // this son is mandatory = yes
    CHKExistChild (WLSEGVAR_CONTENTS (arg_node), arg_node,
                   "mandatory son WLSEGVAR_CONTENTS is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEGVAR_DIMS (arg_node), arg_node,
                       "mandatory attribute WLSEGVAR_DIMS is NULL");

    CHKRightType (WLSEGVAR_DIMS (arg_node), arg_node, "INTEGER",
                  "attribute WLSEGVAR_DIMS hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEGVAR_IDX_MAX (arg_node), arg_node,
                       "mandatory attribute WLSEGVAR_IDX_MAX is NULL");

    CHKRightType (WLSEGVAR_IDX_MAX (arg_node), arg_node, "NODEPOINTER",
                  "attribute WLSEGVAR_IDX_MAX hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEGVAR_IDX_MIN (arg_node), arg_node,
                       "mandatory attribute WLSEGVAR_IDX_MIN is NULL");

    CHKRightType (WLSEGVAR_IDX_MIN (arg_node), arg_node, "NODEPOINTER",
                  "attribute WLSEGVAR_IDX_MIN hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEGVAR_SCHEDULING (arg_node), arg_node,
                       "mandatory attribute WLSEGVAR_SCHEDULING is NULL");

    CHKRightType (WLSEGVAR_SCHEDULING (arg_node), arg_node, "SCHEDULING",
                  "attribute WLSEGVAR_SCHEDULING hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSEGVAR_TASKSEL (arg_node), arg_node,
                       "mandatory attribute WLSEGVAR_TASKSEL is NULL");

    CHKRightType (WLSEGVAR_TASKSEL (arg_node), arg_node, "TASKSEL",
                  "attribute WLSEGVAR_TASKSEL hasnt the right type");

    if (WLSEGVAR_CONTENTS (arg_node) != NULL) {
        WLSEGVAR_CONTENTS (arg_node) = Trav (WLSEGVAR_CONTENTS (arg_node), arg_info);
    }

    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        WLSEGVAR_NEXT (arg_node) = Trav (WLSEGVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlstride (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlstride");

    // this son is mandatory = yes
    CHKExistChild (WLSTRIDE_CONTENTS (arg_node), arg_node,
                   "mandatory son WLSTRIDE_CONTENTS is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_BOUND1 (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_BOUND1 is NULL");

    CHKRightType (WLSTRIDE_BOUND1 (arg_node), arg_node, "INTEGER",
                  "attribute WLSTRIDE_BOUND1 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_BOUND2 (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_BOUND2 is NULL");

    CHKRightType (WLSTRIDE_BOUND2 (arg_node), arg_node, "INTEGER",
                  "attribute WLSTRIDE_BOUND2 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_DIM (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_DIM is NULL");

    CHKRightType (WLSTRIDE_DIM (arg_node), arg_node, "INTEGER",
                  "attribute WLSTRIDE_DIM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_LEVEL (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_LEVEL is NULL");

    CHKRightType (WLSTRIDE_LEVEL (arg_node), arg_node, "INTEGER",
                  "attribute WLSTRIDE_LEVEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_MODIFIED (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_MODIFIED is NULL");

    CHKRightType (WLSTRIDE_MODIFIED (arg_node), arg_node, "LINK",
                  "attribute WLSTRIDE_MODIFIED hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_PART (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_PART is NULL");

    CHKRightType (WLSTRIDE_PART (arg_node), arg_node, "LINK",
                  "attribute WLSTRIDE_PART hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_STEP (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_STEP is NULL");

    CHKRightType (WLSTRIDE_STEP (arg_node), arg_node, "INTEGER",
                  "attribute WLSTRIDE_STEP hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDE_UNROLLING (arg_node), arg_node,
                       "mandatory attribute WLSTRIDE_UNROLLING is NULL");

    CHKRightType (WLSTRIDE_UNROLLING (arg_node), arg_node, "BOOL",
                  "attribute WLSTRIDE_UNROLLING hasnt the right type");

    if (WLSTRIDE_CONTENTS (arg_node) != NULL) {
        WLSTRIDE_CONTENTS (arg_node) = Trav (WLSTRIDE_CONTENTS (arg_node), arg_info);
    }

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        WLSTRIDE_NEXT (arg_node) = Trav (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlstridevar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlstridevar");

    // this son is mandatory = yes
    CHKExistChild (WLSTRIDEVAR_BOUND1 (arg_node), arg_node,
                   "mandatory son WLSTRIDEVAR_BOUND1 is NULL");

    // this son is mandatory = yes
    CHKExistChild (WLSTRIDEVAR_BOUND2 (arg_node), arg_node,
                   "mandatory son WLSTRIDEVAR_BOUND2 is NULL");

    // this son is mandatory = yes
    CHKExistChild (WLSTRIDEVAR_CONTENTS (arg_node), arg_node,
                   "mandatory son WLSTRIDEVAR_CONTENTS is NULL");

    // this son is mandatory = yes
    CHKExistChild (WLSTRIDEVAR_STEP (arg_node), arg_node,
                   "mandatory son WLSTRIDEVAR_STEP is NULL");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDEVAR_DIM (arg_node), arg_node,
                       "mandatory attribute WLSTRIDEVAR_DIM is NULL");

    CHKRightType (WLSTRIDEVAR_DIM (arg_node), arg_node, "INTEGER",
                  "attribute WLSTRIDEVAR_DIM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLSTRIDEVAR_LEVEL (arg_node), arg_node,
                       "mandatory attribute WLSTRIDEVAR_LEVEL is NULL");

    CHKRightType (WLSTRIDEVAR_LEVEL (arg_node), arg_node, "INTEGER",
                  "attribute WLSTRIDEVAR_LEVEL hasnt the right type");

    if (WLSTRIDEVAR_BOUND1 (arg_node) != NULL) {
        WLSTRIDEVAR_BOUND1 (arg_node) = Trav (WLSTRIDEVAR_BOUND1 (arg_node), arg_info);
    }

    if (WLSTRIDEVAR_BOUND2 (arg_node) != NULL) {
        WLSTRIDEVAR_BOUND2 (arg_node) = Trav (WLSTRIDEVAR_BOUND2 (arg_node), arg_info);
    }

    if (WLSTRIDEVAR_CONTENTS (arg_node) != NULL) {
        WLSTRIDEVAR_CONTENTS (arg_node)
          = Trav (WLSTRIDEVAR_CONTENTS (arg_node), arg_info);
    }

    if (WLSTRIDEVAR_NEXT (arg_node) != NULL) {
        WLSTRIDEVAR_NEXT (arg_node) = Trav (WLSTRIDEVAR_NEXT (arg_node), arg_info);
    }

    if (WLSTRIDEVAR_STEP (arg_node) != NULL) {
        WLSTRIDEVAR_STEP (arg_node) = Trav (WLSTRIDEVAR_STEP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CHKwlublock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CHKwlublock");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLUBLOCK_BOUND1 (arg_node), arg_node,
                       "mandatory attribute WLUBLOCK_BOUND1 is NULL");

    CHKRightType (WLUBLOCK_BOUND1 (arg_node), arg_node, "INTEGER",
                  "attribute WLUBLOCK_BOUND1 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLUBLOCK_BOUND2 (arg_node), arg_node,
                       "mandatory attribute WLUBLOCK_BOUND2 is NULL");

    CHKRightType (WLUBLOCK_BOUND2 (arg_node), arg_node, "INTEGER",
                  "attribute WLUBLOCK_BOUND2 hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLUBLOCK_DIM (arg_node), arg_node,
                       "mandatory attribute WLUBLOCK_DIM is NULL");

    CHKRightType (WLUBLOCK_DIM (arg_node), arg_node, "INTEGER",
                  "attribute WLUBLOCK_DIM hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLUBLOCK_LEVEL (arg_node), arg_node,
                       "mandatory attribute WLUBLOCK_LEVEL is NULL");

    CHKRightType (WLUBLOCK_LEVEL (arg_node), arg_node, "INTEGER",
                  "attribute WLUBLOCK_LEVEL hasnt the right type");

    // this attribute is mandatory = yes
    CHKExistAttribute (WLUBLOCK_STEP (arg_node), arg_node,
                       "mandatory attribute WLUBLOCK_STEP is NULL");

    CHKRightType (WLUBLOCK_STEP (arg_node), arg_node, "INTEGER",
                  "attribute WLUBLOCK_STEP hasnt the right type");

    if (WLUBLOCK_CONTENTS (arg_node) != NULL) {
        WLUBLOCK_CONTENTS (arg_node) = Trav (WLUBLOCK_CONTENTS (arg_node), arg_info);
    }

    if (WLUBLOCK_NEXT (arg_node) != NULL) {
        WLUBLOCK_NEXT (arg_node) = Trav (WLUBLOCK_NEXT (arg_node), arg_info);
    }

    if (WLUBLOCK_NEXTDIM (arg_node) != NULL) {
        WLUBLOCK_NEXTDIM (arg_node) = Trav (WLUBLOCK_NEXTDIM (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
