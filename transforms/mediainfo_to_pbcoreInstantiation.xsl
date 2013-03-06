<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.pbcore.org/PBCore/PBCoreNamespace.html"
    exclude-result-prefixes="">
  <xsl:output method="xml" indent="yes" encoding="UTF-8"/>
  <!-- Will emit a pbcoreInstantiation element if not set to true. -->
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
  
  <xsl:template match="Color_space" mode="general">
    <instantiationColors>
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationColors>
  </xsl:template>
  
  <xsl:template match="File_last_modification_date[1]" mode="general">
    <instantiationDate dateType="created">
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
  
  <xsl:template match="Video_Language_List[1]" mode="general">
    <instantiationLanguage>
      <xsl:value-of select="normalize-space(text())"/>
    </instantiationLanguage>
  </xsl:template>
  
  <xsl:template match="Standard" mode="essence">
    <essenceTrackStandard>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackStandard>
  </xsl:template>
  
  <xsl:template match="Format" mode="essence">
    <essenceTrackEncoding>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackEncoding>
  </xsl:template>
  
  <xsl:template match="Sampling_rate[1]" mode="essence">
    <essenceTrackSamplingRate>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackSamplingRate>
  </xsl:template>
  
  <xsl:template match="Bit_rate[1]" mode="essence">
    <essenceTrackDataRate unitsOfMeasure="bits per second">
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackDataRate>
  </xsl:template>
  
  <xsl:template match="Bit_depth[1]" mode="essence">
    <essenceTrackBitDepth>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackBitDepth>
  </xsl:template>
  
  <xsl:template match="Frame_rate[1]" mode="essence">
    <essenceTrackFrameRate unitsOfMeasure="frames per second">
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackFrameRate>
  </xsl:template>
  
  <xsl:template match="Width[position()=1 and not(../Original_width)]" mode="essence">
    <essenceTrackFrameSize>
      <xsl:value-of select="normalize-space(concat(text(), 'x', ../Height[1]/text()))"/>
    </essenceTrackFrameSize>
  </xsl:template>
  
  <xsl:template match="Original_width[1]" mode="essence">
    <essenceTrackFrameSize>
      <xsl:value-of select="normalize-space(concat(text(), 'x', ../Original_height[1]/text()))"/>
    </essenceTrackFrameSize>
  </xsl:template>
  
  <xsl:template match="Display_aspect_ratio[last()]" mode="essence">
    <essenceTrackAspectRatio>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackAspectRatio>
  </xsl:template>
  
  <xsl:template match="Language[2]" mode="essence">
    <essenceTrackLanguage>
      <xsl:value-of select="normalize-space(text())"/>
    </essenceTrackLanguage>
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


