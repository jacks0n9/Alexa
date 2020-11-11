@echo off
rem # *************************************************************************
rem #  FILE           : cygwinxdc.bat
rem #  DESCRIPTION    :
rem #
rem #     This DOS batch scripts installs compile and build tools for AFE pakage
rem #     NOTE: THIS BATCH FILE IS FOR TI INTERNAL USE ONLY. DO NOT MODIFY.
rem #
rem # (C) Copyright 2007 Texas Instruments Incorporated
rem # *************************************************************************

rem # *************************************************************************
rem # *** Set up tools version (which are root directory)
rem # *************************************************************************
set DSPBIOSBASE=bios_5_31_08
set CODEGENBASE=cgen6_0_15
set XDAISBASE=xdais_5_21
set XDCBASE=xdc_2_94_01
set CSLBASE=csl_DAVINCI_v3_00_05_02
set PERLBASE=gen\perl\activestate\5_6_1_635
set DOXYGENBASE=1.5.1-p1
set GRAPHVIZBASE=2.12
set HTML_HELP_WORKSHOP_BASE=10-01-2007
set TI_TEMPLATES_BASE=10-01-2007

rem # *************************************************************************
rem # *** Tools Drive ***
rem # *************************************************************************
set LOCAL_TOOLSDIR_DOS=C:\tools
set LOCAL_TOOLSDIR_UNIX=//C/tools
set TOOLSDRIVE_DOS=T:
set TOOLSDRIVE_UNIX=//T

rem # *************************************************************************
rem # *** cpytool.pl ***
rem # *************************************************************************
set CPYTOOLS=..\swtools\cpytools.pl

rem # *************************************************************************
rem # *** PERL ***
rem #
rem # NOTE: If you wish to run perl from your local hard drive, you must
rem #       relocate perl to local drive. To relocate perl, go to 
rem #       T:\gen\perl\activestate\5_6_1_635\bin and run 
rem #       "reloc_perl -t -a C:\tools\gen\perl\activestate\5_6_1_635 
rem #       T:\gen\perl\activestate\5_6_1_635".
rem #       This will install perl in C:\tools. You can change the following
rem #       two paths to point to C:\tools\gen\perl\activestate\5_6_1_635.
rem #
rem #       CCPERL cannot run reflection page script (clrcase/refpage/refpage.pl)
rem #       because it has a bug in its regexp engine that crashes on some of 
rem #       the complicated regexp in the script.
rem #
rem # *************************************************************************
set PERLEXE=%PERLBASE%\bin\perl.exe

if not exist "%LOCAL_TOOLSDIR_DOS%\%PERLEXE%" goto remoteperl

:localperl
rem # ** Currently perl is run from the local copy
set PERLDOS=%LOCAL_TOOLSDIR_DOS%\%PERLEXE%
set PERLPATH=%LOCAL_TOOLSDIR_DOS%\%PERLBASE%
goto endperl

:remoteperl
rem # ** Currently perl is run from the networked copy
set PERLDOS=%TOOLSDRIVE_DOS%\%PERLEXE%
set PERLPATH=%TOOLSDRIVE_DOS%\%PERLBASE%

:endperl

rem # *************************************************************************
rem # ** Create source and destination for all tools required to compile AFE
rem # ** AFE needs CGEN, DSP/BIOS and DAVINCI CSL 
rem # *************************************************************************

rem # XDC Tools
set TOOLXDCSRC=%TOOLSDRIVE_DOS%\gen\xdc\%XDCBASE%
set TOOLXDCDST=%LOCAL_TOOLSDIR_DOS%\gen\xdc\%XDCBASE%

rem # XDC extensions
set XDCPKGSRC=%TOOLSDRIVE_DOS%\gen\xdcgtx
set XDCPKGDST=%LOCAL_TOOLSDIR_DOS%\gen\xdcgtx

