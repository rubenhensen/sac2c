<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/11/22 17:16:56  sah
  changes.
  DK 04

  Revision 1.1  2004/08/06 21:17:48  sah
  Initial revision


 
 -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- here, the renaming scheme from a node name (as used in name attribute)
     to the nodetype enum name for that node (N_xxx) is defined. All other
     traversals use this template to perform that renaming -->

<xsl:template name="name-to-nodeenum">
  <xsl:param name="name"/>
  <!-- they all start with a leading N_ -->
  <xsl:value-of select="'N_'" />
  <xsl:call-template name="lowercase">
    <xsl:with-param name="string">
      <xsl:value-of select="$name" />
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>
