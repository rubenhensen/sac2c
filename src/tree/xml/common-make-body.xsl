<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.10  2004/11/14 15:18:21  sah
  added support for default values for flags

  Revision 1.9  2004/11/03 17:19:24  sah
  the sons are always initialised to NULL now, as
  TravSons may try to traverse into sons that in
  fact are not used and thus do not get an initial
  value assigned!

  Revision 1.8  2004/10/26 09:36:43  sah
  flags are initialised now

  Revision 1.7  2004/08/29 18:10:05  sah
  general improvements

  Revision 1.6  2004/08/07 17:57:57  sah
  some changes to increase reusability

  Revision 1.5  2004/08/07 16:19:05  sah
  most xsl files use key-tables for type lookups
  now which increases speed significantly.
  lots of small improvements

  Revision 1.4  2004/08/06 21:19:57  sah
  uses common-node-access.xsl common-name-to-nodeenum.xsl now
  and generates assertions using common-make-assertion.xsl

  Revision 1.3  2004/08/06 14:39:48  sah
  some ast improvements

  Revision 1.2  2004/07/31 16:16:57  sah
  added support for flags and moved to memory saving attribute
  structure.

  Revision 1.1  2004/07/11 18:22:03  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-c-code.xsl"/>
<xsl:import href="common-name-to-nodeenum.xsl"/>
<xsl:import href="common-node-access.xsl"/>
<xsl:import href="common-make-assertion.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- templates for generating the body of a make function -->
<xsl:template match="node" mode="make-body">
  <xsl:value-of select="'{'"/>
  <!-- declarate variables -->
  <xsl:value-of select="'node *this;'" />
  <!-- counter for for-loops -->
  <xsl:value-of select="'int cnt;'" />
  <!-- DBUG_ENTER call -->
  <xsl:value-of select="'DBUG_ENTER( &quot;Make'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- allocate new node this -->
  <xsl:value-of select="'DBUG_PRINT( &quot;MAKE&quot;, (&quot;allocating node structure&quot;));'"/>
  <xsl:value-of select="'this = MakeEmptyNode();'" />
  <!-- clear all node pointers -->
  <xsl:value-of select="'for(cnt=0;cnt&lt;MAX_SONS;cnt++){this-&gt;node[cnt]=NULL;}'"/>
  <!-- allocate the attrib structure -->
  <xsl:value-of select="'DBUG_PRINT( &quot;MAKE&quot;, (&quot;allocating attrib structure&quot;));'"/>
  <xsl:value-of select="'this->attribs.N_'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' = Malloc(sizeof(struct AttribS_N_'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'));'"/>
  <!-- set node type -->
  <xsl:value-of select="'DBUG_PRINT( &quot;MAKE&quot;, (&quot;setting node type&quot;));'"/>
  <xsl:value-of select="'NODE_TYPE(this) = '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />
  <!-- set lineno -->
  <xsl:value-of select="'DBUG_PRINT( &quot;MAKE&quot;, (&quot;setting lineno&quot;));'"/>
  <xsl:value-of select="'this->lineno = linenum;'" />
  <!-- set filename -->
  <xsl:value-of select="'DBUG_PRINT( &quot;MAKE&quot;, (&quot;setting filename&quot;));'"/>
  <xsl:value-of select="'this->src_file = filename;'" />
  <!-- assign sons and attributes a value -->
  <xsl:apply-templates select="sons/son" mode="make-body"/>
  <xsl:apply-templates select="attributes/attribute" mode="make-body"/>
  <!-- init flags -->
  <xsl:apply-templates select="flags[flag]" mode="make-body" />
  <!-- if DBUG enabled, check for valid arguments -->
  <xsl:call-template name="newline" />
  <xsl:value-of select="'#ifndef DBUG_OFF'" />
  <xsl:call-template name="newline" />
  <xsl:value-of select="'DBUG_PRINT( &quot;MAKE&quot;, (&quot;doing son target checks&quot;));'"/>
  <!-- generate warning messages -->
  <xsl:apply-templates select="sons/son[not( @default)]" mode="make-assertion-target">
    <xsl:with-param name="self"><xsl:value-of select="'this'"/></xsl:with-param>
  </xsl:apply-templates>
  <xsl:call-template name="newline" />
  <xsl:value-of select="'#endif /* DBUG_OFF */'" />
  <xsl:call-template name="newline" />
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( this);'"/>
  <xsl:value-of select="'}'"/>
</xsl:template>

<!-- generate the assignment for a son -->
<xsl:template match="sons/son" mode="make-body">
  <xsl:value-of select="'DBUG_PRINT( &quot;MAKE&quot;, (&quot;assigning son '"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' initial value&quot;));'"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'this'" />
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
      <xsl:value-of select="'this'" />
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

<!-- a default value implies that this attribute is not 
     passed as an argument to the make function, thus the r-value
     of the assignment is its default -->
<xsl:template match="@name[../@default]" mode="make-body">
  <xsl:value-of select="../@default"/>
</xsl:template>

<!-- no default and beeing a permanent attribute implies that this 
     attribute is passed as an argument thus the r-value is the 
     argument -->
<xsl:template match="@name[not(../@default)][../phases/all]" mode="make-body">
  <xsl:value-of select="."/>
  <!-- if its an array, we have to add the selector -->
  <xsl:if test="key(&quot;arraytypes&quot;, ../type/@name)">
    <xsl:value-of select="'[x]'" />
  </xsl:if>
</xsl:template>

<!-- no default and beeing a temporary attribute implies using the
     init value for this attributes type -->
<xsl:template match="@name[not(../@default)][not(../phases/all)]" mode="make-body">
  <xsl:value-of select="key(&quot;types&quot;, ../type/@name)/@init"/>
</xsl:template>

<xsl:template match="flags" mode="make-body">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'this'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="'Flags'" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = 0;'" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'this'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="'DBug_Flags'" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = 0;'" />
  <!-- init values -->
  <xsl:apply-templates select="flag[@default]" mode="make-body" />
</xsl:template>

<xsl:template match="flag" mode="make-body">
  <xsl:value-of select="'SET_FLAG( '" />
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', this, IS_'" />
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', '" />
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string">
      <xsl:value-of select="@default" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="');'" />
</xsl:template>

</xsl:stylesheet>