rem # C64+ Codegen Tools
set TOOLC64XPLUSSRC=%TOOLSDRIVE_DOS%\c6xx\%CODEGENBASE%\c6000
set TOOLC64XPLUSDST=%LOCAL_TOOLSDIR_DOS%\c6xx\%CODEGENBASE%\c6000

rem # DSP/BIOS
set TOOLBIOSC64XPLUSSRC=%TOOLSDRIVE_DOS%\c6xx\%DSPBIOSBASE%
set TOOLBIOSC64XPLUSDST=%LOCAL_TOOLSDIR_DOS%\c6xx\%DSPBIOSBASE%

rem # CSL
set TOOLCSLDAVINCISRC=%TOOLSDRIVE_DOS%\c6xx\%CSLBASE%
set TOOLCSLDAVINCIDST=%LOCAL_TOOLSDIR_DOS%\c6xx\%CSLBASE%

rem # XDAIS
set TOOLXDAISSRC=%TOOLSDRIVE_DOS%\Xdais\%XDAISBASE%
set TOOLXDAISDST=%LOCAL_TOOLSDIR_DOS%\Xdais\%XDAISBASE%

rem # Doxygen
set TOOLDOXYGENSRC=%TOOLSDRIVE_DOS%\Doxygen\doxygen\%DOXYGENBASE%
set TOOLDOXYGENDST=%LOCAL_TOOLSDIR_DOS%\Doxygen\doxygen\%DOXYGENBASE%

rem # Graphviz
set TOOLGRAPHVIZSRC=%TOOLSDRIVE_DOS%\Doxygen\Graphviz\%GRAPHVIZBASE%
set TOOLGRAPHVIZDST=%LOCAL_TOOLSDIR_DOS%\Doxygen\Graphviz\%GRAPHVIZBASE%

rem # HTML Help Workshop
set TOOLHTMLHWSRC=%TOOLSDRIVE_DOS%\Doxygen\HTML_Help_Workshop\%HTML_HELP_WORKSHOP_BASE%
set TOOLHTMLHWDST=%LOCAL_TOOLSDIR_DOS%\Doxygen\HTML_Help_Workshop\%HTML_HELP_WORKSHOP_BASE%

rem # TI_Templates
set TOOLTITEMPSRC=%TOOLSDRIVE_DOS%\Doxygen\TI_Templates\%TI_TEMPLATES_BASE%
set TOOLTITEMPDST=%LOCAL_TOOLSDIR_DOS%\Doxygen\TI_Templates\%TI_TEMPLATES_BASE%

if "%PERLDOS%" == "" goto environment

rem # *************************************************************************
rem # *** Tools Copy / Validation ***
rem # *************************************************************************
rem # If "bypass" option is provided the skip tools validation
if "%1" == "bypass" goto environment

rem # If "bypass" option is not provided them validate the tools, but don't copy
if "%1" == "" goto toolcheck

rem # If "enable_write" is specified copy the tools
if "%1" == "enable_write" goto toolcopy

rem # Invalid argument supplied
echo ERROR: Option "%1" supplied is invalid...
echo .
echo . Usage: %0 [bypass enable_write]
echo .
echo .     "bypass"       : Configures environment without checking the tools
echo .     "enable_write" : Erase non-matching files and replace with correct tools
goto end

:toolcheck
echo Verifying local tools...
goto toolperl

:toolcopy
echo Updating local tools...

:toolperl

rem # NOTE: in windows "errorlevel 1" means "retval >= 1"

rem # Verify or copy XDC tools
%PERLDOS% ..\swtools\cpytools.pl %TOOLXDCSRC% %TOOLXDCDST% %1
if errorlevel 1 goto error

rem # Verify or copy the XDC Extensions
%PERLDOS% %CPYTOOLS% %XDCPKGSRC% %XDCPKGDST% %1
if errorlevel 1 goto error

rem # Verify or copy the codegen tools
%PERLDOS% ..\swtools\cpytools.pl %TOOLC64XPLUSSRC% %TOOLC64XPLUSDST% %1
if errorlevel 1 goto error

