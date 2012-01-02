# Microsoft Developer Studio Generated NMAKE File, Based on DOOM64.dsp
!IF "$(CFG)" == ""
CFG=DOOM64 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DOOM64 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DOOM64 - Win32 Release" && "$(CFG)" != "DOOM64 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DOOM64 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "..\DOOM64.EXE" "$(OUTDIR)\DOOM64.bsc"


CLEAN :
	-@erase "$(INTDIR)\am_draw.obj"
	-@erase "$(INTDIR)\am_draw.sbr"
	-@erase "$(INTDIR)\am_map.obj"
	-@erase "$(INTDIR)\am_map.sbr"
	-@erase "$(INTDIR)\con_console.obj"
	-@erase "$(INTDIR)\con_console.sbr"
	-@erase "$(INTDIR)\con_cvar.obj"
	-@erase "$(INTDIR)\con_cvar.sbr"
	-@erase "$(INTDIR)\d_devstat.obj"
	-@erase "$(INTDIR)\d_devstat.sbr"
	-@erase "$(INTDIR)\d_main.obj"
	-@erase "$(INTDIR)\d_main.sbr"
	-@erase "$(INTDIR)\d_net.obj"
	-@erase "$(INTDIR)\d_net.sbr"
	-@erase "$(INTDIR)\dgl.obj"
	-@erase "$(INTDIR)\dgl.sbr"
	-@erase "$(INTDIR)\f_finale.obj"
	-@erase "$(INTDIR)\f_finale.sbr"
	-@erase "$(INTDIR)\g_actions.obj"
	-@erase "$(INTDIR)\g_actions.sbr"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_cmds.sbr"
	-@erase "$(INTDIR)\g_game.obj"
	-@erase "$(INTDIR)\g_game.sbr"
	-@erase "$(INTDIR)\g_settings.obj"
	-@erase "$(INTDIR)\g_settings.sbr"
	-@erase "$(INTDIR)\i_audio.obj"
	-@erase "$(INTDIR)\i_audio.sbr"
	-@erase "$(INTDIR)\i_cpu.obj"
	-@erase "$(INTDIR)\i_cpu.sbr"
	-@erase "$(INTDIR)\i_exception.obj"
	-@erase "$(INTDIR)\i_exception.sbr"
	-@erase "$(INTDIR)\i_launcher.obj"
	-@erase "$(INTDIR)\i_launcher.sbr"
	-@erase "$(INTDIR)\i_main.obj"
	-@erase "$(INTDIR)\i_main.sbr"
	-@erase "$(INTDIR)\i_opndir.obj"
	-@erase "$(INTDIR)\i_opndir.sbr"
	-@erase "$(INTDIR)\i_png.obj"
	-@erase "$(INTDIR)\i_png.sbr"
	-@erase "$(INTDIR)\i_system.obj"
	-@erase "$(INTDIR)\i_system.sbr"
	-@erase "$(INTDIR)\i_xinput.obj"
	-@erase "$(INTDIR)\i_xinput.sbr"
	-@erase "$(INTDIR)\in_stuff.obj"
	-@erase "$(INTDIR)\in_stuff.sbr"
	-@erase "$(INTDIR)\info.obj"
	-@erase "$(INTDIR)\info.sbr"
	-@erase "$(INTDIR)\m_cheat.obj"
	-@erase "$(INTDIR)\m_cheat.sbr"
	-@erase "$(INTDIR)\m_fixed.obj"
	-@erase "$(INTDIR)\m_fixed.sbr"
	-@erase "$(INTDIR)\m_keys.obj"
	-@erase "$(INTDIR)\m_keys.sbr"
	-@erase "$(INTDIR)\m_menu.obj"
	-@erase "$(INTDIR)\m_menu.sbr"
	-@erase "$(INTDIR)\m_misc.obj"
	-@erase "$(INTDIR)\m_misc.sbr"
	-@erase "$(INTDIR)\m_password.obj"
	-@erase "$(INTDIR)\m_password.sbr"
	-@erase "$(INTDIR)\m_random.obj"
	-@erase "$(INTDIR)\m_random.sbr"
	-@erase "$(INTDIR)\m_shift.obj"
	-@erase "$(INTDIR)\m_shift.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\net_client.obj"
	-@erase "$(INTDIR)\net_client.sbr"
	-@erase "$(INTDIR)\net_common.obj"
	-@erase "$(INTDIR)\net_common.sbr"
	-@erase "$(INTDIR)\net_dedicated.obj"
	-@erase "$(INTDIR)\net_dedicated.sbr"
	-@erase "$(INTDIR)\net_io.obj"
	-@erase "$(INTDIR)\net_io.sbr"
	-@erase "$(INTDIR)\net_loop.obj"
	-@erase "$(INTDIR)\net_loop.sbr"
	-@erase "$(INTDIR)\net_packet.obj"
	-@erase "$(INTDIR)\net_packet.sbr"
	-@erase "$(INTDIR)\net_query.obj"
	-@erase "$(INTDIR)\net_query.sbr"
	-@erase "$(INTDIR)\net_sdl.obj"
	-@erase "$(INTDIR)\net_sdl.sbr"
	-@erase "$(INTDIR)\net_server.obj"
	-@erase "$(INTDIR)\net_server.sbr"
	-@erase "$(INTDIR)\net_structrw.obj"
	-@erase "$(INTDIR)\net_structrw.sbr"
	-@erase "$(INTDIR)\p_ceilng.obj"
	-@erase "$(INTDIR)\p_ceilng.sbr"
	-@erase "$(INTDIR)\p_doors.obj"
	-@erase "$(INTDIR)\p_doors.sbr"
	-@erase "$(INTDIR)\p_enemy.obj"
	-@erase "$(INTDIR)\p_enemy.sbr"
	-@erase "$(INTDIR)\p_floor.obj"
	-@erase "$(INTDIR)\p_floor.sbr"
	-@erase "$(INTDIR)\p_inter.obj"
	-@erase "$(INTDIR)\p_inter.sbr"
	-@erase "$(INTDIR)\p_lights.obj"
	-@erase "$(INTDIR)\p_lights.sbr"
	-@erase "$(INTDIR)\p_macros.obj"
	-@erase "$(INTDIR)\p_macros.sbr"
	-@erase "$(INTDIR)\p_map.obj"
	-@erase "$(INTDIR)\p_map.sbr"
	-@erase "$(INTDIR)\p_maputl.obj"
	-@erase "$(INTDIR)\p_maputl.sbr"
	-@erase "$(INTDIR)\p_mobj.obj"
	-@erase "$(INTDIR)\p_mobj.sbr"
	-@erase "$(INTDIR)\p_plats.obj"
	-@erase "$(INTDIR)\p_plats.sbr"
	-@erase "$(INTDIR)\p_pspr.obj"
	-@erase "$(INTDIR)\p_pspr.sbr"
	-@erase "$(INTDIR)\p_saveg.obj"
	-@erase "$(INTDIR)\p_saveg.sbr"
	-@erase "$(INTDIR)\p_setup.obj"
	-@erase "$(INTDIR)\p_setup.sbr"
	-@erase "$(INTDIR)\p_sight.obj"
	-@erase "$(INTDIR)\p_sight.sbr"
	-@erase "$(INTDIR)\p_spec.obj"
	-@erase "$(INTDIR)\p_spec.sbr"
	-@erase "$(INTDIR)\p_switch.obj"
	-@erase "$(INTDIR)\p_switch.sbr"
	-@erase "$(INTDIR)\p_telept.obj"
	-@erase "$(INTDIR)\p_telept.sbr"
	-@erase "$(INTDIR)\p_tick.obj"
	-@erase "$(INTDIR)\p_tick.sbr"
	-@erase "$(INTDIR)\p_user.obj"
	-@erase "$(INTDIR)\p_user.sbr"
	-@erase "$(INTDIR)\psnprntf.obj"
	-@erase "$(INTDIR)\psnprntf.sbr"
	-@erase "$(INTDIR)\r_bsp.obj"
	-@erase "$(INTDIR)\r_bsp.sbr"
	-@erase "$(INTDIR)\r_clipper.obj"
	-@erase "$(INTDIR)\r_clipper.sbr"
	-@erase "$(INTDIR)\r_gl.obj"
	-@erase "$(INTDIR)\r_gl.sbr"
	-@erase "$(INTDIR)\r_glExt.obj"
	-@erase "$(INTDIR)\r_glExt.sbr"
	-@erase "$(INTDIR)\r_lights.obj"
	-@erase "$(INTDIR)\r_lights.sbr"
	-@erase "$(INTDIR)\r_main.obj"
	-@erase "$(INTDIR)\r_main.sbr"
	-@erase "$(INTDIR)\r_sky.obj"
	-@erase "$(INTDIR)\r_sky.sbr"
	-@erase "$(INTDIR)\r_texture.obj"
	-@erase "$(INTDIR)\r_texture.sbr"
	-@erase "$(INTDIR)\r_things.obj"
	-@erase "$(INTDIR)\r_things.sbr"
	-@erase "$(INTDIR)\r_vertices.obj"
	-@erase "$(INTDIR)\r_vertices.sbr"
	-@erase "$(INTDIR)\r_wipe.obj"
	-@erase "$(INTDIR)\r_wipe.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\s_sound.obj"
	-@erase "$(INTDIR)\s_sound.sbr"
	-@erase "$(INTDIR)\sc_main.obj"
	-@erase "$(INTDIR)\sc_main.sbr"
	-@erase "$(INTDIR)\SDL_win32_main.obj"
	-@erase "$(INTDIR)\SDL_win32_main.sbr"
	-@erase "$(INTDIR)\st_stuff.obj"
	-@erase "$(INTDIR)\st_stuff.sbr"
	-@erase "$(INTDIR)\tables.obj"
	-@erase "$(INTDIR)\tables.sbr"
	-@erase "$(INTDIR)\v_sdl.obj"
	-@erase "$(INTDIR)\v_sdl.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\w_file.obj"
	-@erase "$(INTDIR)\w_file.sbr"
	-@erase "$(INTDIR)\w_merge.obj"
	-@erase "$(INTDIR)\w_merge.sbr"
	-@erase "$(INTDIR)\w_wad.obj"
	-@erase "$(INTDIR)\w_wad.sbr"
	-@erase "$(INTDIR)\wi_stuff.obj"
	-@erase "$(INTDIR)\wi_stuff.sbr"
	-@erase "$(INTDIR)\z_zone.obj"
	-@erase "$(INTDIR)\z_zone.sbr"
	-@erase "$(OUTDIR)\DOOM64.bsc"
	-@erase "..\DOOM64.EXE"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\DOOM64.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DOOM64.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\am_draw.sbr" \
	"$(INTDIR)\am_map.sbr" \
	"$(INTDIR)\d_devstat.sbr" \
	"$(INTDIR)\d_main.sbr" \
	"$(INTDIR)\d_net.sbr" \
	"$(INTDIR)\f_finale.sbr" \
	"$(INTDIR)\g_actions.sbr" \
	"$(INTDIR)\g_cmds.sbr" \
	"$(INTDIR)\g_game.sbr" \
	"$(INTDIR)\g_settings.sbr" \
	"$(INTDIR)\i_audio.sbr" \
	"$(INTDIR)\i_cpu.sbr" \
	"$(INTDIR)\i_exception.sbr" \
	"$(INTDIR)\i_launcher.sbr" \
	"$(INTDIR)\i_main.sbr" \
	"$(INTDIR)\i_opndir.sbr" \
	"$(INTDIR)\i_png.sbr" \
	"$(INTDIR)\i_system.sbr" \
	"$(INTDIR)\i_xinput.sbr" \
	"$(INTDIR)\m_cheat.sbr" \
	"$(INTDIR)\m_fixed.sbr" \
	"$(INTDIR)\m_keys.sbr" \
	"$(INTDIR)\m_menu.sbr" \
	"$(INTDIR)\m_misc.sbr" \
	"$(INTDIR)\m_password.sbr" \
	"$(INTDIR)\m_random.sbr" \
	"$(INTDIR)\m_shift.sbr" \
	"$(INTDIR)\p_ceilng.sbr" \
	"$(INTDIR)\p_doors.sbr" \
	"$(INTDIR)\p_enemy.sbr" \
	"$(INTDIR)\p_floor.sbr" \
	"$(INTDIR)\p_inter.sbr" \
	"$(INTDIR)\p_lights.sbr" \
	"$(INTDIR)\p_macros.sbr" \
	"$(INTDIR)\p_map.sbr" \
	"$(INTDIR)\p_maputl.sbr" \
	"$(INTDIR)\p_mobj.sbr" \
	"$(INTDIR)\p_plats.sbr" \
	"$(INTDIR)\p_pspr.sbr" \
	"$(INTDIR)\p_saveg.sbr" \
	"$(INTDIR)\p_setup.sbr" \
	"$(INTDIR)\p_sight.sbr" \
	"$(INTDIR)\p_spec.sbr" \
	"$(INTDIR)\p_switch.sbr" \
	"$(INTDIR)\p_telept.sbr" \
	"$(INTDIR)\p_tick.sbr" \
	"$(INTDIR)\p_user.sbr" \
	"$(INTDIR)\r_bsp.sbr" \
	"$(INTDIR)\r_clipper.sbr" \
	"$(INTDIR)\r_gl.sbr" \
	"$(INTDIR)\r_glExt.sbr" \
	"$(INTDIR)\r_lights.sbr" \
	"$(INTDIR)\r_main.sbr" \
	"$(INTDIR)\r_sky.sbr" \
	"$(INTDIR)\r_texture.sbr" \
	"$(INTDIR)\r_things.sbr" \
	"$(INTDIR)\r_vertices.sbr" \
	"$(INTDIR)\r_wipe.sbr" \
	"$(INTDIR)\s_sound.sbr" \
	"$(INTDIR)\w_file.sbr" \
	"$(INTDIR)\w_merge.sbr" \
	"$(INTDIR)\w_wad.sbr" \
	"$(INTDIR)\z_zone.sbr" \
	"$(INTDIR)\wi_stuff.sbr" \
	"$(INTDIR)\v_sdl.sbr" \
	"$(INTDIR)\st_stuff.sbr" \
	"$(INTDIR)\con_console.sbr" \
	"$(INTDIR)\con_cvar.sbr" \
	"$(INTDIR)\net_client.sbr" \
	"$(INTDIR)\net_common.sbr" \
	"$(INTDIR)\net_dedicated.sbr" \
	"$(INTDIR)\net_io.sbr" \
	"$(INTDIR)\net_loop.sbr" \
	"$(INTDIR)\net_packet.sbr" \
	"$(INTDIR)\net_query.sbr" \
	"$(INTDIR)\net_sdl.sbr" \
	"$(INTDIR)\net_server.sbr" \
	"$(INTDIR)\net_structrw.sbr" \
	"$(INTDIR)\in_stuff.sbr" \
	"$(INTDIR)\sc_main.sbr" \
	"$(INTDIR)\dgl.sbr" \
	"$(INTDIR)\info.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\psnprntf.sbr" \
	"$(INTDIR)\SDL_win32_main.sbr" \
	"$(INTDIR)\tables.sbr"

