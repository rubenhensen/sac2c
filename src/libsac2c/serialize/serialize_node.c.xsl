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

<xsl:import href="../xml/common_key_tables.xsl"/>
<xsl:import href="../xml/common_travfun.xsl"/>
<xsl:import href="../xml/common_node_access.xsl"/>
<xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- 
     This stylesheet generates a serialize_node.c file implementing all
     functions needed to serialize the node structures of the ast to
     C code. 

     templates:

     traversals:

     main         generates the complete file. see templates for
                  details.
-->

<!--
     traversal main /

     The output is generated using the following layout:

     - doxygen file header
     - doxygen file group tag
     - includes
     - call to subtemplates to generate the functions itself
     - doxygen file group end tag

-->
<xsl:template match="/">
  <!-- generate file header and doxygen group -->
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'serialize_node.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'serialize'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Serialize Tree Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <!-- includes -->
  <xsl:text>

#include &lt;stdio.h&gt;
#include "serialize_node.h"
#include "serialize_attribs.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "serialize_filenames.h"
#include "tree_basic.h"
#include "traverse.h"
#include "internal_lib.h"
#include "dbug.h"

  </xsl:text>
  <!-- functions -->
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <!-- end of doxygen group -->
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<!-- 
     traversal main node

     generates a serialize function for each node type

     layout of output:

     - function head and comment
       - call templates for @name
     - function body
       - call templates for sons

     remarks:

-->
<xsl:template match="node">
  <!-- generate head and comment -->
  <xsl:apply-templates select="@name"/>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- if there is a for loop for initialising attributes, we 
         need a variable cnt, which is created here -->
  <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)][not(key(&quot;types&quot;, ./type/@name)/@persist = &quot;no&quot;)]">
    <xsl:value-of select="'int cnt;'" />
  </xsl:if>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;SET'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- DBUG PRINT -->
  <xsl:value-of select="'DBUG_PRINT( &quot;SET&quot;, (&quot;Serializing '" />
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' node&quot;));'" />
  <!-- print start of block -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, SHLPmakeNode( %d, %d, FILENAME( %d) &quot;, '" />
  <!-- generate nodetype argument -->
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', NODE_LINE( arg_node), SFNgetId( NODE_FILE( arg_node)));'" /> 
  <!-- print generators of all arguments -->
  <xsl:apply-templates select="." mode="gen-values"/>
  <!-- print end of block -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;)&quot;);'"/>
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( arg_node);'"/>
  <!-- end of body -->
  <xsl:value-of select="'}'"/>
</xsl:template>

<!--
     traversal main @name

     generates a comment and function head

     layout:
   
     - call travfun-comment template
     - call travfun-head template
-->

<xsl:template match="@name">
  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">SET</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
    <xsl:with-param name="text">Creates C code for this node type</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">SET</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
  </xsl:call-template>
</xsl:template>

<!--
     traversal gen-values node

     layout:

-->
  
<xsl:template match="node[@name = &quot;Fundef&quot;]" mode="gen-values">
  <!-- generate all sons and attributes for Fundef node -->
  <xsl:apply-templates select="attributes/attribute" mode="gen-values"/>
  <xsl:apply-templates select="sons/son" mode="gen-values-fundef"/>
  <xsl:apply-templates select="flags" mode="gen-values"/>
</xsl:template>

<xsl:template match="node[@name = &quot;Typedef&quot;]" mode="gen-values">
  <!-- generate all sons and attributes for Fundef node -->
  <xsl:apply-templates select="attributes/attribute" mode="gen-values"/>
  <xsl:apply-templates select="sons/son" mode="gen-values-nonext"/>
  <xsl:apply-templates select="flags" mode="gen-values"/>
</xsl:template>

<xsl:template match="node[@name = &quot;Objdef&quot;]" mode="gen-values">
  <!-- generate all sons and attributes for Fundef node -->
  <xsl:apply-templates select="attributes/attribute" mode="gen-values"/>
  <xsl:apply-templates select="sons/son" mode="gen-values-nonext"/>
  <xsl:apply-templates select="flags" mode="gen-values"/>
</xsl:template>

<xsl:template match="node" mode="gen-values">
  <!-- generate all sons and attributes -->
  <xsl:apply-templates select="attributes/attribute" mode="gen-values"/>
  <xsl:apply-templates select="sons/son" mode="gen-values"/>
  <xsl:apply-templates select="flags" mode="gen-values"/>
</xsl:template>

<!--
     template gen-values attribute

     generates the value of an attribute, if it has persist = yes (default).
     All other attributes are ignored, as they will be set to their
     default values lateron. see ast2serialize_helper_c.xsl for details.
-->

<xsl:template match="attributes/attribute[not( key(&quot;types&quot;, ./type/@name)/@persist = &quot;no&quot;)]" mode="gen-values">
  <!-- if it is an array, we have to build a for loop over its elements -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
    <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
    <xsl:value-of select="'; cnt++) { '" />
  </xsl:if>
  <!-- make it print a , -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, &quot;);'" />
  <!-- call serialization function for attribute -->
  <xsl:value-of select="'SATserialize'" />
  <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@name" />
  <xsl:value-of select="'( arg_info, '" />
  <!-- if it is an array, we need to pass the selector as well -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'cnt, '" />
  </xsl:if>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
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
  <xsl:value-of select="', arg_node);'" />
  <!-- if it is an array, we have to complete the for loop -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'}'"/>
  </xsl:if>
</xsl:template>

<!--
     template gen-values attribute

     all attributes that are not matched somewhere else
-->

<xsl:template match="attributes/attribute[key(&quot;types&quot;, ./type/@name)/@persist = &quot;no&quot;]" mode="gen-values" />

<xsl:template match="son[@name = &quot;Body&quot;]" mode="gen-values-fundef">
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, NULL&quot;);'" />
</xsl:template>

<xsl:template match="son" mode="gen-values-fundef">
  <xsl:apply-templates select="." mode="gen-values-nonext" />
</xsl:template>

<xsl:template match="son[@name = &quot;Next&quot;]" mode="gen-values-nonext">
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, NULL&quot;);'" />
</xsl:template>

<xsl:template match="son" mode="gen-values-nonext">
  <xsl:apply-templates select="." mode="gen-values" />
</xsl:template>

<xsl:template match="son" mode="gen-values">
  <!-- check for NULL pointer son -->
  <xsl:value-of select="'if ( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' == NULL)'" />
  <!-- generate code for a NULL pointer son -->
  <xsl:value-of select="'{ fprintf( INFO_SER_FILE( arg_info), &quot;, NULL&quot;); }'" />
  <!-- call Trav otherwise -->
  <xsl:value-of select="'else { TRAVdo( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', arg_info); }'" />
</xsl:template>

<xsl:template match="flags/flag" mode="gen-values">
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, %d&quot;, '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="');'" />
</xsl:template>

</xsl:stylesheet>
