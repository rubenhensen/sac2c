<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/11/26 11:00:39  sah
  added default traversal function

  Revision 1.1  2004/11/23 22:18:52  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="common-key-tables.xsl"/>
  <xsl:import href="common-travfun.xsl"/>
  <xsl:import href="common-name-to-nodeenum.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'traverse_tables.c'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the function tables for traversal.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id$'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#include "traverse_tables.h"
#include "traverse_helper.h"
    </xsl:text>
      <xsl:apply-templates select="/definition/phases//traversal" mode="include" />
    <xsl:text>

travtables_t travtables = {
    </xsl:text>
    <xsl:apply-templates select="/definition/phases//traversal" />
    <xsl:text>
};

    </xsl:text>
  </xsl:template>

  <xsl:template match="traversal">
    <xsl:variable name="phase">
      <xsl:value-of select="@id" />
    </xsl:variable>
    <xsl:if test="position() > 1">
      <xsl:value-of select="', '" />
    </xsl:if>
    <xsl:value-of select="' { &amp;TRAVerror'" />
    <xsl:for-each select="/definition/syntaxtree/node" >
      <xsl:value-of select="', &amp;'" />
      <xsl:choose>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/travuser/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">user</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/travsons/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">sons</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/travnone/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">none</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/traverror/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="current()/@id" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">error</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">
              <xsl:value-of select="key( &quot;traversals&quot;, $phase)/@default" />
            </xsl:with-param>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
    <xsl:value-of select="' }'" />
  </xsl:template>

  <xsl:template name="travtab-to-travname">
    <xsl:param name="phase" />
    <xsl:param name="node" />
    <xsl:param name="style" />
   
    <xsl:choose>
      <xsl:when test="$style = &quot;user&quot;">
        <xsl:call-template name="travfun-name">
          <xsl:with-param name="prefix">
            <xsl:value-of select="$phase" />
          </xsl:with-param>
          <xsl:with-param name="name">
            <xsl:value-of select="$node" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="$style = &quot;sons&quot;">
        <xsl:value-of select="'TRAVsons'" />
      </xsl:when>
      <xsl:when test="$style = &quot;none&quot;">
        <xsl:value-of select="'TRAVnone'" />
      </xsl:when>
      <xsl:when test="$style = &quot;error&quot;">
        <xsl:value-of select="'TRAVerror'" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$style" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="traversal" mode="include">
    <xsl:value-of select="'#include &quot;'" />
    <xsl:value-of select="@include" />
    <xsl:value-of select="'&quot;'" />
    <xsl:call-template name="newline" />
  </xsl:template>

</xsl:stylesheet>