"$(OUTDIR)\DOOM64.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sdl.lib sdlmain.lib SDL_net.lib opengl32.lib glu32.lib libpng13.lib libfluidsynth.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\DOOM64.pdb" /machine:I386 /nodefaultlib:"LIBCMT" /out:"E:\sources\Kex\DOOM64.EXE" 
LINK32_OBJS= \
	"$(INTDIR)\am_draw.obj" \
	"$(INTDIR)\am_map.obj" \
	"$(INTDIR)\d_devstat.obj" \
	"$(INTDIR)\d_main.obj" \
	"$(INTDIR)\d_net.obj" \
	"$(INTDIR)\f_finale.obj" \
	"$(INTDIR)\g_actions.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_game.obj" \
	"$(INTDIR)\g_settings.obj" \
	"$(INTDIR)\i_audio.obj" \
	"$(INTDIR)\i_cpu.obj" \
	"$(INTDIR)\i_exception.obj" \
	"$(INTDIR)\i_launcher.obj" \
	"$(INTDIR)\i_main.obj" \
	"$(INTDIR)\i_opndir.obj" \
	"$(INTDIR)\i_png.obj" \
	"$(INTDIR)\i_system.obj" \
	"$(INTDIR)\i_xinput.obj" \
	"$(INTDIR)\m_cheat.obj" \
	"$(INTDIR)\m_fixed.obj" \
	"$(INTDIR)\m_keys.obj" \
	"$(INTDIR)\m_menu.obj" \
	"$(INTDIR)\m_misc.obj" \
	"$(INTDIR)\m_password.obj" \
	"$(INTDIR)\m_random.obj" \
	"$(INTDIR)\m_shift.obj" \
	"$(INTDIR)\p_ceilng.obj" \
	"$(INTDIR)\p_doors.obj" \
	"$(INTDIR)\p_enemy.obj" \
	"$(INTDIR)\p_floor.obj" \
	"$(INTDIR)\p_inter.obj" \
	"$(INTDIR)\p_lights.obj" \
	"$(INTDIR)\p_macros.obj" \
	"$(INTDIR)\p_map.obj" \
	"$(INTDIR)\p_maputl.obj" \
	"$(INTDIR)\p_mobj.obj" \
	"$(INTDIR)\p_plats.obj" \
	"$(INTDIR)\p_pspr.obj" \
	"$(INTDIR)\p_saveg.obj" \
	"$(INTDIR)\p_setup.obj" \
	"$(INTDIR)\p_sight.obj" \
	"$(INTDIR)\p_spec.obj" \
	"$(INTDIR)\p_switch.obj" \
	"$(INTDIR)\p_telept.obj" \
	"$(INTDIR)\p_tick.obj" \
	"$(INTDIR)\p_user.obj" \
	"$(INTDIR)\r_bsp.obj" \
	"$(INTDIR)\r_clipper.obj" \
	"$(INTDIR)\r_gl.obj" \
	"$(INTDIR)\r_glExt.obj" \
	"$(INTDIR)\r_lights.obj" \
	"$(INTDIR)\r_main.obj" \
	"$(INTDIR)\r_sky.obj" \
	"$(INTDIR)\r_texture.obj" \
	"$(INTDIR)\r_things.obj" \
	"$(INTDIR)\r_vertices.obj" \
	"$(INTDIR)\r_wipe.obj" \
	"$(INTDIR)\s_sound.obj" \
	"$(INTDIR)\w_file.obj" \
	"$(INTDIR)\w_merge.obj" \
	"$(INTDIR)\w_wad.obj" \
	"$(INTDIR)\z_zone.obj" \
	"$(INTDIR)\wi_stuff.obj" \
	"$(INTDIR)\v_sdl.obj" \
	"$(INTDIR)\st_stuff.obj" \
	"$(INTDIR)\con_console.obj" \
	"$(INTDIR)\con_cvar.obj" \
	"$(INTDIR)\net_client.obj" \
	"$(INTDIR)\net_common.obj" \
	"$(INTDIR)\net_dedicated.obj" \
	"$(INTDIR)\net_io.obj" \
	"$(INTDIR)\net_loop.obj" \
	"$(INTDIR)\net_packet.obj" \
	"$(INTDIR)\net_query.obj" \
	"$(INTDIR)\net_sdl.obj" \
	"$(INTDIR)\net_server.obj" \
	"$(INTDIR)\net_structrw.obj" \
	"$(INTDIR)\in_stuff.obj" \
	"$(INTDIR)\sc_main.obj" \
	"$(INTDIR)\dgl.obj" \
	"$(INTDIR)\info.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\psnprntf.obj" \
	"$(INTDIR)\SDL_win32_main.obj" \
	"$(INTDIR)\tables.obj" \
	"$(INTDIR)\resource.res"

