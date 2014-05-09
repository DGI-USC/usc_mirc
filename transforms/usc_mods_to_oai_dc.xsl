<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                xmlns:mods="http://www.loc.gov/mods/v3" exclude-result-prefixes="mods"
                xmlns:dc="http://purl.org/dc/elements/1.1/"
                xmlns:srw_dc="info:srw/schema/1/dc-schema"
                xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <xsl:output method="xml" indent="yes" omit-xml-declaration="yes"/>
    <xsl:strip-space elements="*"/>
    <xsl:template match="//mods:mods">
        <oai_dc:dc xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
            <xsl:apply-templates/>
        </oai_dc:dc>
    </xsl:template>

    <xsl:template match="mods:titleInfo">
        <dc:title>
            <xsl:value-of select="mods:nonSort"/>
            <xsl:if test="mods:nonSort">
                <xsl:text> </xsl:text>
            </xsl:if>
            <xsl:value-of select="mods:title"/>
            <xsl:if test="mods:subTitle">
                <xsl:text>: </xsl:text>
                <xsl:value-of select="mods:subTitle"/>
            </xsl:if>
            <xsl:if test="mods:partNumber">
                <xsl:text>. </xsl:text>
                <xsl:value-of select="mods:partNumber"/>
            </xsl:if>
            <xsl:if test="mods:partName">
                <xsl:text>. </xsl:text>
                <xsl:value-of select="mods:partName"/>
            </xsl:if>
        </dc:title>
    </xsl:template>

    <xsl:template match="mods:name">
        <xsl:if test="mods:role/mods:roleTerm[@type='text'] = 'Creator'">
            <dc:creator>
                <xsl:value-of select="normalize-space(mods:displayForm/text())"/>
            </dc:creator>
        </xsl:if>
    </xsl:template>

    <xsl:template match="mods:subject[mods:topic | mods:name | mods:geographic]">
        <dc:subject>
            <xsl:for-each select="mods:topic">
                <xsl:value-of select="."/>
                <xsl:if test="position()!=last()">--</xsl:if>
            </xsl:for-each>

            <xsl:for-each select="mods:name">
                <xsl:call-template name="name"/>
            </xsl:for-each>
        </dc:subject>

        <xsl:for-each select="mods:geographic">
            <dc:coverage>
                <xsl:value-of select="."/>
            </dc:coverage>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="mods:abstract | mods:note[@displayLabel='Biographical/Historical note']">
        <dc:description>
            <xsl:value-of select="."/>
        </dc:description>
    </xsl:template>

    <xsl:template match="mods:originInfo">
        <xsl:apply-templates select="*[@point='start']"/>
        <xsl:for-each
                select="mods:dateCreated[@point!='start' and @point!='end']">
            <dc:date>
                <xsl:value-of select="."/>
            </dc:date>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="mods:typeOfResource">
        <xsl:if test="@collection='yes'">
            <dc:type>Collection</dc:type>
        </xsl:if>
        <xsl:if test=". ='software' and ../mods:genre='database'">
            <dc:type>DataSet</dc:type>
        </xsl:if>
        <xsl:if test=".='software' and ../mods:genre='online system or service'">
            <dc:type>Service</dc:type>
        </xsl:if>
        <xsl:if test=".='software'">
            <dc:type>Software</dc:type>
        </xsl:if>
        <xsl:if test=".='cartographic material'">
            <dc:type>Image</dc:type>
        </xsl:if>
        <xsl:if test=".='multimedia'">
            <dc:type>InteractiveResource</dc:type>
        </xsl:if>
        <xsl:if test=".='moving image'">
            <dc:type>MovingImage</dc:type>
        </xsl:if>
        <xsl:if test=".='three-dimensional object'">
            <dc:type>PhysicalObject</dc:type>
        </xsl:if>
        <xsl:if test="starts-with(.,'sound recording')">
            <dc:type>Sound</dc:type>
        </xsl:if>
        <xsl:if test=".='still image'">
            <dc:type>StillImage</dc:type>
        </xsl:if>
        <xsl:if test=". ='text'">
            <dc:type>Text</dc:type>
        </xsl:if>
        <xsl:if test=".='notated music'">
            <dc:type>Text</dc:type>
        </xsl:if>
    </xsl:template>

    <xsl:template match="mods:accessCondition">
        <dc:rights>
            <xsl:value-of select="."/>
        </dc:rights>
    </xsl:template>

    <xsl:template name="name">
        <xsl:variable name="name">
            <xsl:for-each select="mods:namePart[not(@type)]">
                <xsl:value-of select="."/>
                <xsl:text> </xsl:text>
            </xsl:for-each>
            <xsl:value-of select="mods:namePart[@type='family']"/>
            <xsl:if test="mods:namePart[@type='given']">
                <xsl:text>, </xsl:text>
                <xsl:value-of select="mods:namePart[@type='given']"/>
            </xsl:if>
            <xsl:if test="mods:namePart[@type='date']">
                <xsl:text>, </xsl:text>
                <xsl:value-of select="mods:namePart[@type='date']"/>
                <xsl:text/>
            </xsl:if>
            <xsl:if test="mods:displayForm">
                <xsl:text> (</xsl:text>
                <xsl:value-of select="mods:displayForm"/>
                <xsl:text>) </xsl:text>
            </xsl:if>
            <xsl:for-each select="mods:role[mods:roleTerm[@type='text']!='creator']">
                <xsl:text> (</xsl:text>
                <xsl:value-of select="normalize-space(.)"/>
                <xsl:text>) </xsl:text>
            </xsl:for-each>
        </xsl:variable>
        <xsl:value-of select="normalize-space($name)"/>
    </xsl:template>

    <!-- suppress all else:-->
    <xsl:template match="*"/>
</xsl:stylesheet>
