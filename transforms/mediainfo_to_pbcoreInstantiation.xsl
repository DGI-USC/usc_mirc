<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.pbcore.org/PBCore/PBCoreNamespace.html"
    exclude-result-prefixes="">
  <xsl:output method="xml" indent="yes" encoding="UTF-8"/>
  <xsl:param name="instanceDoc">TRUE</xsl:param>
  
  <xsl:template match="File[1]">
    <xsl:choose>
      <xsl:when test="$instanceDoc='TRUE'">
        <pbcoreInstantiationDocument>
          <xsl:apply-templates/>
        </pbcoreInstantiationDocument>
      </xsl:when>
      <xsl:otherwise>
        <pbcoreInstantiation>
          <xsl:apply-templates/>
        </pbcoreInstantiation>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="track">
    <xsl:choose>
      <xsl:when test="@type='General'">
        <xsl:apply-templates mode="general"/>
      </xsl:when>
      <xsl:otherwise>
        <instantiationEssenceTrack>
          <essenceTrackType>
            <xsl:value-of select="@type"/>
          </essenceTrackType>
          <xsl:apply-templates mode="essence"/>
        </instantiationEssenceTrack>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="Complete_name[1]" mode="general">
    <instantiationIdentifier source="filename">
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationIdentifier>
  </xsl:template>
  
  <xsl:template match="File_last_modification_date[1]" mode="general">
    <instantiationDate dateType="modified">
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationDate>
  </xsl:template>
  
  <xsl:template match="Internet_media_type[1]" mode="general">
    <instantiationDigital>
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationDigital>
  </xsl:template>
  
  <xsl:template match="Format[1]" mode="general">
    <instantiationStandard source="container">
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationStandard>
  </xsl:template>
  
  <xsl:template match="File_size[1]" mode="general">
    <instantiationFileSize unitsOfMeasure="bytes">
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationFileSize>
  </xsl:template>
  
  <xsl:template match="Duration[last()]" mode="general">
    <instantiationDuration>
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationDuration>
  </xsl:template>
  
  <xsl:template match="Overall_bit_rate[1]" mode="general">
    <instantiationDataRate>
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationDataRate>
  </xsl:template>
  
  <xsl:template match="Codec_CC" mode="essence">
    <essenceTrackStandard>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackStandard>
  </xsl:template>
  
  <xsl:template match="Codec_Info" mode="essence">
    <essenceTrackEncoding>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackEncoding>
  </xsl:template>
  
  <xsl:template match="Bit_rate[1]" mode="essence">
    <essenceTrackDataRate unitsOfMeasure="bits per second">
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackDataRate>
  </xsl:template>
  
  <xsl:template match="Frame_rate[1]" mode="essence">
    <essenceTrackFrameRate unitsOfMeasure="frames per second">
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackFrameRate>
  </xsl:template>
  
  <xsl:template match="Display_aspect_ratio[1]" mode="essence">
    <essenceTrackAspectRatio>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackAspectRatio>
  </xsl:template>
  
  <xsl:template match="Duration[last()]" mode="essence">
    <essenceTrackDuration>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackDuration>
  </xsl:template>
  
  <xsl:template match="Bit_depth[1]" mode="essence">
    <essenceTrackBitDepth>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackBitDepth>
  </xsl:template>
  
  <xsl:template match="Stream_size[1]" mode="essence">
    <essenceTrackAnnotation annotationType="stream_size_in_bytes">
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackAnnotation>
  </xsl:template>
  
  <xsl:template match="text()" mode="essence"/>
  <xsl:template match="text()" mode="general"/>
  
  <xsl:template match="*">
    <xsl:apply-templates/>
  </xsl:template>
</xsl:stylesheet>

