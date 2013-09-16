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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_key_tables.xsl"/>
<xsl:import href="../xml/common_travfun.xsl"/>
<xsl:import href="../xml/common_node_access.xsl"/>
<xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- 
     This stylesheet generates some helper functions for the
     deserialization process. 

     MakeNode to allocate and fill an empty node structure

     FixLink to fix links between nodes

     templates:

     traversals:

     main         generates the complete file. see templates for
                  details.
-->

<!--
     traversal main /

-->

<xsl:template match ="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'serialize_helper.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by de-serialization code.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'serialize_helper.c.xsl'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'serializehelper'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'De-Serialize Helper Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by de-serialize code.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <!-- includes etc. -->
  <xsl:text>

#include "types.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "node_alloc.h"
#include "serialize.h"
#include "stdarg.h"
#include "check_mem.h"
#include "serialize_stack.h"
#include "serialize_helper.h"

  </xsl:text>
  <xsl:apply-templates select="." mode="gen-make-fun" />
  <xsl:apply-templates select="." mode="gen-fixlink-fun" />
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<xsl:template match="/" mode="gen-make-fun">
  <!-- function added for cygwin compat -->
  <xsl:text>
#if IS_CYGWIN
node *
SHLPmakeNode( int _node_type, char* sfile, size_t lineno, size_t col ...)
{
  node *result;
  va_list Argp;

  va_start (Argp, sfile);
  result = SHLPmakeNodeVa( _node_type, sfile, lineno, col, Argp);
  va_end(Argp);
  
  return( result);
}
  </xsl:text>
  <xsl:value-of select="'node *SHLPmakeNodeVa (int _node_type, char* sfile, size_t lineno, size_t col, va_list args) {'" />
  <xsl:text>
#else
  </xsl:text>
  <xsl:value-of select="'node *SHLPmakeNode (int _node_type, char* sfile, size_t lineno, size_t col, ...) {'" />
  <xsl:value-of select="'va_list args;'" />
  <xsl:text>
#endif /* IS_CYGWIN */
  </xsl:text>
  <xsl:value-of select="'nodetype node_type = (nodetype)_node_type;'" />
  <xsl:value-of select="'node *xthis = NULL;'" />
  <xsl:if test="key(&quot;arraytypes&quot;, //syntaxtree//type/@name)">
    <xsl:value-of select="'int cnt, max;'" />
  </xsl:if>
  <xsl:value-of select="'switch (node_type) {'" />
  <xsl:apply-templates select="//syntaxtree/node" mode="gen-case" />
  <xsl:value-of select="'default: /* error */ '" />
  <xsl:value-of select="'break;'" />
  <xsl:value-of select="'} '" />
  <xsl:value-of select="'return(xthis);}'" />
</xsl:template>

<xsl:template match="node" mode="gen-case">
  <xsl:value-of select="'case '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="': { '" />
  <xsl:apply-templates select="." mode="gen-alloc-fun" />
  <xsl:apply-templates select="." mode="gen-fill-fun" />
  <xsl:value-of select="'break; }'" />
</xsl:template>

<xsl:template match="node" mode="gen-alloc-fun">
  <!-- temp variable -->
  <xsl:variable name="uppername">
    <xsl:call-template name="uppercase" >
      <xsl:with-param name="string" >
	<xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
  </xsl:variable>

  <xsl:value-of select="'struct NODE_ALLOC_N_'" />
  <xsl:value-of select="$uppername" />
  <xsl:value-of select="' *nodealloc; '" />
  <!-- allocate structure -->
  <xsl:value-of select="'nodealloc = (struct NODE_ALLOC_N_'" />
  <xsl:value-of select="$uppername" />
  <xsl:value-of select="' *) MEMmalloc( sizeof( struct NODE_ALLOC_N_'" />
  <xsl:value-of select="$uppername" />
  <xsl:value-of select="'));'" />
  <!-- set basic info -->
  <xsl:value-of select="'xthis = (node *) &amp;(nodealloc->nodestructure);'" />
  <xsl:value-of select="'NODE_TYPE( xthis) = node_type;'" />
  <xsl:value-of select="'NODE_FILE( xthis) = sfile;'" />
  <xsl:value-of select="'NODE_LINE( xthis) = lineno;'" />
  <xsl:value-of select="'NODE_COL( xthis) = col;'" />
  <xsl:value-of select="'NODE_ERROR( xthis) = NULL;'" />
  <xsl:call-template name="newline" />
  <xsl:text>
#ifndef DBUG_OFF
  </xsl:text>
  <xsl:value-of select="'CHKMisNode (xthis, node_type);'" />
  <xsl:text>
#endif
  </xsl:text>
  <xsl:call-template name="newline" />
  <!-- set sons and attribs types -->
  <xsl:if test="sons/son">
    <xsl:value-of select="'xthis->sons.'" />
    <xsl:call-template name="name-to-nodeenum" >
      <xsl:with-param name="name" select="@name" />
    </xsl:call-template>
    <xsl:value-of select="' = (struct SONS_N_'" />
    <xsl:value-of select="$uppername" />
    <xsl:value-of select="' *) &amp;(nodealloc->sonstructure); '"/>
  </xsl:if>
  <xsl:if test="attributes/attribute | flags/flag">
    <xsl:value-of select="'xthis->attribs.'" />
    <xsl:call-template name="name-to-nodeenum" >
      <xsl:with-param name="name" select="@name" />
    </xsl:call-template>
    <xsl:value-of select="' = (struct ATTRIBS_N_'" />
    <xsl:value-of select="$uppername" />
        <xsl:value-of select="' *) &amp;(nodealloc->attributestructure);'" />
  </xsl:if>