"..\DOOM64.EXE" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=Generate Revision String
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "..\DOOM64.EXE" "$(OUTDIR)\DOOM64.bsc"
   "C:\Program Files\TortoiseSVN\bin\SubWCRev.exe" E:\sources\Kex\SRC E:\sources\Kex\SRC\revconfig.txt E:\sources\Kex\SRC\version.h
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "DOOM64 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\DOOM64.EXE" "$(OUTDIR)\DOOM64.bsc"


CLEAN :
	-@erase "$(INTDIR)\am_draw.obj"
	-@erase "$(INTDIR)\am_draw.sbr"
	-@erase "$(INTDIR)\am_map.obj"
	-@erase "$(INTDIR)\am_map.sbr"
	-@erase "$(INTDIR)\con_console.obj"
	-@erase "$(INTDIR)\con_console.sbr"
	-@erase "$(INTDIR)\con_cvar.obj"
	-@erase "$(INTDIR)\con_cvar.sbr"
	-@erase "$(INTDIR)\d_devstat.obj"
	-@erase "$(INTDIR)\d_devstat.sbr"
	-@erase "$(INTDIR)\d_main.obj"
	-@erase "$(INTDIR)\d_main.sbr"
	-@erase "$(INTDIR)\d_net.obj"
	-@erase "$(INTDIR)\d_net.sbr"
	-@erase "$(INTDIR)\dgl.obj"
	-@erase "$(INTDIR)\dgl.sbr"
	-@erase "$(INTDIR)\f_finale.obj"
	-@erase "$(INTDIR)\f_finale.sbr"
	-@erase "$(INTDIR)\g_actions.obj"
	-@erase "$(INTDIR)\g_actions.sbr"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_cmds.sbr"
	-@erase "$(INTDIR)\g_game.obj"
	-@erase "$(INTDIR)\g_game.sbr"
	-@erase "$(INTDIR)\g_settings.obj"
	-@erase "$(INTDIR)\g_settings.sbr"
	-@erase "$(INTDIR)\i_audio.obj"
	-@erase "$(INTDIR)\i_audio.sbr"
	-@erase "$(INTDIR)\i_cpu.obj"
	-@erase "$(INTDIR)\i_cpu.sbr"
	-@erase "$(INTDIR)\i_exception.obj"
	-@erase "$(INTDIR)\i_exception.sbr"
	-@erase "$(INTDIR)\i_launcher.obj"
	-@erase "$(INTDIR)\i_launcher.sbr"
	-@erase "$(INTDIR)\i_main.obj"
	-@erase "$(INTDIR)\i_main.sbr"
	-@erase "$(INTDIR)\i_opndir.obj"
	-@erase "$(INTDIR)\i_opndir.sbr"
	-@erase "$(INTDIR)\i_png.obj"
	-@erase "$(INTDIR)\i_png.sbr"
	-@erase "$(INTDIR)\i_system.obj"
	-@erase "$(INTDIR)\i_system.sbr"
	-@erase "$(INTDIR)\i_xinput.obj"
	-@erase "$(INTDIR)\i_xinput.sbr"
	-@erase "$(INTDIR)\in_stuff.obj"
	-@erase "$(INTDIR)\in_stuff.sbr"
	-@erase "$(INTDIR)\info.obj"
	-@erase "$(INTDIR)\info.sbr"
	-@erase "$(INTDIR)\m_cheat.obj"
	-@erase "$(INTDIR)\m_cheat.sbr"
	-@erase "$(INTDIR)\m_fixed.obj"
	-@erase "$(INTDIR)\m_fixed.sbr"
	-@erase "$(INTDIR)\m_keys.obj"
	-@erase "$(INTDIR)\m_keys.sbr"
	-@erase "$(INTDIR)\m_menu.obj"
	-@erase "$(INTDIR)\m_menu.sbr"
	-@erase "$(INTDIR)\m_misc.obj"
	-@erase "$(INTDIR)\m_misc.sbr"
	-@erase "$(INTDIR)\m_password.obj"
	-@erase "$(INTDIR)\m_password.sbr"
	-@erase "$(INTDIR)\m_random.obj"
	-@erase "$(INTDIR)\m_random.sbr"
	-@erase "$(INTDIR)\m_shift.obj"
	-@erase "$(INTDIR)\m_shift.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\net_client.obj"
	-@erase "$(INTDIR)\net_client.sbr"
	-@erase "$(INTDIR)\net_common.obj"
	-@erase "$(INTDIR)\net_common.sbr"
	-@erase "$(INTDIR)\net_dedicated.obj"
	-@erase "$(INTDIR)\net_dedicated.sbr"
	-@erase "$(INTDIR)\net_io.obj"
	-@erase "$(INTDIR)\net_io.sbr"
	-@erase "$(INTDIR)\net_loop.obj"
	-@erase "$(INTDIR)\net_loop.sbr"
	-@erase "$(INTDIR)\net_packet.obj"
	-@erase "$(INTDIR)\net_packet.sbr"
	-@erase "$(INTDIR)\net_query.obj"
	-@erase "$(INTDIR)\net_query.sbr"
	-@erase "$(INTDIR)\net_sdl.obj"
	-@erase "$(INTDIR)\net_sdl.sbr"
	-@erase "$(INTDIR)\net_server.obj"
	-@erase "$(INTDIR)\net_server.sbr"
	-@erase "$(INTDIR)\net_structrw.obj"
	-@erase "$(INTDIR)\net_structrw.sbr"
	-@erase "$(INTDIR)\p_ceilng.obj"
	-@erase "$(INTDIR)\p_ceilng.sbr"
	-@erase "$(INTDIR)\p_doors.obj"
	-@erase "$(INTDIR)\p_doors.sbr"
	-@erase "$(INTDIR)\p_enemy.obj"
	-@erase "$(INTDIR)\p_enemy.sbr"
	-@erase "$(INTDIR)\p_floor.obj"
	-@erase "$(INTDIR)\p_floor.sbr"
	-@erase "$(INTDIR)\p_inter.obj"
	-@erase "$(INTDIR)\p_inter.sbr"
	-@erase "$(INTDIR)\p_lights.obj"
	-@erase "$(INTDIR)\p_lights.sbr"
	-@erase "$(INTDIR)\p_macros.obj"
	-@erase "$(INTDIR)\p_macros.sbr"
	-@erase "$(INTDIR)\p_map.obj"
	-@erase "$(INTDIR)\p_map.sbr"
	-@erase "$(INTDIR)\p_maputl.obj"
	-@erase "$(INTDIR)\p_maputl.sbr"
	-@erase "$(INTDIR)\p_mobj.obj"
	-@erase "$(INTDIR)\p_mobj.sbr"
	-@erase "$(INTDIR)\p_plats.obj"
	-@erase "$(INTDIR)\p_plats.sbr"
	-@erase "$(INTDIR)\p_pspr.obj"
	-@erase "$(INTDIR)\p_pspr.sbr"
	-@erase "$(INTDIR)\p_saveg.obj"
	-@erase "$(INTDIR)\p_saveg.sbr"
	-@erase "$(INTDIR)\p_setup.obj"
	-@erase "$(INTDIR)\p_setup.sbr"
	-@erase "$(INTDIR)\p_sight.obj"
	-@erase "$(INTDIR)\p_sight.sbr"
	-@erase "$(INTDIR)\p_spec.obj"
	-@erase "$(INTDIR)\p_spec.sbr"
	-@erase "$(INTDIR)\p_switch.obj"
	-@erase "$(INTDIR)\p_switch.sbr"
	-@erase "$(INTDIR)\p_telept.obj"
	-@erase "$(INTDIR)\p_telept.sbr"
	-@erase "$(INTDIR)\p_tick.obj"
	-@erase "$(INTDIR)\p_tick.sbr"
	-@erase "$(INTDIR)\p_user.obj"
	-@erase "$(INTDIR)\p_user.sbr"
	-@erase "$(INTDIR)\psnprntf.obj"
	-@erase "$(INTDIR)\psnprntf.sbr"
	-@erase "$(INTDIR)\r_bsp.obj"
	-@erase "$(INTDIR)\r_bsp.sbr"
	-@erase "$(INTDIR)\r_clipper.obj"
	-@erase "$(INTDIR)\r_clipper.sbr"
	-@erase "$(INTDIR)\r_gl.obj"
	-@erase "$(INTDIR)\r_gl.sbr"
	-@erase "$(INTDIR)\r_glExt.obj"
	-@erase "$(INTDIR)\r_glExt.sbr"
	-@erase "$(INTDIR)\r_lights.obj"
	-@erase "$(INTDIR)\r_lights.sbr"
	-@erase "$(INTDIR)\r_main.obj"
	-@erase "$(INTDIR)\r_main.sbr"
	-@erase "$(INTDIR)\r_sky.obj"
	-@erase "$(INTDIR)\r_sky.sbr"
	-@erase "$(INTDIR)\r_texture.obj"
	-@erase "$(INTDIR)\r_texture.sbr"
	-@erase "$(INTDIR)\r_things.obj"
	-@erase "$(INTDIR)\r_things.sbr"
	-@erase "$(INTDIR)\r_vertices.obj"
	-@erase "$(INTDIR)\r_vertices.sbr"
	-@erase "$(INTDIR)\r_wipe.obj"
	-@erase "$(INTDIR)\r_wipe.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\s_sound.obj"
	-@erase "$(INTDIR)\s_sound.sbr"
	-@erase "$(INTDIR)\sc_main.obj"
	-@erase "$(INTDIR)\sc_main.sbr"
	-@erase "$(INTDIR)\SDL_win32_main.obj"
	-@erase "$(INTDIR)\SDL_win32_main.sbr"
	-@erase "$(INTDIR)\st_stuff.obj"
	-@erase "$(INTDIR)\st_stuff.sbr"
	-@erase "$(INTDIR)\tables.obj"
	-@erase "$(INTDIR)\tables.sbr"
	-@erase "$(INTDIR)\v_sdl.obj"
	-@erase "$(INTDIR)\v_sdl.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\w_file.obj"
	-@erase "$(INTDIR)\w_file.sbr"
	-@erase "$(INTDIR)\w_merge.obj"
	-@erase "$(INTDIR)\w_merge.sbr"
	-@erase "$(INTDIR)\w_wad.obj"
	-@erase "$(INTDIR)\w_wad.sbr"
	-@erase "$(INTDIR)\wi_stuff.obj"
	-@erase "$(INTDIR)\wi_stuff.sbr"
	-@erase "$(INTDIR)\z_zone.obj"
	-@erase "$(INTDIR)\z_zone.sbr"
	-@erase "$(OUTDIR)\DOOM64.bsc"
	-@erase "$(OUTDIR)\DOOM64.map"
	-@erase "$(OUTDIR)\DOOM64.pdb"
	-@erase "..\DOOM64.EXE"
	-@erase "..\DOOM64.ILK"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\DOOM64.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DOOM64.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\am_draw.sbr" \
	"$(INTDIR)\am_map.sbr" \
	"$(INTDIR)\d_devstat.sbr" \
	"$(INTDIR)\d_main.sbr" \
	"$(INTDIR)\d_net.sbr" \
	"$(INTDIR)\f_finale.sbr" \
	"$(INTDIR)\g_actions.sbr" \
	"$(INTDIR)\g_cmds.sbr" \
	"$(INTDIR)\g_game.sbr" \
	"$(INTDIR)\g_settings.sbr" \
	"$(INTDIR)\i_audio.sbr" \
	"$(INTDIR)\i_cpu.sbr" \
	"$(INTDIR)\i_exception.sbr" \
	"$(INTDIR)\i_launcher.sbr" \
	"$(INTDIR)\i_main.sbr" \
	"$(INTDIR)\i_opndir.sbr" \
	"$(INTDIR)\i_png.sbr" \
	"$(INTDIR)\i_system.sbr" \
	"$(INTDIR)\i_xinput.sbr" \
	"$(INTDIR)\m_cheat.sbr" \
	"$(INTDIR)\m_fixed.sbr" \
	"$(INTDIR)\m_keys.sbr" \
	"$(INTDIR)\m_menu.sbr" \
	"$(INTDIR)\m_misc.sbr" \
	"$(INTDIR)\m_password.sbr" \
	"$(INTDIR)\m_random.sbr" \
	"$(INTDIR)\m_shift.sbr" \
	"$(INTDIR)\p_ceilng.sbr" \
	"$(INTDIR)\p_doors.sbr" \
	"$(INTDIR)\p_enemy.sbr" \
	"$(INTDIR)\p_floor.sbr" \
	"$(INTDIR)\p_inter.sbr" \
	"$(INTDIR)\p_lights.sbr" \
	"$(INTDIR)\p_macros.sbr" \
	"$(INTDIR)\p_map.sbr" \
	"$(INTDIR)\p_maputl.sbr" \
	"$(INTDIR)\p_mobj.sbr" \
	"$(INTDIR)\p_plats.sbr" \
	"$(INTDIR)\p_pspr.sbr" \
	"$(INTDIR)\p_saveg.sbr" \
	"$(INTDIR)\p_setup.sbr" \
	"$(INTDIR)\p_sight.sbr" \
	"$(INTDIR)\p_spec.sbr" \
	"$(INTDIR)\p_switch.sbr" \
	"$(INTDIR)\p_telept.sbr" \
	"$(INTDIR)\p_tick.sbr" \
	"$(INTDIR)\p_user.sbr" \
	"$(INTDIR)\r_bsp.sbr" \
	"$(INTDIR)\r_clipper.sbr" \
	"$(INTDIR)\r_gl.sbr" \
	"$(INTDIR)\r_glExt.sbr" \
	"$(INTDIR)\r_lights.sbr" \
	"$(INTDIR)\r_main.sbr" \
	"$(INTDIR)\r_sky.sbr" \
	"$(INTDIR)\r_texture.sbr" \
	"$(INTDIR)\r_things.sbr" \
	"$(INTDIR)\r_vertices.sbr" \
	"$(INTDIR)\r_wipe.sbr" \
	"$(INTDIR)\s_sound.sbr" \
	"$(INTDIR)\w_file.sbr" \
	"$(INTDIR)\w_merge.sbr" \
	"$(INTDIR)\w_wad.sbr" \
	"$(INTDIR)\z_zone.sbr" \
	"$(INTDIR)\wi_stuff.sbr" \
	"$(INTDIR)\v_sdl.sbr" \
	"$(INTDIR)\st_stuff.sbr" \
	"$(INTDIR)\con_console.sbr" \
	"$(INTDIR)\con_cvar.sbr" \
	"$(INTDIR)\net_client.sbr" \
	"$(INTDIR)\net_common.sbr" \
	"$(INTDIR)\net_dedicated.sbr" \
	"$(INTDIR)\net_io.sbr" \
	"$(INTDIR)\net_loop.sbr" \
	"$(INTDIR)\net_packet.sbr" \
	"$(INTDIR)\net_query.sbr" \
	"$(INTDIR)\net_sdl.sbr" \
	"$(INTDIR)\net_server.sbr" \
	"$(INTDIR)\net_structrw.sbr" \
	"$(INTDIR)\in_stuff.sbr" \
	"$(INTDIR)\sc_main.sbr" \
	"$(INTDIR)\dgl.sbr" \
	"$(INTDIR)\info.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\psnprntf.sbr" \
	"$(INTDIR)\SDL_win32_main.sbr" \
	"$(INTDIR)\tables.sbr"

