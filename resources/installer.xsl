<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:w="http://schemas.microsoft.com/wix/2006/wi" exclude-result-prefixes="w">
    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>
    <xsl:template match="w:Directory[@Id='INSTALLDIR']">
        <Directory>
            <xsl:apply-templates select="@* | *"/>
            <Directory Id="UPDATEDIR" Name="update">
                <Component Id="_5efc6c34_bcd0_4a96_9731_aa0ad8921614" Guid="dcab29b2-5e2e-4ac0-b81b-ff0e4257cfff" Win64="yes">
                    <File Id="_c0e26376_eba2_4bc2_91f8_4b9d5469af91" Name="checker.exe" KeyPath="yes" DiskId="1"
                            Source="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/checker.exe"/>
                </Component>
                <Component Id="_7b774e9f_13af_49bf_b3fe_8adf83674223" Guid="0274b7d8-6d3f-41d2-9903-3dc97b363e7b" Win64="yes">
                    <File Id="_200d8b28_0acd_43b9_978d_faac6f11d740" Name="notifier.vbs" KeyPath="yes" DiskId="1"
                            Source="${CMAKE_CURRENT_LIST_DIR}/resources/checker.vbs"/>
                </Component>
                <Component Id="_a621153a_aacd_4755_bb6d_134f24411ba1" Guid="cc7a0b60-ce7a-4ef2-b081-eaa448dfb8c4" Win64="yes">
                    <File Id="_3fddad02_1300_4590_9f16_e26fa1172558" Name="checker.crt" KeyPath="yes" DiskId="1"
                            Source="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/checker.crt"/>
                </Component>
                <Component Id="_0e25658d_02f0_490e_8c00_b91b99397a45" Guid="841da243-4edb-4895-be71-54f2dbb9b8fc" Win64="yes">
                    <File Id="_a051ecac_87fa_45fa_9d28_9c55799cf6ff" Name="notifier.exe" KeyPath="yes" DiskId="1"
                            Source="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/notifier.exe"/>
                </Component>
                <Component Id="_c393a906_51fe_4a26_b1d8_df8d9297fb9c" Guid="eebfcd98-7d44-4bf1-87a2-7b1bf243d7e5" Win64="yes">
                    <File Id="_8d0555db_4d3c_4a8c_920a_caf43b3aeb28" Name="config.json" KeyPath="yes" DiskId="1"
                            Source="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json"/>
                </Component>
            </Directory>
        </Directory>
    </xsl:template>
    <xsl:template match="w:Feature[@ConfigurableDirectory='INSTALLDIR']">
        <Feature>
            <xsl:apply-templates select="@* | *"/>
        </Feature>
        <Feature Id="update_notifier" ConfigurableDirectory="UPDATEDIR" Absent="allow" AllowAdvertise="no" Level="2" 
                Title="Updater Notifier"
                Description="TODO: description">
            <ComponentRef Id="_5efc6c34_bcd0_4a96_9731_aa0ad8921614"/>
            <ComponentRef Id="_7b774e9f_13af_49bf_b3fe_8adf83674223"/>
            <ComponentRef Id="_a621153a_aacd_4755_bb6d_134f24411ba1"/>
            <ComponentRef Id="_0e25658d_02f0_490e_8c00_b91b99397a45"/>
            <ComponentRef Id="_c393a906_51fe_4a26_b1d8_df8d9297fb9c"/>
        </Feature>
    </xsl:template>
</xsl:stylesheet>
