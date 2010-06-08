<?xml version="1.0"?>

<!--
 ***********************************************************************
 *                                                                     *
 *                      Copyright (c) 1994-2007                        *
 *         SAC Research Foundation (http://www.sac-home.org/)          *
 *                                                                     *
 *                        All Rights Reserved                          *
 *                                                                     *
 *   The copyright holder makes no warranty of any kind with respect   *
 *   to this product and explicitly disclaims any implied warranties   *
 *   of merchantability or fitness for any particular purpose.         *
 *                                                                     *
 ***********************************************************************
 -->

<!--  $Id$  -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">
  
  <xsl:import href="../xml/common_travfun.xsl"/>
  <xsl:import href="../xml/common_node_access.xsl"/>
  <xsl:import href="../xml/common_c_code.xsl"/>
  <xsl:import href="../xml/common_key_tables.xsl"/>
  
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
        <xsl:value-of select="'$Id$'"/>
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

#include "check_reset.h"
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"


/*****************************************************************************
 *
 * @fn node *CHKRSTdoTreeCheckReset( node *syntax_tree)
 *
 *****************************************************************************/
node *CHKRSTdoTreeCheckReset( node *arg_node)
{
  node *keep_next;

  DBUG_ENTER( "CHKRSTdoTreeCheckReset");

  DBUG_ASSERT( (NODE_TYPE( arg_node) == N_module)
               || (NODE_TYPE( arg_node) == N_fundef),
               "Illegal argument node!");

  DBUG_ASSERT( (NODE_TYPE( arg_node) == N_module)
               || global.local_funs_grouped ,
               "If run fun-based, special funs must be grouped.");

  if (NODE_TYPE( arg_node) == N_fundef) {
    /* 
     * If this check is called function-based, we do not want to traverse
     * into the next fundef, but restrict ourselves to this function and
     * its subordinate special functions.
     */
    keep_next = FUNDEF_NEXT( arg_node);
    FUNDEF_NEXT( arg_node) = NULL;
  }

  DBUG_PRINT( "CHKRST", ("Reset tree check mechanism"));

  TRAVpush( TR_chkrst);
  arg_node = TRAVdo( arg_node, NULL);
  TRAVpop();

  DBUG_PRINT( "CHKRST", ("Reset tree check mechanism completed"));

  if (NODE_TYPE( arg_node) == N_fundef) {
    /* 
     * If this check is called function-based, we must restore the original
     * fundef chain here.
     */
    FUNDEF_NEXT( arg_node) = keep_next;
  }
  
  DBUG_RETURN( arg_node);
}
  </xsl:text>
  <xsl:value-of select="$newline"/>
  <xsl:apply-templates select="//syntaxtree/node" mode="function">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
</xsl:template>


<xsl:template match="node" mode="function">
  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">CHKRST</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
    <xsl:with-param name="text">build the testenviroment for the Checks</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">CHKRST</xsl:with-param>
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
    <xsl:value-of select="'( &quot;CHKRST'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'&quot;);'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="$newline"/>

    <xsl:value-of select="'  NODE_CHECKVISITED( arg_node) = FALSE;'"/>
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