"$(OUTDIR)\DOOM64.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sdl.lib sdlmain.lib SDL_net.lib opengl32.lib glu32.lib libpng13.lib libfluidsynth.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\DOOM64.pdb" /map:"$(INTDIR)\DOOM64.map" /debug /machine:I386 /nodefaultlib:"LIBCMT" /out:"E:\sources\Kex\DOOM64.EXE" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\am_draw.obj" \
	"$(INTDIR)\am_map.obj" \
	"$(INTDIR)\d_devstat.obj" \
	"$(INTDIR)\d_main.obj" \
	"$(INTDIR)\d_net.obj" \
	"$(INTDIR)\f_finale.obj" \
	"$(INTDIR)\g_actions.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_game.obj" \
	"$(INTDIR)\g_settings.obj" \
	"$(INTDIR)\i_audio.obj" \
	"$(INTDIR)\i_cpu.obj" \
	"$(INTDIR)\i_exception.obj" \
	"$(INTDIR)\i_launcher.obj" \
	"$(INTDIR)\i_main.obj" \
	"$(INTDIR)\i_opndir.obj" \
	"$(INTDIR)\i_png.obj" \
	"$(INTDIR)\i_system.obj" \
	"$(INTDIR)\i_xinput.obj" \
	"$(INTDIR)\m_cheat.obj" \
	"$(INTDIR)\m_fixed.obj" \
	"$(INTDIR)\m_keys.obj" \
	"$(INTDIR)\m_menu.obj" \
	"$(INTDIR)\m_misc.obj" \
	"$(INTDIR)\m_password.obj" \
	"$(INTDIR)\m_random.obj" \
	"$(INTDIR)\m_shift.obj" \
	"$(INTDIR)\p_ceilng.obj" \
	"$(INTDIR)\p_doors.obj" \
	"$(INTDIR)\p_enemy.obj" \
	"$(INTDIR)\p_floor.obj" \
	"$(INTDIR)\p_inter.obj" \
	"$(INTDIR)\p_lights.obj" \
	"$(INTDIR)\p_macros.obj" \
	"$(INTDIR)\p_map.obj" \
	"$(INTDIR)\p_maputl.obj" \
	"$(INTDIR)\p_mobj.obj" \
	"$(INTDIR)\p_plats.obj" \
	"$(INTDIR)\p_pspr.obj" \
	"$(INTDIR)\p_saveg.obj" \
	"$(INTDIR)\p_setup.obj" \
	"$(INTDIR)\p_sight.obj" \
	"$(INTDIR)\p_spec.obj" \
	"$(INTDIR)\p_switch.obj" \
	"$(INTDIR)\p_telept.obj" \
	"$(INTDIR)\p_tick.obj" \
	"$(INTDIR)\p_user.obj" \
	"$(INTDIR)\r_bsp.obj" \
	"$(INTDIR)\r_clipper.obj" \
	"$(INTDIR)\r_gl.obj" \
	"$(INTDIR)\r_glExt.obj" \
	"$(INTDIR)\r_lights.obj" \
	"$(INTDIR)\r_main.obj" \
	"$(INTDIR)\r_sky.obj" \
	"$(INTDIR)\r_texture.obj" \
	"$(INTDIR)\r_things.obj" \
	"$(INTDIR)\r_vertices.obj" \
	"$(INTDIR)\r_wipe.obj" \
	"$(INTDIR)\s_sound.obj" \
	"$(INTDIR)\w_file.obj" \
	"$(INTDIR)\w_merge.obj" \
	"$(INTDIR)\w_wad.obj" \
	"$(INTDIR)\z_zone.obj" \
	"$(INTDIR)\wi_stuff.obj" \
	"$(INTDIR)\v_sdl.obj" \
	"$(INTDIR)\st_stuff.obj" \
	"$(INTDIR)\con_console.obj" \
	"$(INTDIR)\con_cvar.obj" \
	"$(INTDIR)\net_client.obj" \
	"$(INTDIR)\net_common.obj" \
	"$(INTDIR)\net_dedicated.obj" \
	"$(INTDIR)\net_io.obj" \
	"$(INTDIR)\net_loop.obj" \
	"$(INTDIR)\net_packet.obj" \
	"$(INTDIR)\net_query.obj" \
	"$(INTDIR)\net_sdl.obj" \
	"$(INTDIR)\net_server.obj" \
	"$(INTDIR)\net_structrw.obj" \
	"$(INTDIR)\in_stuff.obj" \
	"$(INTDIR)\sc_main.obj" \
	"$(INTDIR)\dgl.obj" \
	"$(INTDIR)\info.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\psnprntf.obj" \
	"$(INTDIR)\SDL_win32_main.obj" \
	"$(INTDIR)\tables.obj" \
	"$(INTDIR)\resource.res"

