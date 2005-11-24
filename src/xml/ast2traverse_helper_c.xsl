<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.5  2005/06/28 16:17:07  sah
  removed a warning message

  Revision 1.4  2004/11/27 03:00:31  sah
  bugfix

  Revision 1.3  2004/11/26 00:20:39  sah
  fixed bug

  Revision 1.2  2004/11/25 23:49:26  sah
  added ugly funs for CMPtree

  Revision 1.1  2004/11/24 17:55:56  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="common-travfun.xsl"/>
  <xsl:import href="common-key-tables.xsl"/>
  <xsl:import href="common-name-to-nodeenum.xsl"/>
  <xsl:import href="common-node-access.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'traverse_helper.c'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'Defines the helper function needed by the traversal system'" />
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id$'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>

#include "traverse_helper.h"
#include "dbug.h"
#include "tree_basic.h"
#include "traverse.h"

#define TRAV( son, info)    if (son != NULL) { son = TRAVdo( son, info); }

node *TRAVnone(node *arg_node, info *arg_info)
{
   return(arg_node);
}

node *TRAVerror(node *arg_node, info *arg_info)
{
  DBUG_ASSERT( (FALSE), "Illegal node type found.");

  return( arg_node);
}

node *TRAVsons(node *arg_node, info *arg_info)
{ 
  switch (NODE_TYPE( arg_node)) {
  </xsl:text>
  <xsl:apply-templates select="/definition/syntaxtree" mode="travsons" />
  <xsl:text>
    default:
      DBUG_ASSERT( (FALSE), 
         "Illegal nodetype found!" );
      break;
  }

  return( arg_node);
}

int TRAVnumSons( node *node)
{
  int result = 0;

  DBUG_ENTER("TRAVnumSons");

  switch (NODE_TYPE( node)) {
  </xsl:text>
  <xsl:apply-templates select="/definition/syntaxtree" mode="travnumsons" />
  <xsl:text>
    default:
      DBUG_ASSERT( (FALSE),
         "Illegal nodetype found!" );
      break;
  }

  DBUG_RETURN( result);
}

node *TRAVgetSon( int no, node *parent)
{
  node * result = NULL;

  DBUG_ENTER("TRAVgetSon");

  switch (NODE_TYPE( parent)) {
  </xsl:text>
  <xsl:apply-templates select="/definition/syntaxtree" mode="travgetson" />
  <xsl:text>
    default:
      DBUG_ASSERT( (FALSE),
         "Illegal nodetype found!" );
      break;
  }

  DBUG_RETURN( result);
}
    </xsl:text>
  </xsl:template>

  <xsl:template match="node" mode="travsons" >
    <xsl:value-of select="'case '" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name" select="@name" />
    </xsl:call-template>
    <xsl:value-of select="': '" />
      <xsl:apply-templates select="sons/son" mode="travsons" />
    <xsl:value-of select="'break;'" />
  </xsl:template>

  <xsl:template match="sons/son" mode="travsons" >
    <xsl:value-of select="'TRAV( '" />
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">
        <xsl:value-of select="'arg_node'" />
      </xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name" />
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="', arg_info);'" />
  </xsl:template>

  <xsl:template match="node" mode="travnumsons" >
    <xsl:value-of select="'case '" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name" select="@name" />
    </xsl:call-template>
    <xsl:value-of select="': '" />
    <xsl:value-of select="'result = '" />
    <xsl:value-of select="count( sons/son)" />
    <xsl:value-of select="';'" />
    <xsl:value-of select="'break;'" />
  </xsl:template>

  <xsl:template match="node" mode="travgetson" >
    <xsl:value-of select="'case '" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name" select="@name" />
    </xsl:call-template>
    <xsl:value-of select="': switch (no) { '" />
    <xsl:apply-templates select="sons/son" mode="travgetson" />
    <xsl:value-of select="'default: DBUG_ASSERT( (FALSE), &quot;index out of range!&quot;); break; } break;'" />
  </xsl:template>

  <xsl:template match="son" mode="travgetson" >
    <xsl:value-of select="'case '" />
    <xsl:value-of select="position()-1" />
    <xsl:value-of select="': result = '" />
    <xsl:call-template name="node-access">
      <xsl:with-param name="node" select="'parent'" />
      <xsl:with-param name="nodetype" select="../../@name" />
      <xsl:with-param name="field" select="@name" />
    </xsl:call-template>
    <xsl:value-of select="'; break;'" />
  </xsl:template>

</xsl:stylesheet>
