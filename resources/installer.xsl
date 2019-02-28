<?xml version="1.0" encoding="UTF-8"?>
<!--
 Copyright 2016 Red Hat, Inc.

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
        <Directory Id="INSTALLDIR" xmlns="http://schemas.microsoft.com/wix/2006/wi">
            <xsl:apply-templates select="@* | *"/>
            <Directory Id="UPDATEDIR" Name="update">
                <Component Id="_5efc6c34_bcd0_4a96_9731_aa0ad8921614" Guid="dcab29b2-5e2e-4ac0-b81b-ff0e4257cfff" Win64="${openjdk_INSTALLER_WIN64_WIX}">
                    <File Id="_c0e26376_eba2_4bc2_91f8_4b9d5469af91" Name="checker.exe" KeyPath="yes" DiskId="1"
                            Source="update/checker.exe"/>
                </Component>
                <Component Id="_7b774e9f_13af_49bf_b3fe_8adf83674223" Guid="0274b7d8-6d3f-41d2-9903-3dc97b363e7b" Win64="${openjdk_INSTALLER_WIN64_WIX}">
                    <File Id="_200d8b28_0acd_43b9_978d_faac6f11d740" Name="checker.vbs" KeyPath="yes" DiskId="1"
                            Source="update/checker.vbs"/>
                </Component>
                <Component Id="_a621153a_aacd_4755_bb6d_134f24411ba1" Guid="cc7a0b60-ce7a-4ef2-b081-eaa448dfb8c4" Win64="${openjdk_INSTALLER_WIN64_WIX}">
                    <File Id="_3fddad02_1300_4590_9f16_e26fa1172558" Name="checker.crt" KeyPath="yes" DiskId="1"
                            Source="update/checker.crt"/>
                </Component>
                <Component Id="_0e25658d_02f0_490e_8c00_b91b99397a45" Guid="841da243-4edb-4895-be71-54f2dbb9b8fc" Win64="${openjdk_INSTALLER_WIN64_WIX}">
                    <File Id="_a051ecac_87fa_45fa_9d28_9c55799cf6ff" Name="notifier.exe" KeyPath="yes" DiskId="1"
                            Source="update/notifier.exe"/>
                </Component>
                <Component Id="_c393a906_51fe_4a26_b1d8_df8d9297fb9c" Guid="eebfcd98-7d44-4bf1-87a2-7b1bf243d7e5" Win64="${openjdk_INSTALLER_WIN64_WIX}">
                    <File Id="_8d0555db_4d3c_4a8c_920a_caf43b3aeb28" Name="config.json" KeyPath="yes" DiskId="1"
                            Source="update/config.json"/>
                </Component>
                <Component Id="_ee5a58db_81ac_46d2_b628_67a9123dee16" Guid="f838cfb5-dfd1-4245-b26a-67d61b7b171b" Win64="${openjdk_INSTALLER_WIN64_WIX}">
                    <File Id="_1818b11f_0301_44a1_a9b1_26d6a17cf2e1" Name="icon.bmp" KeyPath="yes" DiskId="1" 
                            Source="update/icon.bmp"/>
                </Component>
            </Directory>
        </Directory>
    </xsl:template>
    <xsl:template match="w:Feature[@ConfigurableDirectory='INSTALLDIR']">
        <Feature Id="jdk" xmlns="http://schemas.microsoft.com/wix/2006/wi">
            <xsl:apply-templates select="@* | *"/>
        </Feature>
        <Feature Id="update_notifier" ConfigurableDirectory="UPDATEDIR" Absent="allow" AllowAdvertise="no" Level="${${PROJECT_NAME}_INSTALLER_FEATURE_LEVEL}" 
                Title="${${PROJECT_NAME}_INSTALLER_FEATURE_TITLE}"
                Description="Update Notifier checks availability of a new product version online and displays UI notification. This feature installs two tasks for Windows Task Scheduler."
                xmlns="http://schemas.microsoft.com/wix/2006/wi">
            <ComponentRef Id="_5efc6c34_bcd0_4a96_9731_aa0ad8921614"/>
            <ComponentRef Id="_7b774e9f_13af_49bf_b3fe_8adf83674223"/>
            <ComponentRef Id="_a621153a_aacd_4755_bb6d_134f24411ba1"/>
            <ComponentRef Id="_0e25658d_02f0_490e_8c00_b91b99397a45"/>
            <ComponentRef Id="_c393a906_51fe_4a26_b1d8_df8d9297fb9c"/>
            <ComponentRef Id="_ee5a58db_81ac_46d2_b628_67a9123dee16"/>
        </Feature>

        <!-- impersonated -->
        <CustomAction Id="checker_cleanup_impersonated_prop" Property="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}CmdLine" Value="&quot;[UPDATEDIR]checker.exe&quot; -d" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="checker_cleanup_impersonated" BinaryKey="WixCA" DllEntry="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}" Return="ignore" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>

        <!-- immediate -->
        <CustomAction Id="checker_preregister_immediate" Property="checker_preregister_deferred" Value="&quot;[SystemFolder]schtasks.exe&quot; /delete /f /tn ${${PROJECT_NAME}_INSTALLER_CHECKER_TASK_NAME}" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="notifier_preregister_immediate" Property="notifier_preregister_deferred" Value="&quot;[SystemFolder]schtasks.exe&quot; /delete /f /tn ${${PROJECT_NAME}_INSTALLER_NOTIFIER_TASK_NAME}" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="checker_register_immediate" Property="checker_register_deferred" Value="&quot;[SystemFolder]schtasks.exe&quot; /create /tn ${${PROJECT_NAME}_INSTALLER_CHECKER_TASK_NAME} ${${PROJECT_NAME}_INSTALLER_CHECKER_SCHEDULE} /tr &quot;wscript \&quot;[UPDATEDIR]checker.vbs\&quot;&quot;" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="notifier_register_immediate" Property="notifier_register_deferred" Value="&quot;[SystemFolder]schtasks.exe&quot; /create /tn ${${PROJECT_NAME}_INSTALLER_NOTIFIER_TASK_NAME} ${${PROJECT_NAME}_INSTALLER_NOTIFIER_SCHEDULE} /tr &quot;\&quot;[UPDATEDIR]notifier.exe\&quot;&quot;" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="checker_unregister_immediate" Property="checker_unregister_deferred" Value="&quot;[SystemFolder]schtasks.exe&quot; /delete /f /tn ${${PROJECT_NAME}_INSTALLER_CHECKER_TASK_NAME}" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="notifier_unregister_immediate" Property="notifier_unregister_deferred" Value="&quot;[SystemFolder]schtasks.exe&quot; /delete /f /tn ${${PROJECT_NAME}_INSTALLER_NOTIFIER_TASK_NAME}" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>

        <!-- deferred -->
        <CustomAction Id="checker_preregister_deferred" BinaryKey="WixCA" DllEntry="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}" Return="ignore" Execute="deferred" Impersonate="no" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="notifier_preregister_deferred" BinaryKey="WixCA" DllEntry="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}" Return="ignore" Execute="deferred" Impersonate="no" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="checker_register_deferred" BinaryKey="WixCA" DllEntry="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}" Return="ignore" Execute="deferred" Impersonate="no" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="notifier_register_deferred" BinaryKey="WixCA" DllEntry="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}" Return="ignore" Execute="deferred" Impersonate="no" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="checker_unregister_deferred" BinaryKey="WixCA" DllEntry="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}" Return="ignore" Execute="deferred" Impersonate="no" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>
        <CustomAction Id="notifier_unregister_deferred" BinaryKey="WixCA" DllEntry="WixQuietExec${openjdk_INSTALLER_WIN64_EXEC_WIX}" Return="ignore" Execute="deferred" Impersonate="no" xmlns="http://schemas.microsoft.com/wix/2006/wi"/>

        <InstallExecuteSequence xmlns="http://schemas.microsoft.com/wix/2006/wi">
            <!-- impersonated -->
            <Custom Action="checker_cleanup_impersonated_prop" Before="checker_cleanup_impersonated"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="checker_cleanup_impersonated" Before="RemoveFiles"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>

            <!-- immediate -->
            <Custom Action="checker_preregister_immediate" Before="InstallInitialize"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="notifier_preregister_immediate" Before="InstallInitialize"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="checker_register_immediate" Before="InstallInitialize"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="notifier_register_immediate" Before="InstallInitialize"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="checker_unregister_immediate" Before="InstallInitialize"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="notifier_unregister_immediate" Before="InstallInitialize"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>

            <!-- deferred -->
            <Custom Action="checker_preregister_deferred" Before="notifier_preregister_deferred"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="notifier_preregister_deferred" Before="checker_register_deferred"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="checker_register_deferred" Before="notifier_register_deferred"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="notifier_register_deferred" Before="InstallFinalize"><![CDATA[&update_notifier=3 AND NOT !update_notifier=3]]></Custom>
            <Custom Action="checker_unregister_deferred" Before="notifier_unregister_deferred"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
            <Custom Action="notifier_unregister_deferred" Before="InstallFinalize"><![CDATA[!update_notifier=3 AND REMOVE]]></Custom>
        </InstallExecuteSequence>

    </xsl:template>
</xsl:stylesheet>