</xsl:template>

<xsl:template match="node" mode="gen-fill-fun">
  <!-- check whether there was an argument -->
  <xsl:if test="sons/son | attributes/attribute">
    <xsl:text>
#if !IS_CYGWIN
    </xsl:text>
    <xsl:value-of select="'va_start (args, col);'" />
    <xsl:text>
#endif
    </xsl:text>
    <xsl:apply-templates select="attributes/attribute" mode="gen-fill-fun" />
    <xsl:apply-templates select="sons/son" mode="gen-fill-fun" />
    <xsl:apply-templates select="flags/flag" mode="gen-fill-fun" />
    <xsl:text>
#if !IS_CYGWIN
    </xsl:text>
    <xsl:value-of select="'va_end (args);'" />
    <xsl:text>
#endif
    </xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template match="attribute[key( &quot;types&quot;, ./type/@name)/@persist = &quot;no&quot;]" mode="gen-fill-fun">
  <!-- in case of an array, we have to iterate in a for loop -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <!-- first fetch size of the array -->
    <xsl:value-of select="'max = va_arg( args, int);'" />
    <!-- start for loop -->
    <xsl:value-of select="'for( cnt=0; cnt &lt; max; cnt++) {'" />
  </xsl:if>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">xthis</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
    <!-- if its is an array, we have to add another parameter -->
    <xsl:with-param name="index">
      <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="'cnt'"/>
      </xsl:if>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'= '" />
  <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@init" />
  <xsl:value-of select="';'" />
  <!-- end of for loop in case of an array -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'}'"/>
  </xsl:if>
</xsl:template>

<xsl:template match="attribute" mode="gen-fill-fun">
  <!-- in case of an array, we have to iterate in a for loop -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <!-- first fetch size of the array -->
    <xsl:value-of select="'max = va_arg( args, int);'" />
    <!-- start for loop -->
    <xsl:value-of select="'for( cnt=0; cnt &lt; max; cnt++) {'" />
  </xsl:if>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">xthis</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
    <!-- if its is an array, we have to add another parameter -->
    <xsl:with-param name="index">
      <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="'cnt'"/>
      </xsl:if>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = '" />
  <!-- generate right hand side of assignment -->
  <xsl:apply-templates select="key(&quot;types&quot;, ./type/@name)"
                       mode="gen-fill-fun" />
  <xsl:value-of select="';'" />
  <!-- end of for loop in case of an array -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'}'"/>
  </xsl:if>
</xsl:template>

<xsl:template match="//attributetypes/type[@vtype]" mode="gen-fill-fun">
  <xsl:value-of select="'va_arg( args, '" />
  <xsl:value-of select="@vtype" />
  <xsl:value-of select="')'" />
</xsl:template>

<xsl:template match="//attributetypes/type[not(@vtype)]" mode="gen-fill-fun">
  <xsl:value-of select="'va_arg( args, '" />
  <xsl:value-of select="@ctype" />
  <xsl:value-of select="')'" />
</xsl:template>

<xsl:template match="son" mode="gen-fill-fun">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">xthis</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'= va_arg( args, node*);'" />
</xsl:template>

<xsl:template match="flags/flag" mode="gen-fill-fun" >
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">xthis</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'= va_arg( args, int);'" />
</xsl:template>
  

<xsl:template match="/" mode="gen-fixlink-fun">
  <xsl:value-of select="'void SHLPfixLink( serstack_t *stack, int from, int no, int to) {'" />
  <xsl:value-of select="'node *fromp = NULL;node *top = NULL;'" />
  <xsl:value-of select="'if ( from != SERSTACK_NOT_FOUND ) {'" />
  <xsl:value-of select="'fromp = SSlookup( from, stack);'" />
  <xsl:value-of select="'if ( to != SERSTACK_NOT_FOUND) {'" />
  <xsl:value-of select="'top = SSlookup( to, stack); }'" />
  <xsl:value-of select="'switch (NODE_TYPE( fromp)) {'" />
  <xsl:apply-templates select="//syntaxtree/node" mode="gen-fixlink-fun" />
  <xsl:value-of select="'default: break; } }'" />
  <xsl:value-of select="'}'" />

</xsl:template>

<xsl:template match="node" mode="gen-fixlink-fun">
  <xsl:value-of select="'case '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="': switch( no) {'" />
  <xsl:apply-templates select="attributes/attribute[type/@name=&quot;Link&quot;] | attributes/attribute[type/@name=&quot;CodeLink&quot;]" mode="gen-fixlink-fun" />
  <xsl:value-of select="'default: break;'" />
  <xsl:value-of select="'} break;'" />
</xsl:template>

<xsl:template match="attributes/attribute" mode="gen-fixlink-fun">
  <xsl:value-of select="'case '" />
  <xsl:value-of select="position()" />
  <xsl:value-of select="': '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'fromp'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
      </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = top;'" />
  <xsl:value-of select="'break;'" />
</xsl:template>

</xsl:stylesheet>
