<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/08/07 16:20:14  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">
<xsl:import href="common-c-code.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- generate a free function head -->
<xsl:template name="travfun-head">
  <xsl:param name="prefix" />
  <xsl:param name="name" />
  <xsl:value-of select="'node *'"/>
  <xsl:value-of select="$prefix"/>
  <xsl:value-of select="$name"/>
  <xsl:value-of select="'( node *arg_node, info *arg_info )'"/>
</xsl:template>

<xsl:template name="travfun-comment">
  <xsl:param name="prefix" />
  <xsl:param name="name" />
  <xsl:param name="text" />
  <xsl:text>
/** &lt;!--******************************************************************--&gt;
 *
 * @fn </xsl:text><xsl:value-of select="$prefix"/><xsl:value-of select="$name"/><xsl:text>
 *
 * @brief </xsl:text><xsl:value-of select="$text"/><xsl:text>
 *
 * @param arg_node </xsl:text><xsl:value-of select="$name"/><xsl:text> node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
  </xsl:text>
</xsl:template>

</xsl:stylesheet>
