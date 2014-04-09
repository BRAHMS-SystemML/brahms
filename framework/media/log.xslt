<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- 

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitchinson
	URL: http://brahms.sourceforge.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!--
<xsl:variable name="basecol">blue</xsl:variable>
<xsl:copy-of select="$basecol"/>
-->



<xsl:template match="/">
	<html>
		<head>
			<style>

				/* common */
				* { font-family:Segoe UI; }
				body { font-size:10pt; background-color:white; margin:0; }

				/* timestamp */
				.t { color:blue; margin:0 1em 0 1em; }

				/* thread markers */
				.C, .W, .S, .R, .D { border:solid 1px black; font-weight:bold; margin-left:32px; font-size:80%; display:inline-block; text-align:center; width:32px; }
				.C { background-color:red; color:white; }
				.W { background-color:orange; color:black; }
				.S { background-color:#9900FF; color:white; }
				.R { background-color:#66bbFF; color:white; }
				.D { background-color:black; color:white; }

				/* errors */
				.error { margin-bottom:8px; color:red; }
				.trace { color:#600; padding-left:2em; }

				/* hilites */
				.hilite { border:solid 1px black; font-size:80%; padding:0 0.2em 0 0.2em; background-color:#090; color:white; font-weight:bold; }
				.hiliteerror { border:solid 1px black; font-size:80%; padding:0 0.2em 0 0.2em; background-color:red; color:white; font-weight:bold; }
				.warning { border:solid 1px black; font-size:80%; padding:0 0.2em 0 0.2em; background-color:orange; color:white; font-weight:bold; }

				/* layout */
				.header { background-color:#ccc; border:solid red; border-width:0 0 0.2em 0; padding:0.5em;
					font-size:160%; font-weight:bold; color:blue; }
				.content { padding:1em; }
				.footer { background-color:#ccc; border:solid red; border-width:0.2em 0 0 0; padding:1em; }

				/* sections */
				.sect { margin:2px; background-color:yellow; color:black; border:solid 1px black; display:inline-block; font-weight:bold; font-size:80%; padding:4px; }

			</style>
		</head>
		<body>
			<div class="header">
			<div class="title">BRAHMS log (Voice 
				<xsl:value-of select="log/data/voiceindex"/>
				of
				<xsl:value-of select="log/data/voicecount"/>)
			</div>
			</div>
			<div class="content">
				<xsl:apply-templates select="log"/>
			</div>
			<div class="footer">
				<xsl:apply-templates select="log/errors/error"/>
			</div>
		</body>
	</html>
</xsl:template>

<xsl:template match="log">
	<div class="log">
		<xsl:apply-templates select="msg|sect"/>
	</div>
</xsl:template>

<xsl:template match="sect">
	<div class="sect">
	<xsl:value-of select="@lvl"/> : <xsl:value-of select="@key"/>
	</div><br/>
	<xsl:apply-templates select="msg|sect"/>
</xsl:template>

<xsl:template match="msg">
	<xsl:param name="id"/>
	<xsl:variable name="threadclass"><xsl:value-of select="substring(@i,1,1)"/></xsl:variable>
	
	<xsl:variable name="threadpad">
		<xsl:choose>
			<xsl:when test="$threadclass='W'">
				<xsl:value-of select="substring(@i,2) + 1"/>em
			</xsl:when>
			<xsl:when test="$threadclass='S'">
				<xsl:value-of select="substring(@i,2) + 1 + 8"/>em
			</xsl:when>
			<xsl:when test="$threadclass='R'">
				<xsl:value-of select="substring(@i,2) + 1 + 12"/>em
			</xsl:when>
			<xsl:when test="$threadclass='D'">
				<xsl:value-of select="substring(@i,2) + 1 + 16"/>em
			</xsl:when>
			<xsl:otherwise>
				0
			</xsl:otherwise>
		</xsl:choose>
	</xsl:variable>

	<div class="msg" style="padding-left:{$threadpad}">
		<span class="{$threadclass}"><xsl:value-of select="@i"/></span>
		<span class="t"><xsl:value-of select="@t"/></span>
		<xsl:choose>
			<xsl:when test="@f='h'">
				<span class="hilite">
					<xsl:value-of select="."/>
				</span>
			</xsl:when>
			<xsl:when test="@f='e'">
				<span class="hiliteerror">
					<xsl:value-of select="."/>
				</span>
			</xsl:when>
			<xsl:when test="@f='w'">
				<span class="warning">
					<xsl:value-of select="."/>
				</span>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="."/>
			</xsl:otherwise>
		</xsl:choose>
	</div>
</xsl:template>

<xsl:template match="error">
	<div class="error">
		<xsl:value-of select="code"/>:
		<xsl:value-of select="msg"/>
		<xsl:apply-templates select="trace"/>
	</div>
</xsl:template>

<xsl:template match="trace">
	<div class="trace">
		<xsl:value-of select="."/>
	</div>
</xsl:template>

</xsl:stylesheet>

