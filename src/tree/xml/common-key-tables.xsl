<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/08/07 16:20:36  sah
  Initial revision


 
 -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- as we make lookups to attributetypes quite often, we create a key -->
<xsl:key name="types" match="//attributetypes/type" use="@name"/>
<xsl:key name="arraytypes" match="//attributetypes/type[@size]" use="@name"/>

</xsl:stylesheet>
