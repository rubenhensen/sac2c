<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/11/24 00:25:04  sah
  added another key
  ,

  Revision 1.1  2004/11/23 11:34:56  sah
  Initial revision


 
 -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- as we make lookups to attributetypes quite often, we create a key -->
<xsl:key name="types" match="//attributetypes/type" use="@name"/>
<xsl:key name="arraytypes" match="//attributetypes/type[@size]" use="@name"/>

<!-- the same for traversal defaults -->
<xsl:key name="traversals" match="//phases//traversal" use="@id" />

</xsl:stylesheet>