"..\DOOM64.EXE" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("DOOM64.dep")
!INCLUDE "DOOM64.dep"
!ELSE 
!MESSAGE Warning: cannot find "DOOM64.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DOOM64 - Win32 Release" || "$(CFG)" == "DOOM64 - Win32 Debug"
SOURCE=.\am_draw.c

"$(INTDIR)\am_draw.obj"	"$(INTDIR)\am_draw.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\am_map.c

"$(INTDIR)\am_map.obj"	"$(INTDIR)\am_map.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\d_devstat.c

"$(INTDIR)\d_devstat.obj"	"$(INTDIR)\d_devstat.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\d_main.c

"$(INTDIR)\d_main.obj"	"$(INTDIR)\d_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\d_net.c

"$(INTDIR)\d_net.obj"	"$(INTDIR)\d_net.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\f_finale.c

"$(INTDIR)\f_finale.obj"	"$(INTDIR)\f_finale.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\g_actions.c

"$(INTDIR)\g_actions.obj"	"$(INTDIR)\g_actions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\g_cmds.c

"$(INTDIR)\g_cmds.obj"	"$(INTDIR)\g_cmds.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\g_game.c

"$(INTDIR)\g_game.obj"	"$(INTDIR)\g_game.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\g_settings.c

"$(INTDIR)\g_settings.obj"	"$(INTDIR)\g_settings.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_audio.c

