# Microsoft Developer Studio Project File - Name="VirtualT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=VirtualT - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "VirtualT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VirtualT.mak" CFG="VirtualT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VirtualT - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "VirtualT - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "VirtualT - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /Od /I "$(FLTKDIR)" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 $(FLTKDIR)/lib/fltk.lib $(FLTKDIR)/lib/fltkimages.lib wsock32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /libpath:"..\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes /nodefaultlib

!ELSEIF  "$(CFG)" == "VirtualT - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "$(FLTKDIR)" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(FLTKDIR)/lib/fltkd.lib $(FLTKDIR)/lib/fltkimagesd.lib wsock32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcd" /libpath:"..\lib"
# SUBTRACT LINK32 /pdb:none /incremental:no /debug /pdbtype:<none>

!ENDIF 

# Begin Target

# Name "VirtualT - Win32 Release"
# Name "VirtualT - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "asm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\a85parse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\assemble.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MString.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MStringArray.cpp
# End Source File
# Begin Source File

SOURCE=.\src\rpn_eqn.cpp
# End Source File
# Begin Source File

SOURCE=.\src\vtobj.cpp
# End Source File
# End Group
# Begin Group "ide"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Flu_DND.cpp
# End Source File
# Begin Source File

SOURCE=.\src\flu_pixmaps.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Flu_Tree_Browser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FluSimpleString.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ide.cpp
# End Source File
# Begin Source File

SOURCE=.\src\multiwin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\multiwin_icons.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\cpuregs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\disassemble.cpp
# End Source File
# Begin Source File

SOURCE=.\src\display.cpp
# End Source File
# Begin Source File

SOURCE=.\src\doins.c
# End Source File
# Begin Source File

SOURCE=.\src\file.cpp
# End Source File
# Begin Source File

SOURCE=.\src\genwrap.c
# End Source File
# Begin Source File

SOURCE=.\src\intelhex.c
# End Source File
# Begin Source File

SOURCE=.\src\io.c
# End Source File
# Begin Source File

SOURCE=.\src\m100emu.c
# End Source File
# Begin Source File

SOURCE=.\src\m100rom.c
# End Source File
# Begin Source File

SOURCE=.\src\m200rom.c
# End Source File
# Begin Source File

SOURCE=.\src\memedit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\memory.c
# End Source File
# Begin Source File

SOURCE=.\src\n8201rom.c
# End Source File
# Begin Source File

SOURCE=.\src\periph.cpp
# End Source File
# Begin Source File

SOURCE=.\src\romstrings.c
# End Source File
# Begin Source File

SOURCE=.\src\serial.c
# End Source File
# Begin Source File

SOURCE=.\src\setup.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "asm.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\a85parse.h
# End Source File
# Begin Source File

SOURCE=.\src\assemble.h
# End Source File
# Begin Source File

SOURCE=.\src\elf.h
# End Source File
# Begin Source File

SOURCE=.\src\MString.h
# End Source File
# Begin Source File

SOURCE=.\src\MStringArray.h
# End Source File
# Begin Source File

SOURCE=.\src\rpn_eqn.h
# End Source File
# Begin Source File

SOURCE=.\src\vtobj.h
# End Source File
# End Group
# Begin Group "ide.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\FLU\Flu_DND.h
# End Source File
# Begin Source File

SOURCE=.\src\FLU\Flu_Enumerations.h
# End Source File
# Begin Source File

SOURCE=.\src\FLU\flu_export.h
# End Source File
# Begin Source File

SOURCE=.\src\FLU\flu_pixmaps.h
# End Source File
# Begin Source File

SOURCE=.\src\FLU\Flu_Tree_Browser.h
# End Source File
# Begin Source File

SOURCE=.\src\FLU\FluSimpleString.h
# End Source File
# Begin Source File

SOURCE=.\src\ide.h
# End Source File
# Begin Source File

SOURCE=.\src\multiwin.h
# End Source File
# Begin Source File

SOURCE=.\src\multiwin_icons.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\cpu.h
# End Source File
# Begin Source File

SOURCE=.\src\cpuregs.h
# End Source File
# Begin Source File

SOURCE=.\src\disassemble.h
# End Source File
# Begin Source File

SOURCE=.\src\display.h
# End Source File
# Begin Source File

SOURCE=.\src\do_instruct.h
# End Source File
# Begin Source File

SOURCE=.\src\doins.h
# End Source File
# Begin Source File

SOURCE=.\src\gen_defs.h
# End Source File
# Begin Source File

SOURCE=.\src\genwrap.h
# End Source File
# Begin Source File

SOURCE=.\src\intelhex.h
# End Source File
# Begin Source File

SOURCE=.\src\io.h
# End Source File
# Begin Source File

SOURCE=.\src\m100emu.h
# End Source File
# Begin Source File

SOURCE=.\src\memedit.h
# End Source File
# Begin Source File

SOURCE=.\src\memory.h
# End Source File
# Begin Source File

SOURCE=.\src\periph.h
# End Source File
# Begin Source File

SOURCE=.\src\roms.h
# End Source File
# Begin Source File

SOURCE=.\src\romstrings.h
# End Source File
# Begin Source File

SOURCE=.\src\serial.h
# End Source File
# Begin Source File

SOURCE=.\src\setup.h
# End Source File
# Begin Source File

SOURCE=.\src\sound.h
# End Source File
# Begin Source File

SOURCE=.\src\VirtualT.h
# End Source File
# Begin Source File

SOURCE=.\src\wrapdll.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
