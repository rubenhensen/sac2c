<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/08/08 16:07:21  sah
  beautified generated code
  include more doxygen comments

  Revision 1.1  2004/08/07 16:22:04  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-key-tables.xsl"/>
<xsl:import href="common-travfun.xsl"/>
<xsl:import href="common-node-access.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a free_node.c file implementing all
     functions needed to free a node -->

<xsl:template match="/">
  <!-- generate file header and doxygen group -->
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'free_node.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by free traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'free'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Free Tree Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by free traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#include "free_node.h"
#include "free_attribs.h"
#include "tree_basic.h"
#include "traverse.h"
#include "internal_lib.h"
#include "dbug.h"

#define AST_NO_COMPAT
#include "node_compat.h"

#define FREETRAV( node, info) Trav( node, info)

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>
#undef AST_NO_COMPAT
#include "node_compat.h"
  </xsl:text>
  <!-- end of doxygen group -->
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<xsl:template match="node">
  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">Free</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
    <xsl:with-param name="text">Frees the node and its sons/attributes</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">Free</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- if there is a for loop for initialising attributes, we 
       need a variable cnt, which is created here -->
  <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)]">
    <xsl:value-of select="'int cnt;'" />
  </xsl:if>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;Free'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- call free for sons -->
  <xsl:apply-templates select="sons/son"/>
  <!-- call free for attributes -->
  <xsl:apply-templates select="attributes/attribute"/>
  <!-- free attribute structure -->
  <xsl:value-of select="'arg_node->attribs.N_'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' = Free( arg_node->attribs.N_'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="');'"/>
  <!-- free the node itself -->
  <xsl:value-of select="'arg_node = Free( arg_node);'"/>
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( arg_node);'"/>
  <!-- end of body -->
  <xsl:value-of select="'}'"/>
</xsl:template>

<!-- free all sons -->
<xsl:template match="son">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = FREETRAV( '"/>
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
</xsl:template>

<!-- call free functions for attributes -->
<xsl:template match="attribute">
  <xsl:choose>
    <!-- literal attributes are ignored -->
    <xsl:when test="key(&quot;types&quot;, ./type/@name)[@copy = &quot;literal&quot;]">
      <!-- do nothing -->
    </xsl:when>
    <xsl:otherwise>
      <!-- if it is an array, we have to build a for loop over its elements -->
      <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
        <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
        <xsl:value-of select="'; cnt++) { '" />
      </xsl:if>
      <!-- left side of assignment -->
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
        <!-- if its is an array, we have to add another parameter -->
        <xsl:with-param name="index">
          <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
            <xsl:value-of select="'cnt'"/>
          </xsl:if>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="' = '" />
      <!-- right side of assignment -->
      <xsl:value-of select="'Free'"/>
      <xsl:value-of select="./type/@name"/>
      <xsl:value-of select="'('"/>
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
        <!-- if its is an array, we have to add another parameter -->
        <xsl:with-param name="index">
          <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
            <xsl:value-of select="'cnt'"/>
          </xsl:if>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="');'"/>
      <!-- if it is an array, we have to complete the for loop -->
      <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="'}'"/>
      </xsl:if>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>
