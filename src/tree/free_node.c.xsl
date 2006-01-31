<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.9  2005/09/29 22:04:36  sah
  removed unused special cases for INT_ASSIGN
  and cleaned up old code

  Revision 1.8  2005/06/06 13:17:20  jhb
  added handling of NODE_ERROR

  Revision 1.7  2005/03/10 09:41:09  cg
  Added some necessary includes.

  Revision 1.6  2005/03/04 21:21:42  cg
  Setting of zombie flag is hidden behind function FREEzombify.
  This allows us to recall the existence of zombie functions
  in a static global varibale in free.c and to check for removal
  of zombie functions only if some actually exist.

  Revision 1.5  2005/01/08 09:55:24  ktr
  Fixed some issues related to loops.

  Revision 1.4  2004/12/09 18:53:05  sah
  extended signature of free functions for
  attributes so that they contain a link
  to the parent node, now

  Revision 1.3  2004/11/28 22:12:49  ktr
  NEXT is rescued now before removing sons union.

  Revision 1.2  2004/11/25 22:00:41  sah
  ...

  Revision 1.1  2004/11/23 11:30:11  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common-key-tables.xsl"/>
<xsl:import href="../xml/common-travfun.xsl"/>
<xsl:import href="../xml/common-node-access.xsl"/>
<xsl:import href="../xml/common-name-to-nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- 
     This stylesheet generates a free_node.c file implementing all
     functions needed to free a specific node

     templates:

     traversals:

     main         generates the complete file. see templates for
                  details.
-->

<!--
     traversal main /

     The output is generated using the following layout:

     - doxygen file header
     - doxugen file group tag
     - includes
     - call to subtemplates to generate the functions itself
     - doxygen file group end tag

-->
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
  <!-- includes -->
  <xsl:text>

#include "free.h"
#include "free_node.h"
#include "free_attribs.h"
#include "free_info.h"
#include "tree_basic.h"
#include "traverse.h"
#include "internal_lib.h"
#include "dbug.h"

#define FREETRAV( node, info) (node != NULL) ? TRAVdo( node, info) : node
#define FREECOND( node, info)                                    \
  (INFO_FREE_FLAG( info) != arg_node)                            \
    ? FREETRAV( node, info)                                      \
    : (node)

  </xsl:text>
  <!-- functions -->
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <!-- end of doxygen group -->
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<!-- 
     traversal main node[@name = "Fundef"]

     generates a free functions for Fundef nodes

     layout of output:

     - function head and comment
       - call templates for @name
     - function body
       - call templates for sons
       - call templates for attributes except those needed for zombies

     remarks:

     see generic template.

     Fundef nodes are never freed. Instead, they are zombiealised, thus
     their status is set to zombie and all attributes and sons are
     freed, except for NAME, MOD, LINKMOD, TYPE and TYPES. Furthermore,
     the node structure itself is not freed. This has to be done by a
     cal of FreeAllZombies.
     
-->
<xsl:template match="node[@name = &quot;Fundef&quot;]">
  <!-- generate head and comment -->
  <xsl:apply-templates select="@name"/>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- if there is a for loop for initialising attributes, we 
       need a variable cnt, which is created here -->
  <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)]">
    <xsl:value-of select="'int cnt;'" />
  </xsl:if>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;FREE'"/>
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'&quot;);'"/>
  <!-- set status of Fundef to zombie -->
  <xsl:value-of select="'DBUG_PRINT(&quot;FREE&quot;, (&quot;transforming %s at &quot; F_PTR &quot; into a zombie&quot;, '"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">Name</xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', arg_node));'"/>
  <xsl:value-of select="'arg_node = FREEzombify( arg_node);'"/>
  <!-- first free everything downwards in the ast -->
  <xsl:apply-templates select="sons/son[@name = &quot;Next&quot;]"/>
  <!-- free all attributes, except NAME, MOD, LINKMOD. IMPL, and TYPES --> 
  <xsl:apply-templates select="attributes/attribute[@name != &quot;Name&quot;][@name != &quot;Mod&quot;][@name!=&quot;LinkMod&quot;][@name != &quot;Types&quot;][@name != &quot;Type&quot;][@name!=&quot;Impl&quot;]"/>
  <!-- call free for all other sons -->
  <xsl:apply-templates select="sons/son[not( @name= &quot;Next&quot;)]"/>
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( arg_node);'"/>
  <!-- end of body -->
  <xsl:value-of select="'}'"/>
</xsl:template>

<!--
     traversal main node

     generates a generic free function for any node

     layout of output:

     - function head and comment
       - call templates for @name
     - function body
       - call templates for sons
       - call templates for attributes
     
     remarks:

     the body contains calls to free for all nodes and attributes.
     For each attribute a unique free function is called. This function
     has to decide whether to free an attribute or not. This includes
     a test for non-null if the attribute is a pointer type!
     Attribute arrays are iterated by this function, thus the free
     function for array attributes has to free one element only!

     The return value is the value of the NEXT son, or if no NEXT son
     is present the result of Free. This way, depending on the 
     TRAVCOND macro, the full chain of nodes or only one node can
     be freed.
