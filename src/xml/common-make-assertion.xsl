<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/11/24 19:37:53  sah
  COMPILES

  Revision 1.1  2004/11/23 11:35:19  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-c-code.xsl"/>
<xsl:import href="common-name-to-nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<xsl:template match="node" mode="make-assertion">
  <xsl:apply-templates select="sons/son" mode="make-assertion" />
</xsl:template>

<!-- templates for generating assertions for a correct node type -->
<xsl:template match="son" mode="make-assertion">
  <xsl:param name="self"/>
  <xsl:apply-templates select="." mode="make-assertion-nonnull">
    <xsl:with-param name="self"><xsl:value-of select="$self"/></xsl:with-param>
  </xsl:apply-templates>
  <xsl:apply-templates select="." mode="make-assertion-target">
    <xsl:with-param name="self"><xsl:value-of select="$self"/></xsl:with-param>
  </xsl:apply-templates>
</xsl:template>

<!-- check for mandatory sons to be set to a value other than NULL -->
<xsl:template match="son[@mandatory = 'yes']" mode="make-assertion-nonnull">
  <xsl:param name="self"/>
  <xsl:value-of select="'DBUG_ASSERT( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="$self" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' != NULL, ( &quot;Field '" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' of node N_'" />
  <xsl:value-of select="../../@name" />
  <xsl:value-of select="' is mandatory but has value NULL&quot;));'" />
</xsl:template>

<!-- ignore all other sons -->
<xsl:template match="son" mode="make-assertion-nonnull">
  <xsl:param name="self"/>
</xsl:template>

<!-- check whether the value is of the right target -->
<xsl:template match="son" mode="make-assertion-target">
  <xsl:param name="self"/>
  <xsl:call-template name="newline" />
  <xsl:value-of select="'if ( ( '"/>
  <!-- check non-zero -->
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="$self" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' != NULL) '"/>
  <!-- check for target types -->
  <xsl:apply-templates select="target" mode="make-assertion-target">
    <xsl:with-param name="self" select="$self" />
  </xsl:apply-templates>
  <!-- a reasonable errormessage -->
  <xsl:value-of select="') { SYSWARN(( &quot;Field '" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' of node N_'" />
  <xsl:value-of select="../../@name" />
  <xsl:value-of select="' has non-allowed target node: %s&quot;, '" />
  <xsl:value-of select="'NODE_TEXT( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="$self" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'))); }'" />
</xsl:template>

<!-- for each target node we generate one N_xxx for the conditional -->
<xsl:template match="target/node" mode="make-assertion-target">
  <xsl:param name="self"/>
  <xsl:value-of select="'&amp;&amp; ( NODE_TYPE( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="$self" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field" >
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="') != '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="')'" />
</xsl:template>

<xsl:template match="target/set" mode="make-assertion-target">
  <xsl:param name="self"/>
  <!-- save information about current node for use in for-each loop -->
  <xsl:variable name="nodetype">
    <xsl:value-of select="../../../../@name" />
  </xsl:variable>
  <xsl:variable name="field">
    <xsl:value-of select="../../@name" />
  </xsl:variable>
  <!-- iterate over all nodeset members -->
  <xsl:for-each select="//nodesets/nodeset[@name = current()/@name]/target/node">
    <xsl:value-of select="'&amp;&amp; ( NODE_TYPE( '" />
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">
        <xsl:value-of select="$self" />
      </xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="$nodetype" />
      </xsl:with-param>
      <xsl:with-param name="field" >
        <xsl:value-of select="$field" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="') != '" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="')'" />
  </xsl:for-each>
</xsl:template>

<xsl:template match="target/unknown" mode="make-assertion-target">
  <xsl:value-of select="'&amp;&amp; FALSE'" />
</xsl:template>

</xsl:stylesheet>
