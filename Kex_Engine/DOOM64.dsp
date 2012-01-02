# Microsoft Developer Studio Project File - Name="DOOM64" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DOOM64 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DOOM64.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DOOM64.mak" CFG="DOOM64 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DOOM64 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DOOM64 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DOOM64 - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sdl.lib sdlmain.lib SDL_net.lib opengl32.lib glu32.lib libpng13.lib libfluidsynth.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBCMT" /out:"E:\sources\Kex\DOOM64.EXE"
# Begin Special Build Tool
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=Generate Revision String
PostBuild_Cmds="C:\Program Files\TortoiseSVN\bin\SubWCRev.exe" $(WkspDir) $(WkspDir)\revconfig.txt $(WkspDir)\version.h
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DOOM64 - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sdl.lib sdlmain.lib SDL_net.lib opengl32.lib glu32.lib libpng13.lib libfluidsynth.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"LIBCMT" /out:"E:\sources\Kex\DOOM64.EXE" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "DOOM64 - Win32 Release"
# Name "DOOM64 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "AM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\am_draw.c
# End Source File
# Begin Source File

SOURCE=.\am_map.c
# End Source File
# End Group
# Begin Group "D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\d_devstat.c
# End Source File
# Begin Source File

SOURCE=.\d_main.c
# End Source File
# Begin Source File

SOURCE=.\d_net.c
# End Source File
# End Group
# Begin Group "F"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\f_finale.c
# End Source File
# End Group
# Begin Group "G"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\g_actions.c
# End Source File
# Begin Source File

SOURCE=.\g_cmds.c
# End Source File
# Begin Source File

SOURCE=.\g_game.c
# End Source File
# Begin Source File

SOURCE=.\g_settings.c
# End Source File
# End Group
# Begin Group "I"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\i_audio.c
# End Source File
# Begin Source File

SOURCE=.\i_cpu.c
# End Source File
# Begin Source File

SOURCE=.\i_exception.c
# End Source File
# Begin Source File

SOURCE=.\i_launcher.c
# End Source File
# Begin Source File

SOURCE=.\i_main.c
# End Source File
# Begin Source File

SOURCE=.\i_opndir.c
# End Source File
# Begin Source File

SOURCE=.\i_png.c
# End Source File
# Begin Source File

SOURCE=.\i_system.c
# End Source File
# Begin Source File

SOURCE=.\i_xinput.c
# End Source File
# End Group
# Begin Group "M"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\m_cheat.c
# End Source File
# Begin Source File

SOURCE=.\m_fixed.c
# End Source File
# Begin Source File

SOURCE=.\m_keys.c
# End Source File
# Begin Source File

SOURCE=.\m_menu.c
# End Source File
# Begin Source File

SOURCE=.\m_misc.c
# End Source File
# Begin Source File

SOURCE=.\m_password.c
# End Source File
# Begin Source File

SOURCE=.\m_random.c
# End Source File
# Begin Source File

SOURCE=.\m_shift.c
# End Source File
# End Group
# Begin Group "P"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\p_ceilng.c
# End Source File
# Begin Source File

SOURCE=.\p_doors.c
# End Source File
# Begin Source File

SOURCE=.\p_enemy.c
# End Source File
# Begin Source File

SOURCE=.\p_floor.c
# End Source File
# Begin Source File

SOURCE=.\p_inter.c
# End Source File
# Begin Source File

SOURCE=.\p_lights.c
# End Source File
# Begin Source File

SOURCE=.\p_macros.c
# End Source File
# Begin Source File

SOURCE=.\p_map.c
# End Source File
# Begin Source File

SOURCE=.\p_maputl.c
# End Source File
# Begin Source File

SOURCE=.\p_mobj.c
# End Source File
# Begin Source File

SOURCE=.\p_plats.c
# End Source File
# Begin Source File

SOURCE=.\p_pspr.c
# End Source File
# Begin Source File

SOURCE=.\p_saveg.c
# End Source File
# Begin Source File

SOURCE=.\p_setup.c
# End Source File
# Begin Source File

SOURCE=.\p_sight.c
# End Source File
# Begin Source File

SOURCE=.\p_spec.c
# End Source File
# Begin Source File