-->

<xsl:template match="node">
  <!-- generate head and comment -->
  <xsl:apply-templates select="@name"/>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- if there is a for loop for initialising attributes, we 
       need a variable cnt, which is created here -->
  <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)]">
    <xsl:value-of select="'int cnt;'" />
  </xsl:if>
  <!-- variable for result -->
  <xsl:value-of select="'node *result = NULL;'"/>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;FREE'"/>
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'&quot;);'"/>
  <!-- give hint we start to free now -->
  <xsl:value-of select="'DBUG_PRINT( &quot;FREE&quot;, (&quot;Processing node %s at &quot; F_PTR, NODE_TEXT( arg_node), arg_node));'"/>
  <xsl:value-of select="'NODE_ERROR( arg_node) = FREECOND( NODE_ERROR( arg_node), arg_info);'"/>
  <!-- first free everything downwards in the ast -->
  <xsl:apply-templates select="sons/son[@name = &quot;Next&quot;]"/>
  <!-- call free for attributes -->
  <xsl:apply-templates select="attributes/attribute"/>
  <!-- call free for all other sons -->
  <xsl:apply-templates select="sons/son[not( @name= &quot;Next&quot;)]"/>
  <!-- rescue NEXT element -->
  <xsl:choose>
    <xsl:when test="sons/son[@name = &quot;Next&quot;]">
      <xsl:value-of select="'result = '"/>
      <xsl:call-template name="node-access">
        <xsl:with-param name="node">arg_node</xsl:with-param>
        <xsl:with-param name="nodetype">
          <xsl:value-of select="@name"/>
        </xsl:with-param>
        <xsl:with-param name="field">Next </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="';'" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="'result = NULL;'"/>
    </xsl:otherwise>
  </xsl:choose>
  <!-- free sons structure -->
  <xsl:value-of select="'arg_node->sons.'"/>
  <xsl:call-template name="name-to-nodeenum" >
    <xsl:with-param name="name" select="@name"/>
  </xsl:call-template>
  <xsl:value-of select="' = ILIBfree( arg_node->sons.'"/>
  <xsl:call-template name="name-to-nodeenum" >
    <xsl:with-param name="name" select="@name"/>
  </xsl:call-template>
  <xsl:value-of select="');'"/>
  <!-- free attribute structure -->
  <xsl:value-of select="'arg_node->attribs.'"/>
  <xsl:call-template name="name-to-nodeenum" >
    <xsl:with-param name="name" select="@name"/>
  </xsl:call-template>
  <xsl:value-of select="' = ILIBfree( arg_node->attribs.'"/>
  <xsl:call-template name="name-to-nodeenum" >
    <xsl:with-param name="name" select="@name"/>
  </xsl:call-template>
  <xsl:value-of select="');'"/>
  <!-- calculate return value and free node -->
  <xsl:value-of select="'DBUG_PRINT( &quot;FREE&quot;, (&quot;Freeing node %s at &quot; F_PTR, NODE_TEXT( arg_node), arg_node));'"/>
  <xsl:choose>
    <xsl:when test="sons/son[@name = &quot;Next&quot;]">
      <xsl:value-of select="'arg_node = ILIBfree( arg_node);'"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="'result = ILIBfree( arg_node);'"/>
    </xsl:otherwise>
  </xsl:choose>
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( result);'"/>
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
    <xsl:with-param name="prefix">FREE</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
    <xsl:with-param name="text">Frees the node and its sons/attributes</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">FREE</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
  </xsl:call-template>
</xsl:template>

<!--
     traversal main son

     calls the free function for a specific son

     example:

     ARG_NEXT( arg_node) = FREETRAV( ARG_NEXT( arg_node), arg_info);

     remarks:
 
     for sons named NEXT, the macro FREECONT is called, for all other
     sons FREETRAV is called. This allows to delete only one son in
     a chain of nodes.
-->
     
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
  <xsl:choose>
    <xsl:when test="@name = &quot;Next&quot;">
      <xsl:value-of select="' = FREECOND( '"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="' = FREETRAV( '"/>
    </xsl:otherwise>
  </xsl:choose>
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

<!--
     traversal main attribute

     calls the free function for a specific attribute

     example:

     ASSIGN_NEXT (arg_node) = FREETRAV (ASSIGN_NEXT (arg_node), arg_info);
     for (cnt = 0; cnt < 7; cnt++)
     {
       ASSIGN_MASK (arg_node, cnt) = FreeMask (ASSIGN_MASK (arg_node, cnt));
     }

     remark:

     Array attributes are iterated by the generated code using a for loop.
     Free functions for attributes are always called with one element!

-->
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
      <xsl:value-of select="'FREEattrib'"/>
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
      <xsl:value-of select="', arg_node);'"/>
      <!-- if it is an array, we have to complete the for loop -->
      <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="'}'"/>
      </xsl:if>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>