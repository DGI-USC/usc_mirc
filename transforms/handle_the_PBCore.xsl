<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.pbcore.org/PBCore/PBCoreNamespace.html"
    xmlns:pbcore="http://www.pbcore.org/PBCore/PBCoreNamespace.html"
    exclude-result-prefixes="pbcore"
    version="1.0">
    <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes" media-type="text/xml"/>
    
    <xsl:param name='handle_value'>this_is_not_a_handle</xsl:param>
    
    <xsl:template match="/">
        <xsl:apply-templates/>
    </xsl:template>
    
    <xsl:template match="pbcore:pbcoreInstantiationDocument">
        <xsl:copy>
            <xsl:apply-templates/>
            <instantiationIdentifier source="hdl">
                <xsl:value-of select="$handle_value"/>
            </instantiationIdentifier>
        </xsl:copy>
    </xsl:template>
    
    <!-- delete handle ID at the top... -->
    <xsl:template match="pbcore:pbcoreInstantiationDocument/pbcore:instantiationIdentifier[@source='hdl']"/>
    
    <xsl:template match="node()|@*">
        <xsl:if test="normalize-space(current())">
            <xsl:copy>
                <xsl:apply-templates select="node()|@*"/>
            </xsl:copy>
        </xsl:if>
    </xsl:template>
</xsl:stylesheet>