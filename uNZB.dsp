# Microsoft Developer Studio Project File - Name="uNZB" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=uNZB - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "uNZB.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uNZB.mak" CFG="uNZB - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uNZB - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "uNZB - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "uNZB - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\tinyxml\\" /I ".\ssl\include\\" /I ".\\" /FI"pragma.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib shlwapi.lib /nologo /subsystem:windows /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx ".\release\uNZB.exe"	copy .\release\unzb.exe C:\PROGRA~1\uNZB\uNZB.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "uNZB - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\tinyxml\\" /I ".\ssl\include\\" /I ".\\" /FI"pragma.h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib shlwapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "uNZB - Win32 Release"
# Name "uNZB - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "xml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tinyxml\tinystr.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxml.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxmlerror.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml\tinyxmlparser.cpp
# End Source File
# End Group
# Begin Group "ssl"

# PROP Default_Filter ""
# Begin Group "library"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ssl\library\aes.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\arc4.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\asn1parse.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\base64.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\bignum.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\camellia.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\certs.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\cipher.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\cipher_wrap.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ctr_drbg.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\debug.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\des.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\dhm.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\entropy.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\entropy_poll.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\error.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\havege.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md2.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md4.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md5.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\md_wrap.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\net.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\padlock.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\pem.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\rsa.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\sha1.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\sha2.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\sha4.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ssl_cli.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ssl_srv.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\ssl_tls.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\timing.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\x509parse.c
# End Source File
# Begin Source File

SOURCE=.\ssl\library\xtea.c
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\debug_print.c
# End Source File
# Begin Source File

SOURCE=.\ini_file.c
# End Source File
# Begin Source File

SOURCE=.\listview.c
# End Source File
# Begin Source File

SOURCE=.\network.c
# End Source File
# Begin Source File

SOURCE=.\nzb_files.cpp
# End Source File
# Begin Source File

SOURCE=.\resize.c
# End Source File
# Begin Source File

SOURCE=.\uNZB.c
# End Source File
# Begin Source File

SOURCE=.\xml.cpp
# End Source File
# Begin Source File

SOURCE=.\ydecode.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\pragma.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\Script1.rc
# End Source File
# End Group
# End Target
# End Project