SOURCE=.\p_switch.c
# End Source File
# Begin Source File

SOURCE=.\p_telept.c
# End Source File
# Begin Source File

SOURCE=.\p_tick.c
# End Source File
# Begin Source File

SOURCE=.\p_user.c
# End Source File
# End Group
# Begin Group "R"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\r_bsp.c
# End Source File
# Begin Source File

SOURCE=.\r_clipper.c
# End Source File
# Begin Source File

SOURCE=.\r_gl.c
# End Source File
# Begin Source File

SOURCE=.\r_glExt.c
# End Source File
# Begin Source File

SOURCE=.\r_lights.c
# End Source File
# Begin Source File

SOURCE=.\r_main.c
# End Source File
# Begin Source File

SOURCE=.\r_sky.c
# End Source File
# Begin Source File

SOURCE=.\r_texture.c
# End Source File
# Begin Source File

SOURCE=.\r_things.c
# End Source File
# Begin Source File

SOURCE=.\r_vertices.c
# End Source File
# Begin Source File

SOURCE=.\r_wipe.c
# End Source File
# End Group
# Begin Group "S"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\s_sound.c
# End Source File
# End Group
# Begin Group "W"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\w_file.c
# End Source File
# Begin Source File

SOURCE=.\w_merge.c
# End Source File
# Begin Source File

SOURCE=.\w_wad.c
# End Source File
# End Group
# Begin Group "Z"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\z_zone.c
# End Source File
# End Group
# Begin Group "WI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wi_stuff.c
# End Source File
# End Group
# Begin Group "V"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\v_sdl.c
# End Source File
# End Group
# Begin Group "ST"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\st_stuff.c
# End Source File
# End Group
# Begin Group "CON"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\con_console.c
# End Source File
# Begin Source File

SOURCE=.\con_cvar.c
# End Source File
# End Group
# Begin Group "NET"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_client.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_common.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_dedicated.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_io.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_loop.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_packet.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_query.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_sdl.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_server.c
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_structrw.c
# End Source File
# End Group
# Begin Group "IN"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\in_stuff.c
# End Source File
# End Group
# Begin Group "SC"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sc_main.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\dgl.c
# End Source File
# Begin Source File

SOURCE=.\info.c
# End Source File
# Begin Source File

SOURCE=.\Ext\md5.c
# End Source File
# Begin Source File

SOURCE=.\psnprntf.c
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\Ext\SDL_win32_main.c
# End Source File
# Begin Source File

SOURCE=.\tables.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "AM_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\am_draw.h
# End Source File
# Begin Source File

SOURCE=.\am_map.h
# End Source File
# End Group
# Begin Group "D_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\d_devstat.h
# End Source File
# Begin Source File

SOURCE=.\d_englsh.h
# End Source File
# Begin Source File

SOURCE=.\d_event.h
# End Source File
# Begin Source File

SOURCE=.\d_keywds.h
# End Source File
# Begin Source File

SOURCE=.\d_main.h
# End Source File
# Begin Source File

SOURCE=.\d_net.h
# End Source File
# Begin Source File

SOURCE=.\d_player.h
# End Source File
# Begin Source File

SOURCE=.\d_think.h
# End Source File
# Begin Source File

SOURCE=.\d_ticcmd.h
# End Source File
# End Group
# Begin Group "F_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\f_finale.h
# End Source File
# End Group
# Begin Group "G_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\g_actions.h
# End Source File
# Begin Source File

SOURCE=.\g_controls.h
# End Source File
# Begin Source File

SOURCE=.\g_game.h
# End Source File
# Begin Source File

SOURCE=.\g_local.h
# End Source File
# Begin Source File

SOURCE=.\g_settings.h
# End Source File
# End Group
# Begin Group "I_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\i_audio.h
# End Source File
# Begin Source File

SOURCE=.\i_launcher.h
# End Source File
# Begin Source File

SOURCE=.\i_opndir.h
# End Source File
# Begin Source File

SOURCE=.\i_png.h
# End Source File
# Begin Source File

SOURCE=.\i_swap.h
# End Source File
# Begin Source File

SOURCE=.\i_system.h
# End Source File
# Begin Source File

