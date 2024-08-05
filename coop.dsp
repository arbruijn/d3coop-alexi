# Microsoft Developer Studio Project File - Name="coop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=coop - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "coop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "coop.mak" CFG="coop - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "coop - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "coop - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "coop - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "COOP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "." /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "COOP_EXPORTS" /D "_CRT_SECURE_NO_DEPRECATE" /D "__OSIRIS_IMPORT_H_" /FAcs /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ..\..\lib\dmfc.lib kernel32.lib user32.lib gdi32.lib winmm.lib imagehlp.lib ws2_32.lib /nologo /dll /map /debug /machine:I386 /out:"Release/co-op.dll"

!ELSEIF  "$(CFG)" == "coop - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "COOP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "." /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "COOP_EXPORTS" /D "_CRT_SECURE_NO_DEPRECATE" /D "__OSIRIS_IMPORT_H_" /FAcs /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\lib\dmfc.lib kernel32.lib user32.lib gdi32.lib winmm.lib imagehlp.lib ws2_32.lib /nologo /dll /map /debug /machine:I386 /out:"Debug/co-op.d3m" /pdbtype:sept
# Begin Custom Build
TargetPath=.\Debug\co-op.d3m
TargetName=co-op
InputPath=.\Debug\co-op.d3m
SOURCE="$(InputPath)"

"D:\Descent3\netgames\$(TargetName).d3m" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "$(TargetPath)" "D:\Descent3\netgames\$(TargetName).d3m"

# End Custom Build

!ENDIF 

# Begin Target

# Name "coop - Win32 Release"
# Name "coop - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\chat.cpp
# End Source File
# Begin Source File

SOURCE=.\cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\cmd_show.cpp
# End Source File
# Begin Source File

SOURCE=.\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\control.cpp
# End Source File
# Begin Source File

SOURCE=.\coop.cpp
# End Source File
# Begin Source File

SOURCE=.\coop_mod.cpp
# End Source File
# Begin Source File

SOURCE=.\coop_mod_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\dallas.cpp
# End Source File
# Begin Source File

SOURCE=.\drop.cpp
# End Source File
# Begin Source File

SOURCE=.\error.cpp
# End Source File
# Begin Source File

SOURCE=.\exceptions.cpp
# End Source File
# Begin Source File

SOURCE=.\game_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\geoip.cpp
# End Source File
# Begin Source File

SOURCE=.\hooks.cpp
# End Source File
# Begin Source File

SOURCE=.\image.cpp
# End Source File
# Begin Source File

SOURCE=.\inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\log_file.cpp
# End Source File
# Begin Source File

SOURCE=.\missions.cpp
# End Source File
# Begin Source File

SOURCE=.\nicks.cpp
# End Source File
# Begin Source File

SOURCE=.\packet.cpp
# End Source File
# Begin Source File

SOURCE=.\patches.cpp
# End Source File
# Begin Source File

SOURCE=.\powerups.cpp
# End Source File
# Begin Source File

SOURCE=.\rooms.cpp
# End Source File
# Begin Source File

SOURCE=.\screen.cpp
# End Source File
# Begin Source File

SOURCE=.\script.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\chat.h
# End Source File
# Begin Source File

SOURCE=.\cmd.h
# End Source File
# Begin Source File

SOURCE=.\cmd_show.h
# End Source File
# Begin Source File

SOURCE=.\commands.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\control.h
# End Source File
# Begin Source File

SOURCE=.\coop.h
# End Source File
# Begin Source File

SOURCE=.\coop_mod.h
# End Source File
# Begin Source File

SOURCE=.\coop_mod_cmd.h
# End Source File
# Begin Source File

SOURCE=.\coopstr.h
# End Source File
# Begin Source File

SOURCE=.\dallas.h
# End Source File
# Begin Source File

SOURCE=.\drop.h
# End Source File
# Begin Source File

SOURCE=.\error.h
# End Source File
# Begin Source File

SOURCE=.\exceptions.h
# End Source File
# Begin Source File

SOURCE=.\game_utils.h
# End Source File
# Begin Source File

SOURCE=.\geoip.h
# End Source File
# Begin Source File

SOURCE=.\hooks.h
# End Source File
# Begin Source File

SOURCE=.\image.h
# End Source File
# Begin Source File

SOURCE=.\inventory.h
# End Source File
# Begin Source File

SOURCE=.\log_file.h
# End Source File
# Begin Source File

SOURCE=.\macros.h
# End Source File
# Begin Source File

SOURCE=.\missions.h
# End Source File
# Begin Source File

SOURCE=.\nicks.h
# End Source File
# Begin Source File

SOURCE=.\objects.h
# End Source File
# Begin Source File

SOURCE=.\packet.h
# End Source File
# Begin Source File

SOURCE=.\patches.h
# End Source File
# Begin Source File

SOURCE=.\powerups.h
# End Source File
# Begin Source File

SOURCE=.\rooms.h
# End Source File
# Begin Source File

SOURCE=.\screen.h
# End Source File
# Begin Source File

SOURCE=.\script.h
# End Source File
# Begin Source File

SOURCE=.\string2.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