"$(INTDIR)\i_audio.obj"	"$(INTDIR)\i_audio.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_cpu.c

"$(INTDIR)\i_cpu.obj"	"$(INTDIR)\i_cpu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_exception.c

"$(INTDIR)\i_exception.obj"	"$(INTDIR)\i_exception.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_launcher.c

"$(INTDIR)\i_launcher.obj"	"$(INTDIR)\i_launcher.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_main.c

"$(INTDIR)\i_main.obj"	"$(INTDIR)\i_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_opndir.c

"$(INTDIR)\i_opndir.obj"	"$(INTDIR)\i_opndir.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_png.c

"$(INTDIR)\i_png.obj"	"$(INTDIR)\i_png.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_system.c

"$(INTDIR)\i_system.obj"	"$(INTDIR)\i_system.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\i_xinput.c

"$(INTDIR)\i_xinput.obj"	"$(INTDIR)\i_xinput.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_cheat.c

"$(INTDIR)\m_cheat.obj"	"$(INTDIR)\m_cheat.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_fixed.c

"$(INTDIR)\m_fixed.obj"	"$(INTDIR)\m_fixed.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_keys.c

"$(INTDIR)\m_keys.obj"	"$(INTDIR)\m_keys.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_menu.c

"$(INTDIR)\m_menu.obj"	"$(INTDIR)\m_menu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_misc.c

