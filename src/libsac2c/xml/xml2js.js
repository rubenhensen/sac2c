function loadXMLDoc(filename)
{
  xhttp = new XMLHttpRequest();
  xhttp.open("GET", filename, false);
  xhttp.send("");
  return xhttp.responseXML;
}

function XMLconvertInsertAt( xmlFile, xslFile, id)
{
  xml = loadXMLDoc(xmlFile);
  xsl = loadXMLDoc(xslFile);

  if (document.implementation && document.implementation.createDocument)
  {
    xsltProcessor = new XSLTProcessor();
    xsltProcessor.importStylesheet(xsl);
    script = document.createElement("script");
    resultDocument = xsltProcessor.transformToFragment(xml, document);
    script.appendChild(resultDocument);

    position = document.getElementById(id);
    document.body.insertBefore(script,position);

  }
}
