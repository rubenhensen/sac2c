<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="text" version="1.0"
encoding="utf-8" indent="yes"/>

<xsl:template match="/">
    <!-- nodesets -->
    <xsl:for-each select="definition/nodesets/nodeset">
        <xsl:variable name="name">
            <xsl:value-of select="@name"/>
        </xsl:variable>
  var N_<xsl:copy-of select="$name" /> = { name: "<xsl:copy-of select="$name" />", set: true, unfolded : false, children: [ ], labels: [] };
    </xsl:for-each>
    
    <!-- nodes -->
    <xsl:for-each select="definition/syntaxtree/node">
        <xsl:variable name="name">
            <xsl:value-of select="@name"/>
        </xsl:variable>
        <xsl:choose>
          <xsl:when test="count(sons/*) = 0">
  var N_<xsl:copy-of select="$name" /> = { name: "<xsl:copy-of select="$name" />", set: false, unfolded : true, children: [], labels: null };
          </xsl:when>
          <xsl:otherwise>
  var N_<xsl:copy-of select="$name" /> = { name: "<xsl:copy-of select="$name" />", set: false, unfolded : false, children: [ ], labels: [
        <xsl:for-each select="sons/son">
            "<xsl:value-of select="@name"/>",
        </xsl:for-each>
        ""] };
          </xsl:otherwise>
        </xsl:choose>
    </xsl:for-each>


   <!-- nodeset son lists -->
   <xsl:for-each select="definition/nodesets/nodeset">
        <xsl:variable name="name">
            <xsl:value-of select="@name"/>
        </xsl:variable>
  var <xsl:copy-of select="$name" />_sons = [
        <xsl:for-each select="target/node">
            N_<xsl:value-of select="@name"/>,
        </xsl:for-each>
     null ];
    </xsl:for-each>

    <!-- node son lists -->

    <xsl:for-each select="definition/syntaxtree/node">
        <xsl:variable name="name">
            <xsl:value-of select="@name"/>
        </xsl:variable>
  var <xsl:copy-of select="$name" />_sons = [
        <xsl:for-each select="sons/son/targets">
          <xsl:choose>
            <xsl:when test="count(*) = 1">
               N_<xsl:value-of select="target/*/@name"/>,
            </xsl:when>
            <xsl:otherwise>
<!-- Here we need to create ad-hoc sets! -->
               N_<xsl:value-of select="target/*/@name"/>,
            </xsl:otherwise>
          </xsl:choose>
        </xsl:for-each>
     null ];
    </xsl:for-each>

</xsl:template>

</xsl:stylesheet>