"$(INTDIR)\m_misc.obj"	"$(INTDIR)\m_misc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_password.c

"$(INTDIR)\m_password.obj"	"$(INTDIR)\m_password.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_random.c

"$(INTDIR)\m_random.obj"	"$(INTDIR)\m_random.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\m_shift.c

"$(INTDIR)\m_shift.obj"	"$(INTDIR)\m_shift.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_ceilng.c

"$(INTDIR)\p_ceilng.obj"	"$(INTDIR)\p_ceilng.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_doors.c

"$(INTDIR)\p_doors.obj"	"$(INTDIR)\p_doors.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_enemy.c

"$(INTDIR)\p_enemy.obj"	"$(INTDIR)\p_enemy.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_floor.c

"$(INTDIR)\p_floor.obj"	"$(INTDIR)\p_floor.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_inter.c

"$(INTDIR)\p_inter.obj"	"$(INTDIR)\p_inter.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_lights.c

"$(INTDIR)\p_lights.obj"	"$(INTDIR)\p_lights.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_macros.c

"$(INTDIR)\p_macros.obj"	"$(INTDIR)\p_macros.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_map.c

"$(INTDIR)\p_map.obj"	"$(INTDIR)\p_map.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_maputl.c

"$(INTDIR)\p_maputl.obj"	"$(INTDIR)\p_maputl.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_mobj.c

