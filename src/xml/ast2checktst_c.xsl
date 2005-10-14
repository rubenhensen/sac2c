<?xml version="1.0"?>

<!--
  $Log$
  Revision 1.2  2005/07/13 12:10:13  jhb
  bug fixed

  Revision 1.1  2005/06/06 10:14:55  jhb
  Initial revision

  Revision 1.1  2005/05/19 13:34:02  jhb
  added the rangequery for the attributes

  Revision 1.1 2004/09/29 15:15:00 jhb
  Initial revision
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">
  
  <xsl:import href="common-travfun.xsl"/>
  <xsl:import href="common-node-access.xsl"/>
  <xsl:import href="common-c-code.xsl"/>
  <xsl:import href="common-key-tables.xsl"/>
  
  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>
  
  <!-- This stylesheet generates a checktst.c file implementing all functions needed to            check a node -->
  
  <xsl:variable name="newline">
    <xsl:text>
    </xsl:text>
  </xsl:variable>
  
  <xsl:template match="/">
    <!-- generate file header and doxygen group -->
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'checktst.c'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'Functions needed by test check environment.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id: ast2checktst_c.xsl 14294 2005-10-10 12:40:03Z sah $'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:call-template name="travfun-group-begin">
      <xsl:with-param name="group">
        <xsl:value-of select="'checktst'"/>
      </xsl:with-param>
      <xsl:with-param name="name">
        <xsl:value-of select="'Test of the Check tree Functions'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'Functions needed by checktst traversal.'"/>
      </xsl:with-param>
    </xsl:call-template>
    <!-- includes -->
    <xsl:text>

#define NEW_INFO

#include "checktst.h"
#include "check.h"
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "check_lib.h"
#include "types.h"

#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "internal_lib.h"



struct INFO
{
};

static info *MakeInfo()
{
  info *result;

  DBUG_ENTER("MakeInfo");

  result = ILIBmalloc(sizeof(info));

  DBUG_RETURN(result);
} 

static info *FreeInfo(info *info)
{
  DBUG_ENTER("FreeInfo");

  info = ILIBfree(info);

  DBUG_RETURN(info);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKTSTdoTreeCheckTest( node *syntax_tree)
 *
 *****************************************************************************/
node *CHKTSTdoTreeCheckTest( node *syntax_tree)
{
  info *info;

  DBUG_ENTER( "CHKTSTdoTreeCheckTest");

  DBUG_PRINT( "CHKTST", ("Starting the CheckTestmechanism"));

  info = MakeInfo();

  TRAVpush( TR_chktst);
  syntax_tree = TRAVdo( syntax_tree, info);
  TRAVpop();

  info = FreeInfo( info);

  DBUG_PRINT( "CHKTST", ("CheckTestmechanism complete"));

  DBUG_RETURN( syntax_tree);
}
  </xsl:text>
  <xsl:value-of select="$newline"/>
  <xsl:apply-templates select="//syntaxtree/node" mode="function">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
</xsl:template>


<xsl:template match="node" mode="function">
  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">CHKTST</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
    <xsl:with-param name="text">build the testenviroment for the Checks</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">CHKTST</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="$newline"/>   
  <xsl:value-of select="'{'"/>  
  <xsl:value-of select="$newline"/>
    <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)]">
      <xsl:value-of select="$newline"/>
    </xsl:if>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  DBUG_ENTER'"/>
    <xsl:value-of select="'( &quot;CHKTST'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'&quot;);'"/>
    <xsl:value-of select="$newline"/>

    <xsl:apply-templates select="./sons/son" mode="trav">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>

    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  DBUG_RETURN( arg_node);'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'}'"/>
    <xsl:value-of select="$newline"/>
  </xsl:template>

  
  <!-- trav to the sons -->
  <xsl:template match="son" mode="trav">
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  if ( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' != NULL)'"/>
    <xsl:value-of select="' {'"/>
    <xsl:value-of select="$newline"/>
    <xsl:text>    </xsl:text>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'= TRAVdo( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="', arg_info);'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  }'"/>
    <xsl:value-of select="$newline"/>
  </xsl:template>
  
  
</xsl:stylesheet>