<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/11/23 11:31:45  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-key-tables.xsl"/>
<xsl:import href="common-travfun.xsl"/>
<xsl:import href="common-node-access.xsl"/>
<xsl:import href="common-name-to-nodeenum.xsl"/>

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
      <xsl:value-of select="'$Id$'"/>
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
#include "serialize_helper.h"
#include "serialize.h"
#include "stdarg.h"

  </xsl:text>
  <xsl:apply-templates select="." mode="gen-make-fun" />
  <xsl:apply-templates select="." mode="gen-fixlink-fun" />
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<xsl:template match="/" mode="gen-make-fun">
  <xsl:value-of select="'node *SHLPmakeNode( nodetype node_type, int lineno, char* sfile, ...) {'" />
  <xsl:value-of select="'node *this = ILIBmalloc( sizeof( node));'" />
  <xsl:value-of select="'va_list args;'" />
  <xsl:value-of select="'int cnt, max;'" />
  <xsl:value-of select="'this->nodetype=node_type;'" />
  <xsl:value-of select="'this->lineno=lineno;'" />
  <xsl:value-of select="'this->src_file=sfile;'" />
  <xsl:value-of select="'for (cnt=0; cnt &lt; MAX_SONS; cnt++) { this->node[cnt] = NULL; } '" />
  <xsl:value-of select="'switch (node_type) {'" />
  <xsl:apply-templates select="//syntaxtree/node" mode="gen-case" />
  <xsl:value-of select="'default: /* error */ '" />
  <xsl:value-of select="'break;'" />
  <xsl:value-of select="'} return(this);}'" />
</xsl:template>

<xsl:template match="node" mode="gen-case">
  <xsl:value-of select="'case '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="': '" />
  <xsl:apply-templates select="." mode="gen-alloc-fun" />
  <xsl:apply-templates select="." mode="gen-fill-fun" />
  <xsl:value-of select="'break;'" />
</xsl:template>

<xsl:template match="node" mode="gen-alloc-fun">
  <xsl:value-of select="'this->attribs.N_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' = ILIBmalloc(sizeof(struct ATTRIBS_N_'"/>
  <xsl:call-template name="uppercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'));'" />
</xsl:template>

<xsl:template match="node" mode="gen-fill-fun">
  <!-- check whether there was an argument -->
  <xsl:if test="sons/son | attributes/attribute">
    <xsl:value-of select="'va_start( args, sfile);'" />
    <xsl:apply-templates select="attributes/attribute" mode="gen-fill-fun" />
    <xsl:apply-templates select="sons/son" mode="gen-fill-fun" />
    <xsl:apply-templates select="flag/flag" mode="gen-fill-fun" />
    <xsl:value-of select="'va_end( args);'" />
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
    <xsl:with-param name="node">this</xsl:with-param>
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
    <xsl:with-param name="node">this</xsl:with-param>
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
  <xsl:value-of select="'= va_arg( args, '" />
  <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@ctype" />
  <xsl:value-of select="');'" />
  <!-- end of for loop in case of an array -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'}'"/>
  </xsl:if>
</xsl:template>

<xsl:template match="son" mode="gen-fill-fun">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">this</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'= va_arg( args, node*);'" />
</xsl:template>

<xsl:template match="flags/flagd" mode="gen-fill-fun" >
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">this</xsl:with-param>
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
  <xsl:value-of select="'default: break; } } }'" />
</xsl:template>

<xsl:template match="node" mode="gen-fixlink-fun">
  <xsl:value-of select="'case '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="': switch( no) {'" />
  <xsl:apply-templates select="attributes/attribute[type/@name=&quot;Link&quot;] | attributes/attribute[type/@name=&quot;DownLink&quot;]" mode="gen-fixlink-fun" />
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
