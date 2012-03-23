<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:pb="http://www.pbcore.org/PBCore/PBCoreNamespace.html"
    exclude-result-prefixes="">
  <xsl:output method="xml" indent="yes" encoding="UTF-8"/>
  <xsl:param name="instanceDoc">TRUE</xsl:param>
  
  <xsl:template match="File[1]">
    <xsl:choose>
      <xsl:when test="$instanceDoc='TRUE'">
        <pb:pbcoreInstantiationDocument>
          <xsl:apply-templates/>
        </pb:pbcoreInstantiationDocument>
      </xsl:when>
      <xsl:otherwise>
        <pb:pbcoreInstantiation>
          <xsl:apply-templates/>
        </pb:pbcoreInstantiation>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="track">
    <xsl:choose>
      <xsl:when test="@type='General'">
        <xsl:apply-templates mode="general"/>
      </xsl:when>
      <xsl:otherwise>
        <pb:instantiationEssenceTrack>
          <pb:essenceTrackType>
            <xsl:value-of select="@type"/>
          </pb:essenceTrackType>
          <xsl:apply-templates mode="essence"/>
        </pb:instantiationEssenceTrack>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="Complete_name[1]" mode="general">
    <pb:instantiationIdentifier source="filename">
      <xsl:value-of select="normalize-space(text())"/>
    </pb:instantiationIdentifier>
  </xsl:template>
  
  <xsl:template match="File_last_modification_date[1]" mode="general">
    <pb:instantiationDate dateType="modified">
      <xsl:value-of select="normalize-space(text())"/>
    </pb:instantiationDate>
  </xsl:template>
  
  <xsl:template match="Internet_media_type[1]" mode="general">
    <pb:instantiationDigital>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:instantiationDigital>
  </xsl:template>
  
  <xsl:template match="Format[1]" mode="general">
    <pb:instantiationStandard source="container">
      <xsl:value-of select="normalize-space(text())"/>
    </pb:instantiationStandard>
  </xsl:template>
  
  <xsl:template match="File_size[1]" mode="general">
    <pb:instantiationFileSize unitsOfMeasure="bytes">
      <xsl:value-of select="normalize-space(text())"/>
    </pb:instantiationFileSize>
  </xsl:template>
  
  <xsl:template match="Duration[last()]" mode="general">
    <pb:instantiationDuration>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:instantiationDuration>
  </xsl:template>
  
  <xsl:template match="Overall_bit_rate[1]" mode="general">
    <pb:instantiationDataRate>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:instantiationDataRate>
  </xsl:template>
  
  <xsl:template match="Codec_CC" mode="essence">
    <pb:essenceTrackStandard>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackStandard>
  </xsl:template>
  
  <xsl:template match="Codec_Info" mode="essence">
    <pb:essenceTrackEncoding>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackEncoding>
  </xsl:template>
  
  <xsl:template match="Bit_rate[1]" mode="essence">
    <pb:essenceTrackDataRate unitsOfMeasure="bits per second">
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackDataRate>
  </xsl:template>
  
  <xsl:template match="Frame_rate[1]" mode="essence">
    <pb:essenceTrackFrameRate unitsOfMeasure="frames per second">
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackFrameRate>
  </xsl:template>
  
  <xsl:template match="Display_aspect_ratio[1]" mode="essence">
    <pb:essenceTrackAspectRatio>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackAspectRatio>
  </xsl:template>
  
  <xsl:template match="Duration[last()]" mode="essence">
    <pb:essenceTrackDuration>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackDuration>
  </xsl:template>
  
  <xsl:template match="Bit_depth[1]" mode="essence">
    <pb:essenceTrackBitDepth>
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackBitDepth>
  </xsl:template>
  
  <xsl:template match="Stream_size[1]" mode="essence">
    <pb:essenceTrackAnnotation annotationType="stream_size_in_bytes">
      <xsl:value-of select="normalize-space(text())"/>
    </pb:essenceTrackAnnotation>
  </xsl:template>
  
  <xsl:template match="text()" mode="essence"/>
  <xsl:template match="text()" mode="general"/>
  
  <xsl:template match="*">
    <xsl:apply-templates/>
  </xsl:template>
</xsl:stylesheet>

