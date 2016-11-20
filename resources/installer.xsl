<?xml version="1.0" encoding="UTF-8"?>
<!--
 Copyright 2016, akashche at redhat.com

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-->
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
                    <File Id="_200d8b28_0acd_43b9_978d_faac6f11d740" Name="checker.vbs" KeyPath="yes" DiskId="1"
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
        <Feature Id="update_notifier" ConfigurableDirectory="UPDATEDIR" Absent="allow" AllowAdvertise="no" Level="${${PROJECT_NAME}_INSTALLER_FEATURE_LEVEL}" 
                Title="Updater Notifier"
                Description="Update Notifier checks availability of a new product version online and displays UI notification. This feature installs two tasks for Windows Task Scheduler.">
            <ComponentRef Id="_5efc6c34_bcd0_4a96_9731_aa0ad8921614"/>
            <ComponentRef Id="_7b774e9f_13af_49bf_b3fe_8adf83674223"/>
            <ComponentRef Id="_a621153a_aacd_4755_bb6d_134f24411ba1"/>
            <ComponentRef Id="_0e25658d_02f0_490e_8c00_b91b99397a45"/>
            <ComponentRef Id="_c393a906_51fe_4a26_b1d8_df8d9297fb9c"/>
        </Feature>
        <Property Id="WixQuietExec64CmdLine" Value=" "/>
        <CustomAction Id="_268df76e_333f_4d09_98cd_bcab59395e2c" Property="WixQuietExec64CmdLine" Value="&quot;[SystemFolder]schtasks.exe&quot; /create /tn ${${PROJECT_NAME}_INSTALLER_CHECKER_TASK_NAME} ${${PROJECT_NAME}_INSTALLER_CHECKER_SCHEDULE} /tr &quot;wscript \&quot;[UPDATEDIR]checker.vbs\&quot;&quot;"/>
        <CustomAction Id="_ad90e97e_77ee_4f1f_b127_d054239e8174" BinaryKey="WixCA" DllEntry="WixQuietExec64" Return="ignore"/>
        <CustomAction Id="_7e6b83ae_8120_40e2_a2fd_bcc4302b5ac1" Property="WixQuietExec64CmdLine" Value="&quot;[SystemFolder]schtasks.exe&quot; /create /tn ${${PROJECT_NAME}_INSTALLER_NOTIFIER_TASK_NAME} ${${PROJECT_NAME}_INSTALLER_NOTIFIER_SCHEDULE} /tr &quot;[UPDATEDIR]notifier.exe&quot;"/>
        <CustomAction Id="_e2b0a4e7_3b5f_4c08_83f4_e9b692ccf518" BinaryKey="WixCA" DllEntry="WixQuietExec64" Return="ignore"/>
        <CustomAction Id="_79ca21fa_3a02_4a9f_b8ba_767b0a80e4e6" Property="WixQuietExec64CmdLine" Value="&quot;[UPDATEDIR]checker.exe&quot; -d"/>
        <CustomAction Id="_3c33d055_b0b1_46a6_b394_f1214e39ce0f" BinaryKey="WixCA" DllEntry="WixQuietExec64" Return="ignore"/>
        <CustomAction Id="_6ed57788_9ade_421f_bd9f_7a0d32b5242f" Property="WixQuietExec64CmdLine" Value="&quot;[SystemFolder]schtasks.exe&quot; /delete /f /tn ${${PROJECT_NAME}_INSTALLER_CHECKER_TASK_NAME}"/>
        <CustomAction Id="_ce2a8b40_5ca6_45bd_bdfa_a413be73f8ba" BinaryKey="WixCA" DllEntry="WixQuietExec64" Return="ignore"/>
        <CustomAction Id="_a6864546_7d27_41c0_abb4_356d81cf31b8" Property="WixQuietExec64CmdLine" Value="&quot;[SystemFolder]schtasks.exe&quot; /delete /f /tn ${${PROJECT_NAME}_INSTALLER_NOTIFIER_TASK_NAME}"/>
        <CustomAction Id="_7770bf24_eea0_4b01_bec3_094c621b11f4" BinaryKey="WixCA" DllEntry="WixQuietExec64" Return="ignore"/>
        <InstallExecuteSequence>
            <Custom Action="_268df76e_333f_4d09_98cd_bcab59395e2c" After="InstallFinalize"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="_ad90e97e_77ee_4f1f_b127_d054239e8174" After="_268df76e_333f_4d09_98cd_bcab59395e2c"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="_7e6b83ae_8120_40e2_a2fd_bcc4302b5ac1" After="_ad90e97e_77ee_4f1f_b127_d054239e8174"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="_e2b0a4e7_3b5f_4c08_83f4_e9b692ccf518" After="_7e6b83ae_8120_40e2_a2fd_bcc4302b5ac1"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="_79ca21fa_3a02_4a9f_b8ba_767b0a80e4e6" Before="_3c33d055_b0b1_46a6_b394_f1214e39ce0f"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="_3c33d055_b0b1_46a6_b394_f1214e39ce0f" Before="_6ed57788_9ade_421f_bd9f_7a0d32b5242f"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="_6ed57788_9ade_421f_bd9f_7a0d32b5242f" Before="_ce2a8b40_5ca6_45bd_bdfa_a413be73f8ba"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="_ce2a8b40_5ca6_45bd_bdfa_a413be73f8ba" Before="_a6864546_7d27_41c0_abb4_356d81cf31b8"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="_a6864546_7d27_41c0_abb4_356d81cf31b8" Before="_7770bf24_eea0_4b01_bec3_094c621b11f4"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="_7770bf24_eea0_4b01_bec3_094c621b11f4" Before="RemoveFiles"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
        </InstallExecuteSequence>
    </xsl:template>
</xsl:stylesheet>
