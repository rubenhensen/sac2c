<?xml version="1.0"?>
<!--
  $Log$
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

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- templates for generating the body of a make function -->
<xsl:template match="node" mode="make-body">
  <xsl:value-of select="'{'"/>
  <!-- if there is a for loop for initialising attributes, we 
       need a variable cnt, which is created here -->
  <xsl:if test="attributes/attribute/type[@name = //attributetypes/type[@size]/@name]">
    <xsl:value-of select="'int cnt;'" />
  </xsl:if>
  <!-- DBUG_ENTER call -->
  <xsl:value-of select="'DBUG_ENTER( &quot;Make'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- allocate new node this -->
  <xsl:value-of select="'node *this = Malloc(sizeof(node));'"/>
  <!-- allocate the attrib structure -->
  <xsl:value-of select="'this->attribs.N_'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' = Malloc(sizeof(struct AttribS_N_'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'));'"/>
  <!-- set node type -->
  <xsl:value-of select="'NODE_TYPE(this) = N_'" />
  <!-- UGLY HACK: MT and WL nodes use their uppercase name -->
  <xsl:choose>
    <xsl:when test="@name = 'MT'">
      <xsl:value-of select="'mt'" />
    </xsl:when>
    <xsl:when test="starts-with(@name,'MT') or starts-with(@name,'WL')">
      <xsl:value-of select="@name" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="lowercase">
        <xsl:with-param name="string">
          <xsl:value-of select="@name" />
        </xsl:with-param>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:value-of select="';'" />
  <!-- set lineno -->
  <xsl:value-of select="'this->lineno = linenum;'" />
  <!-- set filename -->
  <xsl:value-of select="'this->src_file = filename;'" />
  <!-- assign sons and attributes a value -->
  <xsl:apply-templates select="sons/son" mode="make-body"/>
  <xsl:apply-templates select="attributes/attribute" mode="make-body"/>
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( this);'"/>
  <xsl:value-of select="'}'"/>
</xsl:template>

<!-- generate the assignment for a son -->
<xsl:template match="sons/son" mode="make-body">
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_'"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'(this) = '"/>
  <!-- check for default value -->
  <xsl:if test="@default">
    <xsl:value-of select="@default" />
  </xsl:if>
  <xsl:if test="not(@default)">
    <xsl:value-of select="@name" />
  </xsl:if>
  <xsl:value-of select="';'"/>
</xsl:template>
 
<!-- generate the assignmnent for an attribute -->
<xsl:template match="attributes/attribute" mode="make-body">
  <!-- if it is an array, we have to build a for loop over its elements -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name][@size]">
    <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
    <xsl:value-of select="//attributetypes/type[@name = current()/type/@name]/@size" />
    <xsl:value-of select="'; cnt++) { '" />
  </xsl:if>
  <!-- left side of assignment -->
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_'"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'( this'" />
  <!-- if its is an array, we have to add another parameter -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
    <xsl:value-of select="', cnt'"/>
  </xsl:if>
  <xsl:value-of select="') = '" />
  <!-- right side of assignment -->
  <xsl:apply-templates select="@name" mode="make-body" />
  <xsl:value-of select="';'"/>
  <!-- finally, end the for loop if it was an array -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
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
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
    <xsl:value-of select="'[x]'" />
  </xsl:if>
</xsl:template>

<!-- no default and beeing a temporary attribute implies using the
     init value for this attributes type -->
<xsl:template match="@name[not(../@default)][not(../phases/all)]" mode="make-body">
  <xsl:value-of select="//attributetypes/type[@name = current()/../type/@name]/@init"/>
</xsl:template>

</xsl:stylesheet>
