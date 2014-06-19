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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="../xml/common_c_code.xsl"/>
<xsl:import href="../xml/common_name_to_nodeenum.xsl"/>
<xsl:import href="../xml/common_node_access.xsl"/>
<xsl:import href="../xml/common_make_assertion.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- templates for generating the body of a make function -->
<xsl:template match="node" mode="make-body">
  <xsl:value-of select="'{'"/>
  <!-- declarate variables -->
  <xsl:value-of select="'struct NODE_ALLOC_N_'" />
  <xsl:call-template name="uppercase" >
    <xsl:with-param name="string">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' *nodealloc;'" />
  <xsl:value-of select="'node *xthis;'" />
  <xsl:value-of select="'int size;'" />
  <!-- counter for for-loops -->
  <xsl:if test="attributes/attribute/type[key( &quot;arraytypes&quot;, @name)]" >
    <xsl:value-of select="'int cnt;'" />
  </xsl:if>
  <!-- DBUG_ENTER call -->
  <xsl:value-of select="'DBUG_ENTER ();'"/>
  <!-- allocate new node xthis -->
  <xsl:value-of select="'DBUG_PRINT (&quot;allocating node structure&quot;);'"/>
  <xsl:value-of select="'size = sizeof( struct NODE_ALLOC_N_'" />
  <xsl:call-template name="uppercase" >
    <xsl:with-param name="string">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="');'" />
  <!-- 
      Part for Memorycheck START 
   -->
  <xsl:call-template name="newline" />
  <xsl:value-of select="'nodealloc = (struct NODE_ALLOC_N_'" />
  <xsl:call-template name="uppercase" >
    <xsl:with-param name="string">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' *) MEMmallocAt( size, file, line);'" />
  <xsl:call-template name="newline" />
  <!-- 
      Part for Memorycheck END 
   -->
  <!-- set node structure -->
  <xsl:value-of select="'xthis = (node *) &amp;(nodealloc->nodestructure);'" />
  <xsl:call-template name="newline" />
  <xsl:text>
#ifndef DBUG_OFF
  </xsl:text>
  <xsl:value-of select="'CHKMisNode(xthis, N_'" />
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="');'" />
  <xsl:text>
#endif
  </xsl:text>
  <xsl:call-template name="newline" />
  <!-- set sons and attribs pointer -->
  <!-- only set sons if we have sons -->
  <xsl:if test="sons/son">
    <xsl:value-of select="'xthis->sons.'"/>
    <xsl:call-template name="name-to-nodeenum" >
      <xsl:with-param name="name" select="@name"/>
    </xsl:call-template>
    <xsl:value-of select="' = (struct SONS_N_'" />
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" select="@name"/>
    </xsl:call-template>
    <xsl:value-of select="' *) &amp;(nodealloc->sonstructure);'" />
  </xsl:if>
  <!-- only set attributes/flags if we have some -->
  <xsl:if test="attributes/attribute | flags/flag">
    <xsl:value-of select="'xthis->attribs.'"/>
    <xsl:call-template name="name-to-nodeenum" >
      <xsl:with-param name="name" select="@name"/>
    </xsl:call-template>
    <xsl:value-of select="' = (struct ATTRIBS_N_'" />
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" select="@name"/>
    </xsl:call-template>
    <xsl:value-of select="' *) &amp;(nodealloc->attributestructure);'" />
  </xsl:if>
  <xsl:value-of select="'NODE_TYPE( xthis) = N_'" />
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />

  <xsl:value-of select="'DBUG_PRINT (&quot;address: &quot;F_PTR, xthis);'"/>
  <!-- set node type -->
  <xsl:value-of select="'DBUG_PRINT (&quot;setting node type&quot;);'"/>
  <xsl:value-of select="'NODE_TYPE(xthis) = '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />
  <!-- set lineno -->
  <xsl:value-of select="'DBUG_PRINT (&quot;setting lineno to %d&quot;, global.linenum);'"/>
  <xsl:value-of select="'NODE_LINE( xthis) = global.linenum;'" />
  <xsl:value-of select="'DBUG_PRINT (&quot;setting colno to %d&quot;, global.colnum);'"/>
  <xsl:value-of select="'NODE_COL( xthis) = global.colnum;'" />
  <xsl:value-of select="'NODE_ERROR( xthis) = NULL;'" />
  <!-- set filename -->
  <xsl:value-of select="'DBUG_PRINT (&quot;setting filename to %s&quot;, global.filename);'"/>
  <xsl:value-of select="'NODE_FILE( xthis) = global.filename;'" />
  <!-- assign sons and attributes a value -->
  <xsl:apply-templates select="sons/son" mode="make-body"/>
  <xsl:apply-templates select="attributes/attribute" mode="make-body"/>
  <!-- init flags -->
  <xsl:apply-templates select="flags/flag" mode="make-body" />
  <!-- if DBUG enabled, check for valid arguments -->
  <xsl:call-template name="newline" />
  <xsl:value-of select="'#ifndef DBUG_OFF'" />
  <xsl:call-template name="newline" />
  <xsl:value-of select="'DBUG_PRINT (&quot;doing son target checks&quot;);'"/>
  <!-- generate warning messages -->
  <xsl:apply-templates select="sons/son[not( @default)]" mode="make-assertion-target">
    <xsl:with-param name="self"><xsl:value-of select="'xthis'"/></xsl:with-param>
  </xsl:apply-templates>
  <xsl:call-template name="newline" />
  <xsl:value-of select="'#endif /* DBUG_OFF */'" />
  <xsl:call-template name="newline" />
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN (xthis);'"/>
  <xsl:value-of select="'}'"/>