"$(INTDIR)\p_mobj.obj"	"$(INTDIR)\p_mobj.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_plats.c

"$(INTDIR)\p_plats.obj"	"$(INTDIR)\p_plats.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_pspr.c

"$(INTDIR)\p_pspr.obj"	"$(INTDIR)\p_pspr.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_saveg.c

"$(INTDIR)\p_saveg.obj"	"$(INTDIR)\p_saveg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_setup.c

"$(INTDIR)\p_setup.obj"	"$(INTDIR)\p_setup.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_sight.c

"$(INTDIR)\p_sight.obj"	"$(INTDIR)\p_sight.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_spec.c

"$(INTDIR)\p_spec.obj"	"$(INTDIR)\p_spec.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_switch.c

"$(INTDIR)\p_switch.obj"	"$(INTDIR)\p_switch.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_telept.c

"$(INTDIR)\p_telept.obj"	"$(INTDIR)\p_telept.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_tick.c

"$(INTDIR)\p_tick.obj"	"$(INTDIR)\p_tick.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\p_user.c

"$(INTDIR)\p_user.obj"	"$(INTDIR)\p_user.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_bsp.c

"$(INTDIR)\r_bsp.obj"	"$(INTDIR)\r_bsp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_clipper.c

"$(INTDIR)\r_clipper.obj"	"$(INTDIR)\r_clipper.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_gl.c

"$(INTDIR)\r_gl.obj"	"$(INTDIR)\r_gl.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_glExt.c

"$(INTDIR)\r_glExt.obj"	"$(INTDIR)\r_glExt.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_lights.c

"$(INTDIR)\r_lights.obj"	"$(INTDIR)\r_lights.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_main.c

"$(INTDIR)\r_main.obj"	"$(INTDIR)\r_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_sky.c

"$(INTDIR)\r_sky.obj"	"$(INTDIR)\r_sky.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_texture.c

"$(INTDIR)\r_texture.obj"	"$(INTDIR)\r_texture.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_things.c

"$(INTDIR)\r_things.obj"	"$(INTDIR)\r_things.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_vertices.c

"$(INTDIR)\r_vertices.obj"	"$(INTDIR)\r_vertices.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\r_wipe.c

"$(INTDIR)\r_wipe.obj"	"$(INTDIR)\r_wipe.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\s_sound.c

"$(INTDIR)\s_sound.obj"	"$(INTDIR)\s_sound.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\w_file.c

"$(INTDIR)\w_file.obj"	"$(INTDIR)\w_file.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\w_merge.c

"$(INTDIR)\w_merge.obj"	"$(INTDIR)\w_merge.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\w_wad.c

"$(INTDIR)\w_wad.obj"	"$(INTDIR)\w_wad.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\z_zone.c

"$(INTDIR)\z_zone.obj"	"$(INTDIR)\z_zone.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\wi_stuff.c

"$(INTDIR)\wi_stuff.obj"	"$(INTDIR)\wi_stuff.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\v_sdl.c

"$(INTDIR)\v_sdl.obj"	"$(INTDIR)\v_sdl.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\st_stuff.c

"$(INTDIR)\st_stuff.obj"	"$(INTDIR)\st_stuff.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\con_console.c

"$(INTDIR)\con_console.obj"	"$(INTDIR)\con_console.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\con_cvar.c

"$(INTDIR)\con_cvar.obj"	"$(INTDIR)\con_cvar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Ext\ChocolateDoom\net_client.c

"$(INTDIR)\net_client.obj"	"$(INTDIR)\net_client.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_common.c

"$(INTDIR)\net_common.obj"	"$(INTDIR)\net_common.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_dedicated.c

"$(INTDIR)\net_dedicated.obj"	"$(INTDIR)\net_dedicated.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_io.c

"$(INTDIR)\net_io.obj"	"$(INTDIR)\net_io.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_loop.c

"$(INTDIR)\net_loop.obj"	"$(INTDIR)\net_loop.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_packet.c

"$(INTDIR)\net_packet.obj"	"$(INTDIR)\net_packet.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_query.c

"$(INTDIR)\net_query.obj"	"$(INTDIR)\net_query.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_sdl.c

"$(INTDIR)\net_sdl.obj"	"$(INTDIR)\net_sdl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_server.c

"$(INTDIR)\net_server.obj"	"$(INTDIR)\net_server.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ext\ChocolateDoom\net_structrw.c

"$(INTDIR)\net_structrw.obj"	"$(INTDIR)\net_structrw.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\in_stuff.c

"$(INTDIR)\in_stuff.obj"	"$(INTDIR)\in_stuff.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sc_main.c

"$(INTDIR)\sc_main.obj"	"$(INTDIR)\sc_main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dgl.c

"$(INTDIR)\dgl.obj"	"$(INTDIR)\dgl.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\info.c

"$(INTDIR)\info.obj"	"$(INTDIR)\info.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Ext\md5.c

"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\psnprntf.c

"$(INTDIR)\psnprntf.obj"	"$(INTDIR)\psnprntf.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\Ext\SDL_win32_main.c

"$(INTDIR)\SDL_win32_main.obj"	"$(INTDIR)\SDL_win32_main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\tables.c

"$(INTDIR)\tables.obj"	"$(INTDIR)\tables.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