rem # Verify or copy DSP/BIOS
%PERLDOS% ..\swtools\cpytools.pl %TOOLBIOSC64XPLUSSRC% %TOOLBIOSC64XPLUSDST% %1
if errorlevel 1 goto error

rem # Verify or copy CSL DAVINCI
%PERLDOS% ..\swtools\cpytools.pl %TOOLCSLDAVINCISRC% %TOOLCSLDAVINCIDST% %1
if errorlevel 1 goto error

rem # Verify or copy XDAIS
%PERLDOS% ..\swtools\cpytools.pl %TOOLXDAISSRC% %TOOLXDAISDST% %1
if errorlevel 1 goto error

rem # Verify or copy Doxygen\Doxygen
%PERLDOS% %CPYTOOLS% %TOOLDOXYGENSRC% %TOOLDOXYGENDST% %1
if errorlevel 1 goto error

rem # Verify or copy Doxygen\Graphviz
%PERLDOS% %CPYTOOLS% %TOOLGRAPHVIZSRC% %TOOLGRAPHVIZDST% %1
if errorlevel 1 goto error

rem # Verify or copy Doxygen\HTML_Help_Workshop
%PERLDOS% %CPYTOOLS% %TOOLHTMLHWSRC% %TOOLHTMLHWDST% %1
if errorlevel 1 goto error

rem # Verify or copy Doxygen\TI_Templates
%PERLDOS% %CPYTOOLS% %TOOLTITEMPSRC% %TOOLTITEMPDST% %1
if errorlevel 1 goto error

echo Local tools are good...

rem # **************************************************************************
rem # Establish environment
rem # **************************************************************************
:environment

rem # *************************************************************************
rem # ** Set the PATH
rem # *************************************************************************
rem # Windows path
set PATH=%SystemRoot%;%SystemRoot%\system32
rem # Add XDC path
set PATH=%PATH%;%LOCAL_TOOLSDIR_DOS%\gen\xdc\%XDCBASE%
rem # Add clearcase path
set PATH=%PATH%;C:\Program Files\Rational\ClearCase\bin
rem # Add Doxygen path
set PATH=%PATH%;%LOCAL_TOOLSDIR_DOS%\Doxygen\doxygen\%DOXYGENBASE%\bin
set PATH=%PATH%;%LOCAL_TOOLSDIR_DOS%\Doxygen\Graphviz\%GRAPHVIZBASE%\bin
set PATH=%PATH%;%LOCAL_TOOLSDIR_DOS%\Doxygen\HTML_Help_Workshop\%HTML_HELP_WORKSHOP_BASE%
rem # Add PERL path
set PATH=%PATH%;%PERLPATH%\bin

rem # *************************************************************************
rem # ** Create environment variables for XDC
rem # *************************************************************************
set XDCPATH=%LOCAL_TOOLSDIR_DOS%\gen\xdcgtx\sdoarch_standards_1_00_00_05\packages

rem # Set XDAIS package path
set XDCPATH=%XDCPATH%;%LOCAL_TOOLSDIR_DOS%\Xdais\%XDAISBASE%\packages

rem # DSP/BIOS installation directory
set BIOS_INSTALL_DIR=%LOCAL_TOOLSDIR_DOS%\c6xx\%DSPBIOSBASE%

rem # Set BIOS package path
set XDCPATH=%XDCPATH%;%BIOS_INSTALL_DIR%\packages

rem # CSL installation directory
set CSL_INSTALL_DIR=%LOCAL_TOOLSDIR_DOS%\c6xx\%CSLBASE%

rem # Codegen installation directory
set CGEN_INSTALL_DIR=%LOCAL_TOOLSDIR_DOS%\c6xx\%CODEGENBASE%