SOURCE=.\i_xinput.h
# End Source File
# End Group
# Begin Group "M_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\m_cheat.h
# End Source File
# Begin Source File

SOURCE=.\m_fixed.h
# End Source File
# Begin Source File

SOURCE=.\m_keys.h
# End Source File
# Begin Source File

SOURCE=.\m_menu.h
# End Source File
# Begin Source File

SOURCE=.\m_misc.h
# End Source File
# Begin Source File

SOURCE=.\m_password.h
# End Source File
# Begin Source File

SOURCE=.\m_random.h
# End Source File
# Begin Source File

SOURCE=.\m_shift.h
# End Source File
# End Group
# Begin Group "P_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\p_inter.h
# End Source File
# Begin Source File

SOURCE=.\p_local.h
# End Source File
# Begin Source File

SOURCE=.\p_macros.h
# End Source File
# Begin Source File

SOURCE=.\p_mobj.h
# End Source File
# Begin Source File

SOURCE=.\p_pspr.h
# End Source File
# Begin Source File

SOURCE=.\p_saveg.h
# End Source File
# Begin Source File

SOURCE=.\p_setup.h
# End Source File
# Begin Source File

SOURCE=.\p_spec.h
# End Source File
# Begin Source File

SOURCE=.\p_tick.h
# End Source File
# End Group
# Begin Group "R_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\r_bsp.h
# End Source File
# Begin Source File

SOURCE=.\r_clipper.h
# End Source File
# Begin Source File

SOURCE=.\r_gl.h
# End Source File
# Begin Source File

SOURCE=.\r_glExt.h
# End Source File
# Begin Source File

SOURCE=.\r_lights.h
# End Source File
# Begin Source File

SOURCE=.\r_local.h
# End Source File
# Begin Source File

SOURCE=.\r_main.h
# End Source File
# Begin Source File

SOURCE=.\r_sky.h
# End Source File
# Begin Source File

SOURCE=.\r_texture.h
# End Source File
# Begin Source File

SOURCE=.\r_things.h
# End Source File
# Begin Source File

SOURCE=.\r_vertices.h
# End Source File
# Begin Source File

SOURCE=.\r_wipe.h
# End Source File
# End Group
# Begin Group "S_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\s_sound.h
# End Source File
# End Group
# Begin Group "W_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\w_file.h
# End Source File
# Begin Source File

SOURCE=.\w_merge.h
# End Source File
# Begin Source File

SOURCE=.\w_wad.h
# End Source File
# End Group
# Begin Group "Z_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\z_zone.h
# End Source File
# End Group
# Begin Group "WI_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wi_stuff.h
# End Source File
# End Group
# Begin Group "V_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\v_sdl.h
# End Source File
# End Group
# Begin Group "T_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\t_bsp.h
# End Source File
# End Group
# Begin Group "ST_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\st_stuff.h
# End Source File
# End Group
# Begin Group "CON_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\con_console.h
# End Source File
# Begin Source File

SOURCE=.\con_cvar.h
# End Source File
# End Group
# Begin Group "NET_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_client.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_common.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_dedicated.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_defs.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_io.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_loop.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_packet.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_query.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_sdl.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_server.h
# End Source File
# Begin Source File

SOURCE=.\Ext\ChocolateDoom\net_structrw.h
# End Source File
# End Group
# Begin Group "SC_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sc_main.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\defconfig.inc
# End Source File
# Begin Source File

SOURCE=.\dgl.h
# End Source File
# Begin Source File

SOURCE=.\doomdata.h
# End Source File
# Begin Source File

SOURCE=.\doomdef.h
# End Source File
# Begin Source File

SOURCE=.\doomstat.h
# End Source File
# Begin Source File

SOURCE=.\doomtype.h
# End Source File
# Begin Source File

SOURCE=.\info.h
# End Source File
# Begin Source File

SOURCE=.\Ext\md5.h
# End Source File
# Begin Source File

SOURCE=.\psnprntf.h
# End Source File
# Begin Source File

SOURCE=.\sounds.h
# End Source File
# Begin Source File

SOURCE=.\tables.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\doom64.ico
# End Source File
# End Group
# End Target
# End Project
