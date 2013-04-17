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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a reduced xml file that only holds 
     information that is cruical and critical for the layout of
     modules -->

<xsl:template match="/">
  <xsl:apply-templates select="definition/syntaxtree" />
</xsl:template>

<!-- filter away descriptions -->
<xsl:template match="description" />

<!-- filter away range information -->
<xsl:template match="targets" />

<!-- filter away comments -->
<xsl:template match="comment()" />

<!-- generic template matching all elements and attributes -->
<xsl:template match="node() | @*">
  <xsl:copy>
    <xsl:apply-templates select="node() | @*"/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>
