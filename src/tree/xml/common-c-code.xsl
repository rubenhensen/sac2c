<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/07/11 18:21:28  sah
  Initial revision

 
 -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- basic functions for string processing needed if generating c code -->

<!-- function translating a string to uppercase -->
<xsl:template name="uppercase">
  <xsl:param name="string"/>
  <xsl:value-of select="translate($string, 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
</xsl:template>

<!-- function generating a newline -->
<xsl:template name="newline">
  <xsl:text>
  </xsl:text>
</xsl:template>

</xsl:stylesheet>