echo .
echo . *** NOTE: THIS BATCH FILE IS FOR TI INTERNAL USE ONLY ***
echo .
echo .  Environment successfully configured...
echo .
echo .  XDCVERSION          = %XDCBASE%
echo .  DSPBIOSVERSION      = %DSPBIOSBASE%
echo .  CODEGENVERSION      = %CODEGENBASE%
echo .  CSLVERSION          = %CSLBASE%
echo .  XDAISVERSION        = %XDAISBASE%
echo .  DOXYGENVERSION      = %DOXYGENBASE%
echo .  GRAPGVIZVERSION     = %GRAPHVIZBASE%
echo .  HTMLHELPWORKSHOP    = %HTML_HELP_WORKSHOP_BASE%
echo .  TITEMPLATES         = %TI_TEMPLATES_BASE%
echo .  BIOS_INSTALL_DIR    = %BIOS_INSTALL_DIR%
echo .  CSL_INSTALL_DIR     = %CSL_INSTALL_DIR%
echo .  CGEN_INSTALL_DIR    = %CGEN_INSTALL_DIR%
echo .

goto end

rem ************************** Bad Tools *************************************
rem Tools are bad; deconfigure environment and print error
rem **************************************************************************
:error

set MAKEDEPPATH=

echo .
echo . *** NOTE: THIS BATCH FILE IS FOR TI INTERNAL USE ONLY ***
echo .
echo . ERROR: Bad tools. Build environment is NOT configured. 
echo .        You may re-run the script with following options:
echo .
echo . %0 bypass
echo . 
echo .    This option configures the environment without checking the tools.
echo .    This is used when user manually configures the tools.
echo .
echo . %0 enable_write
echo .
echo .    [CAUTION]: 1) This option recursively deletes non-identical files in the 
echo .                  following directories:
echo .                  %TOOLXDCDST%
echo .                  %TOOLC64XPLUSDST% 
echo .                  %TOOLBIOSC64XPLUSDST%
echo .                  %TOOLCSLDAVINCIDST%
echo .                  %TOOLXDAISDST%
echo .                  %TOOLDOXYGENDST%
echo .                  %TOOLGRAPHVIZDST%
echo .                  %TOOLHTMLHWDST%
echo .                  %TOOLTITEMPDST%
echo .
echo .               2) And copies the unmatched files from the following directories.
echo .                  %TOOLXDCSRC%
echo .                  %TOOLC64XPLUSSRC% 
echo .                  %TOOLBIOSC64XPLUSSRC%
echo .                  %TOOLCSLDAVINCISRC%
echo .                  %TOOLXDAISSRC%
echo .                  %TOOLDOXYGENSRC%
echo .                  %TOOLGRAPHVIZSRC%
echo .                  %TOOLHTMLHWSRC%
echo .                  %TOOLTITEMPSRC%
echo .

goto end

:end

rem # *************************************************************************
rem # Remove all unused environment variables
rem # *************************************************************************
set XDCBASE=
set CODEGENBASE=
set DSPBIOSBASE=
set CSLBASE=
set LOCAL_TOOLSDIR_DOS=
set LOCAL_TOOLSDIR_UNIX=
set TOOLSDRIVE_DOS=
set TOOLSDRIVE_UNIX=
set PERLBASE=
set PERLEXE=
set PERLDOS=
set PERLPATH=
set TOOLXDCSRC=
set TOOLXDCDST=
set XDCPKGSRC=
set XDCPKGDST=
set TOOLC64XPLUSSRC=
set TOOLC64XPLUSDST=
set TOOLBIOSC64XPLUSSRC=
set TOOLBIOSC64XPLUSDST=
set TOOLCSLDAVINCISRC=
set TOOLCSLDAVINCIDST=
set TOOLXDAISSRC=
set TOOLXDAISDST=
set TOOLDOXYGENSRC=
set TOOLDOXYGENDST=
set TOOLGRAPHVIZSRC=
set TOOLGRAPHVIZDST=
set TOOLHTMLHWSRC=
set TOOLHTMLHWDST=
set TOOLTITEMPSRC=
set TOOLTITEMPDST=

rem # End of file