</xsl:template>

<!-- generate the assignment for a son -->
<xsl:template match="sons/son" mode="make-body">
  <xsl:value-of select="'DBUG_PRINT (&quot;assigning son '"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' initial value: &quot;F_PTR, '"/>
  <xsl:choose>
    <xsl:when test="@default">
      <xsl:value-of select="@default" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="@name" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:value-of select="');'"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'xthis'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = '"/>
  <!-- check for default value -->
  <xsl:choose>
    <xsl:when test="@default">
      <xsl:value-of select="@default" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="@name" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:value-of select="';'"/>
  <!-- if the current son is an avis, add the backref -->
  <xsl:if test="&quot;Avis&quot; = @name">
    <xsl:value-of select="'if ( '" />
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">
        <xsl:value-of select="'xthis'" />
      </xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name" />
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="'Avis'" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' != NULL) {'" />
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">
        <xsl:call-template name="node-access">
          <xsl:with-param name="node">
            <xsl:value-of select="'xthis'" />
          </xsl:with-param>
          <xsl:with-param name="nodetype">
            <xsl:value-of select="../../@name" />
          </xsl:with-param>
          <xsl:with-param name="field">
            <xsl:value-of select="'Avis'" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="'Avis'" />
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="'Decl'" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'= xthis; }'"/>
    <xsl:call-template name="newline" />
  </xsl:if>
</xsl:template>
 
<!-- generate the assignmnent for an attribute -->
<xsl:template match="attributes/attribute" mode="make-body">
  <!-- if it is an array, we have to build a for loop over its elements -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
    <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
    <xsl:value-of select="'; cnt++) { '" />
  </xsl:if>
  <!-- left side of assignment -->
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'xthis'" />
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
  <xsl:apply-templates select="@name" mode="make-body" />
  <xsl:value-of select="';'"/>
  <!-- finally, end the for loop if it was an array -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'}'" />
  </xsl:if>
</xsl:template> 

<!-- a default value implies that xthis attribute is not 
     passed as an argument to the make function, thus the r-value
     of the assignment is its default -->
<xsl:template match="@name[../@default]" mode="make-body">
  <xsl:value-of select="../@default"/>
</xsl:template>

<!-- the inconstructor attribute defines whether the attribute is a 
     constructor argument -->
<xsl:template match="@name[../@inconstructor = &quot;yes&quot;]" mode="make-body">
  <xsl:value-of select="."/>
  <!-- if its an array, we have to add the selector -->
  <xsl:if test="key(&quot;arraytypes&quot;, ../type/@name)">
    <xsl:value-of select="'[x]'" />
  </xsl:if>
</xsl:template>

<!-- no default and beeing a temporary attribute implies using the
     init value for xthis attributes type -->
<xsl:template match="@name" mode="make-body">
  <xsl:value-of select="key(&quot;types&quot;, ../type/@name)/@init"/>
</xsl:template>

<xsl:template match="flag[@default]" mode="make-body">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'xthis'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = '"/>
  <xsl:value-of select="@default" />
  <xsl:value-of select="' ;'" />
</xsl:template>

<xsl:template match="flag" mode="make-body">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'xthis'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = FALSE;'"/>
</xsl:template>

</xsl:stylesheet>
