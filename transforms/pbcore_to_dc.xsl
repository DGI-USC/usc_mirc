<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" 
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/dc/" 
    xmlns:dcterms="http://purl.org/dc/terms/"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:pb="http://www.pbcore.org/PBCore/PBCoreNamespace.html"
    exclude-result-prefixes="pb">
    <xsl:output method="xml" indent="yes" />
    <xsl:template match="text()" />
    <xsl:template match="/">
        <oai_dc:dc xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
            <xsl:apply-templates />
        </oai_dc:dc>
    </xsl:template>
    
    <xsl:template match="/pb:pbcoreDescriptionDocument">
      <dc:type>Collection</dc:type>
      <dc:type>Text</dc:type>
      <xsl:apply-templates/>
    </xsl:template>
    
    <xsl:template match="/pb:pbcoreInstantiationDocument | pb:pbcoreInstantiation">
      <dc:type>MovingImage</dc:type>
      <xsl:apply-templates/>
    </xsl:template>
    
    <xsl:template match="pb:pbcoreIdentifier[normalize-space(text())] | pb:instantiationIdentifier[not(@source='instantiation_title') and normalize-space(text())]">
        <dc:identifier><xsl:value-of select="normalize-space(text())"/></dc:identifier>
    </xsl:template>
    
    <xsl:template match="pb:pbcoreTitle[@titleType='Main' and normalize-space(text())] | pb:instantiationAnnotation[@annotationType='instantiation_title' and normalize-space(text())]">
        <dc:title>
            <xsl:value-of select="normalize-space(text())"/>
        </dc:title>
    </xsl:template>
    
    <xsl:template match="pb:pbcoreTitle[not(@titleType='Main') and normalize-space(text())]">
      <dc:alternative><xsl:value-of select="normalize-space(text())"/></dc:alternative>
    </xsl:template>
    
    <xsl:template match="pb:pbcoreSubject[normalize-space(text())]">
      <dc:subject><xsl:value-of select="normalize-space(text())"/></dc:subject>
    </xsl:template>
    
    <xsl:template match="pb:pbcoreDescription[@descriptionType='Abstract' and normalize-space(text())] | pb:instantiationAnnotation[@annotationType='abstract' and normalize-space(text())]">
      <dc:description><xsl:value-of select="normalize-space(text())"/></dc:description>
    </xsl:template>
    
    <xsl:template match="pb:pbcoreGenre[normalize-space(text())]">
      <dc:subject><xsl:value-of select="normalize-space(text())"/></dc:subject>
    </xsl:template>
    <xsl:template match="pb:pbcoreCoverage/pb:coverage[normalize-space(text())]">
      <dc:coverage><xsl:value-of select="normalize-space(text())"/></dc:coverage>
    </xsl:template>

    <xsl:template match="pb:pbcoreCreator">
      <xsl:variable name="person" select="normalize-space(pb:creator)"/>
      <xsl:variable name="role">
        <xsl:choose>
          <xsl:when test="normalize-space(pb:creatorRole)"><xsl:value-of select="normalize-space(pb:creatorRole)"/></xsl:when>
          <xsl:otherwise>No role provided</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      
      <xsl:if test="$person">
        <dc:creator><xsl:value-of select="$person"/> (<xsl:value-of select="$role"/>)</dc:creator>
      </xsl:if>
    </xsl:template>    
    <xsl:template match="pb:pbcoreContributor">
      <xsl:variable name="person" select="normalize-space(pb:contributor)"/>
      <xsl:variable name="role">
        <xsl:choose>
          <xsl:when test="normalize-space(pb:contributorRole)"><xsl:value-of select="normalize-space(pb:contributorRole)"/></xsl:when>
          <xsl:otherwise>No role provided</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      
      <xsl:if test="$person">
        <dc:contributor><xsl:value-of select="$person"/> (<xsl:value-of select="$role"/>)</dc:contributor>
      </xsl:if>
    </xsl:template>

    <xsl:template match="pb:pbcoreRightsSummary[normalize-space(text())] | pb:instantiationRights[normalize-space(text())]">
        <dc:rights><xsl:value-of select="normalize-space(pb:rightsSummary/text())"/></dc:rights>
    </xsl:template>
    
    <xsl:template match="pb:pbcoreAssetDate[normalize-space(text())] | pb:instantiationDate[normalize-space(text())]">
        <dc:date><xsl:value-of select="normalize-space(text())"/></dc:date>
    </xsl:template>
    <xsl:template match="pb:instantiationGenerations[normalize-space(text())]">
        <dc:type><xsl:value-of select="normalize-space(text())" /></dc:type>
    </xsl:template>
    <xsl:template match="pb:instantiationMediaType[normalize-space(text())]">
        <dc:type><xsl:value-of select="normalize-space(text())" /></dc:type>
    </xsl:template>
    
    <xsl:template match="pb:instantiationLanguage[normalize-space(text())]">
        <dc:language><xsl:value-of select="normalize-space(text())" /></dc:language>
    </xsl:template>
        
    <xsl:template match="*">
      <xsl:apply-templates/>
    </xsl:template>
</xsl:stylesheet>


