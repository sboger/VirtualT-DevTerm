/* display.cpp */

/* $Id: display.cpp,v 1.44 2015/02/24 23:41:16 kpettit1 Exp $ */

/*
 * Copyright 2004 Stephen Hurd and Ken Pettit
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <FL/Fl.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Pixmap.H>

//Added by J. VERNET for pref files
// see also cb_xxxxxx
#include <FL/Fl_Preferences.H>
#include <FL/fl_show_colormap.H>

#include "FLU/Flu_Button.h"
#include "FLU/Flu_Return_Button.h"
#include "FLU/flu_pixmaps.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "VirtualT.h"
#include "display.h"
#include "m100emu.h"
#include "io.h"
#include "file.h"
#include "setup.h"
#include "periph.h"
#include "memory.h"
#include "memedit.h"
#include "cpuregs.h"
#include "rememcfg.h"
#include "lpt.h"
#include "fl_action_icon.h"
#include "clock.h"
#include "fileview.h"
#include "romstrings.h"
#include "remote.h"
#include "tpddserverlog.h"


extern "C" {
extern RomDescription_t		gM100_Desc;
extern RomDescription_t		gM200_Desc;
extern RomDescription_t		gN8201_Desc;
extern RomDescription_t		gM10_Desc;
//JV
//extern RomDescription_t		gN8300_Desc;
extern RomDescription_t		gKC85_Desc;

extern RomDescription_t		*gStdRomDesc;
extern int					gRomSize;
extern int					gMaintCount;
extern int					gOsDelay;
extern int					gInMsPlanROM;
extern int					gDelayUpdateKeys;
extern unsigned char        gQuad;
void	set_target_frequency(int freq);
void	memory_monitor_cb(void);
}

// Callback routines to display windows from other modules
void 			cb_Ide(Fl_Widget* w, void*);
void 			cb_TpddServerLog(Fl_Widget* w, void*);

Fl_Window		*MainWin = NULL;
T100_Disp		*gpDisp;
T100_Disp		*gpDebugMonitor;
Fl_Box			*gpCode, *gpGraph, *gpKey, *gpSpeed, *gpCaps, *gpKeyInfo;
Fl_Action_Icon	*gpPrint;
Fl_Box			*gpMap = NULL;
Fl_Menu_Bar		*Menu;
Fl_Preferences	virtualt_prefs(Fl_Preferences::USER, "virtualt.", "virtualt" ); 
char			gsMenuROM[40];
char			gDelayedError[128] = {0};
int				MultFact = 3;
int				DisplayMode = 1;
int				Fullscreen = 0;
int				SolidChars = 0;
int				DispHeight = 64;
int				gRectsize = 2;
int				gXoffset;
int				gYoffset;
int				gSimKey = 0;
int				gFrameColor = FL_BLACK;
int				gDetailColor = FL_WHITE;
int				gBackgroundColor = FL_GRAY;
int				gPixelColor = FL_BLACK;
int				gLabelColor = FL_WHITE;
int				gConsoleDebug = FALSE;

Fl_Double_Window*	gDisplayColors;
Fl_Button*			gLcdBkButton;
Fl_Button*			gLcdPixelButton;
Fl_Button*			gFrameButton;
Fl_Button*			gDetailButton;
Fl_Button*			gFkeyLabelsButton;

class VT_Ide;

extern char*			print_xpm[];
extern Fl_Menu_Item		gPrintMenu[];
extern void				Ide_SavePrefs(void);

extern Fl_Pixmap		gPrinterIcon;
extern Fl_Pixmap		littlehome, little_desktop, little_favorites, ram_drive;
extern VTCpuRegs*		gcpuw;
extern Fl_Window*		gmew;
extern VT_Ide *			gpIde;
extern VTTpddServerLog*	gpLog;

void switch_model(int model);
void key_delay(void);
int remote_load_from_host(const char *filename);

#ifndef	WIN32
#define	min(a,b)	((a)<(b) ? (a):(b))
#endif

#define	VT_SPEED0_FREQ	2457600
#define	VT_SPEED1_FREQ	8000000
#define	VT_SPEED2_FREQ	20000000
#define	VT_SPEED3_FREQ	50000000

void setMonitorWindow(Fl_Window* pWin)
{
	if (pWin == 0)
	{
		gpDebugMonitor = 0;
		return;
	}

	if (gpDebugMonitor == 0)   
	{
		if (gModel == MODEL_T200)
		{
			gpDebugMonitor = new T200_Disp(0, 0, 240*2, 128*2);
			(T200_Disp &) *gpDebugMonitor = (T200_Disp &) *gpDisp;
		}
		else
		{
			gpDebugMonitor = new T100_Disp(0, 0, 240*2,64*2);
			// Copy current monitor to debug monitor
			*gpDebugMonitor = *gpDisp;

		}

		gpDebugMonitor->DisplayMode = 0;
		gpDebugMonitor->SolidChars = 1;
		gpDebugMonitor->MultFact = 2;
		gpDebugMonitor->gRectsize = 2;
		gpDebugMonitor->gXoffset = 0;
		gpDebugMonitor->gYoffset = 0;

		pWin->insert(*gpDebugMonitor, 3);
		gpDebugMonitor->show();
	}
}

int T100_Disp::m_simKeys[32];
int	T100_Disp::m_simEventKey;

const char *gSpKeyText[] = {
	"SHIFT",
	"CTRL",
	"GRAPH",
	"CODE",
	"NUM",
	"CAPS",
	"",
	"PAUSE",

	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",

	"SPACE",
	"BKSP",
	"TAB",
	"ESC",
	"PASTE",
	"LABEL",
	"PRINT",
	"ENTER",

	"",
	"",
	"",
	"",
	"LEFT",
	"RIGHT",
	"UP",
	"DOWN"
};

/*
=======================================================
Collapse window: Causes the given window to appear to
"collapse" into the main window by resizing it smaller
and smaller in steps.
=======================================================
*/
void collapse_window(Fl_Window* pWin)
{
	int		newx, newy, neww, newh;
	int		dx, dy, dw, dh;
	int		steps = 10, c;

#if !defined(__APPLE__)
	// Shrink the window to nothingness...
	newx = MainWin->x() + MainWin->w()/4;
	newy = MainWin->y() + MainWin->h()/4;
	neww = 50;
	newh = 50;

	dx = (newx - pWin->x()) / steps;
	dy = (newy - pWin->y()) / steps;
	dw = (neww - pWin->w()) / steps;
	dh = (newh - pWin->h()) / steps;
	for (c = 0; c < steps-1; c++)
	{
		pWin->resize(pWin->x()+dx, pWin->y()+dy, pWin->w()+dw, pWin->h()+dh);
		Fl::check();
	}
#endif

	// Finally, hide the window altogether
	pWin->hide();
}
/*
=======================================================
Expande window: Causes the given window to appear to
"expand" to it's final size and location by resizing it 
larger and larger in steps.
=======================================================
*/
void expand_window(Fl_Window* pWin, int newx, int newy, int neww, int newh)
{
	int dx, dy, dw, dh;
	int	c, steps = 9;

#if !defined(__APPLE__)
	pWin->hide();
	pWin->resize(MainWin->x(), MainWin->y(), 50, 150);
#endif
	pWin->show();

#if !defined(__APPLE__)
	dx = (newx - pWin->x()) / steps;
	dy = (newy - pWin->y()) / steps;
	dw = (neww - pWin->w()) / steps;
	dh = (newh - pWin->h()) / steps;
	for (c = 0; c < steps-1; c++)
	{
		pWin->resize(pWin->x()+dx, pWin->y()+dy, pWin->w()+dw, pWin->h()+dh);
		Fl::check();
	}
#endif

	pWin->resize(newx, newy, neww, newh);
}
/*
=======================================================
External Menu Item Callbacks
=======================================================
*/
void disassembler_cb(Fl_Widget* w, void*);

void rspeed(Fl_Widget* w, void*) 
{
	fullspeed = 0;
	set_target_frequency(VT_SPEED0_FREQ);
	gMaintCount = 4096;
    virtualt_prefs.set("fullspeed",0);

}
void fspeed(Fl_Widget* w, void*) 
{
	fullspeed = 3;
	set_target_frequency(VT_SPEED3_FREQ);
#ifdef WIN32 
	gMaintCount = 100000;
///	gMaintCount = 4096;
#elif defined(__APPLE__)
	gMaintCount = 65536;
#else
	gMaintCount = 131072;
#endif
	virtualt_prefs.set("fullspeed",3);

}

void pf1speed(Fl_Widget* w, void*) 
{
	fullspeed = 1;
	set_target_frequency(VT_SPEED1_FREQ);
#ifdef WIN32
	gMaintCount = 16384;
//	gMaintCount = 4096;
#else
	gMaintCount = 4096;
#endif
	virtualt_prefs.set("fullspeed",1);

}

void pf2speed(Fl_Widget* w, void*) 
{
	fullspeed = 2;
	set_target_frequency(VT_SPEED2_FREQ);
#ifdef WIN32
	gMaintCount = 100000;
//	gMaintCount = 4096;
#else
	gMaintCount = 16384;
#endif
	virtualt_prefs.set("fullspeed",2);

}

void close_disp_cb(Fl_Widget* w, void*)
{
	if (gpDisp != NULL)
	{
		int		cpuregs_was_open = FALSE;
		int		ide_was_open = FALSE;
		int		memedit_was_open = FALSE;

		// Check if there is a CpuRegs window open and tell it to
		// save user preferences if there is
		if (gcpuw != NULL)
		{
			gcpuw->SavePrefs();
			cpuregs_was_open = TRUE;
		}

		// Check if there is a Memory Edit window open and tell it to
		// save user preferences is there is
		if (gmew != NULL)
		{
			MemoryEditor_SavePrefs();
			memedit_was_open = TRUE;
		}

		// Check if the IDE is opened and tell it to save user
		// preferences if it is
		if (gpIde != NULL)
		{
			Ide_SavePrefs();
			ide_was_open = TRUE;
		}

		// Save preferences for TPDD Server Log window
		if (gpLog != NULL)
			gpLog->SavePreferences();

		// Save open status
		virtualt_prefs.set("WindowState_MemEdit", memedit_was_open);
		virtualt_prefs.set("WindowState_CpuRegs", cpuregs_was_open);
		virtualt_prefs.set("WindowState_IDE", ide_was_open);

		gExitApp = 1;
		gExitLoop = 1;
	}
}

/*
=====================================================================
resize_window:	This function resizes the main window and repositions
				all of the status windows.  The new size of the 
				window is determined by the following values:

				gModel, MultFact, DisplayMode
=====================================================================
*/
extern "C" void resize_window()
{
#ifdef WIN32
	int		hiddenTaskBarAdjust = 0;
#endif

	if (gpDisp == NULL)
		return;

	/* Determine the height of the display based on model */
	if (gModel == MODEL_T200)
	{
		gpDisp->DispHeight = 128;
		DispHeight = 128;
	}
	else
	{
		gpDisp->DispHeight = 64;
		DispHeight = 64;
	}

	if (Fullscreen)
	{
		MainWin->fullscreen();
		//MultFact = min(MainWin->w()/240, MainWin->h()/128);
		gpDisp->MultFact = MultFact;
	}
	else
	{
		if (MainWin->y() <= 0)
			MainWin->fullscreen_off(32, 32, 240*gpDisp->MultFact +
				90*gpDisp->DisplayMode+2, gpDisp->DispHeight*gpDisp->MultFact + 
				50*gpDisp->DisplayMode + MENU_HEIGHT + 22);
		else
			MainWin->fullscreen_off(MainWin->x(), MainWin->y(), 240*gpDisp->MultFact +
				90*gpDisp->DisplayMode+2, gpDisp->DispHeight*gpDisp->MultFact + 
				50*gpDisp->DisplayMode + MENU_HEIGHT + 22);
	}

	/* Ensure the window isn't off the top of the screen */
	if (MainWin->y() <= 0)
	{
		if (!Fullscreen)
			MainWin->resize(32, 32, 240*gpDisp->MultFact +
				90*gpDisp->DisplayMode+2,gpDisp->DispHeight*gpDisp->MultFact + 
				50*gpDisp->DisplayMode + MENU_HEIGHT + 22);
	}
	else
	{
		MainWin->resize(MainWin->x(), MainWin->y(), 240*gpDisp->MultFact +
			90*gpDisp->DisplayMode+2,gpDisp->DispHeight*gpDisp->MultFact + 
			50*gpDisp->DisplayMode + MENU_HEIGHT + 22);
	}

	Menu->resize(0, 0, MainWin->w(), MENU_HEIGHT-2);
	gpDisp->resize(0, MENU_HEIGHT, MainWin->w(), MainWin->h() - MENU_HEIGHT - 20);
	int ctrlY = MainWin->h() - 20;

#ifdef ZIPIT_Z2
	/* Running on Zipit Z2!  Make the status items a bit smaller */
	gpGraph->resize(0, ctrlY, 60, 20);
	gpCode->resize(60, ctrlY, 60, 20);
	gpCaps->resize(-120, ctrlY, 60, 20);	/* Remove this from display completely */
	gpKey->resize(-180, ctrlY, 120, 20);	/* Remove this from display completely */
	gpSpeed->resize(120, ctrlY, 60, 20);
	gpMap->resize(180, ctrlY, 60, 20);
	gpPrint->resize(240, ctrlY, 60, 20);
	gpKeyInfo->resize(-480, ctrlY, MainWin->w()-480, 20);
#else
	gpGraph->resize(0, ctrlY, 60, 20);
	gpCode->resize(60, ctrlY, 60, 20);
	gpCaps->resize(120, ctrlY, 60, 20);
	gpKey->resize(180, ctrlY, 120, 20);
	gpSpeed->resize(300, ctrlY, 60, 20);
	gpMap->resize(360, ctrlY, 60, 20);
	gpPrint->resize(420, ctrlY, 60, 20);
	gpKeyInfo->resize(480, ctrlY, MainWin->w()-480, 20);
	if (gpDisp->MultFact < 3)
	{
		gpKeyInfo->label("F Keys");
		gpKeyInfo->tooltip("F9:Label  F10:Print  F11:Paste  F12:Pause");
	}
	else
	{
		gpKeyInfo->tooltip("");
		gpKeyInfo->label("F9:Label  F10:Print  F11:Paste  F12:Pause");
	}
#endif	/* ZIPIT_Z2 */

	gpDisp->CalcScreenCoords();

	Fl::check();
	MainWin->redraw();
}

/*
=======================================================
Calculate the xoffset, yoffset, border locations, etc.
=======================================================
*/
void T100_Disp::CalcScreenCoords(void)
{
	// Calculatet the pixel rectangle size
	::gRectsize = MultFact - (1 - SolidChars);
	if (::gRectsize == 0)
		::gRectsize = 1;

	// Calculate xoffset and yoffset
#ifdef ZIPIT_Z2
	{
		::gXoffset = 0;
		::gYoffset = (MainWin->h() - MENU_HEIGHT - 20 - (int) (float) DispHeight * 1.3333) / 3 + MENU_HEIGHT+1;
	}
#else
	if (Fullscreen)
	{
		::gXoffset = MainWin->w() / 2 - 120 * MultFact;
		::gYoffset = (MainWin->h() - MENU_HEIGHT - 20 - DispHeight * MultFact) / 3 + MENU_HEIGHT+1;
	}
	else
	{
		::gXoffset = 45*DisplayMode+1;
		::gYoffset = 25*DisplayMode + MENU_HEIGHT+1;
	}
#endif

	gRectsize = ::gRectsize;
	gXoffset = ::gXoffset;
	gYoffset = ::gYoffset;

	// If the display is framed, then calculate the frame coords
	if (DisplayMode)
	{
		// Calculate the Bezel location
		int		wantedH = 0;
		int		wantedW = 0;
//		int		topH, bottomH, leftW, rightW;
		int		bottomSpace;
		int		rightSpace;

		// Calculate the top height of the Bezel
		m_HasTopChassis = TRUE;
		if (gYoffset-1 - MENU_HEIGHT-1 >= wantedH + 5)
			m_BezelTopH = wantedH;
		else
		{
			// Test if there's room for both Bezel and chassis detail
			if (gYoffset > 6)
				m_BezelTopH = gYoffset - MENU_HEIGHT - 1 - 6;
			else
			{
				m_BezelTopH = gYoffset - MENU_HEIGHT - 1 - 1;
				m_HasTopChassis = FALSE;
			}
		}
		m_BezelTop = gYoffset - m_BezelTopH - 1;

		// Calculate the bottom height of the Bezel
		m_BezelBottom = gYoffset + DispHeight * MultFact + 1;
		bottomSpace = MainWin->h() - m_BezelBottom - 20;
		m_HasBottomChassis = TRUE;
		if (bottomSpace >= wantedH + 5)
			m_BezelBottomH = wantedH;
		else
		{
			m_BezelBottomH = bottomSpace;
			m_HasBottomChassis = FALSE;
		}

		// Calculate the left Bezel border width
		m_HasLeftChassis = TRUE;
		if (gXoffset-1 >= wantedW + 5)
			m_BezelLeftW = wantedW;
		else
		{
			// Test if there's room for Bezel plus chassis
			if (gXoffset > 6)
				m_BezelLeftW = gXoffset - 6;
			else
			{
				m_BezelLeftW = gXoffset - 1;
				m_HasLeftChassis = FALSE;
			}
		}
		m_BezelLeft = gXoffset - m_BezelLeftW - 1;

		// Calculate the Bezel right width
		m_BezelRight = gXoffset + 240 * MultFact + 1;
		rightSpace = w() - m_BezelRight;
		m_HasRightChassis = TRUE;
		if (rightSpace >= wantedW + 5)
			m_BezelRightW = wantedW;
		else
		{
			// Test if there's room for Bezel plus chassis
			if (rightSpace > 5)
				m_BezelRightW = rightSpace - 5;
			else
			{
				m_BezelRightW = rightSpace;
				m_HasRightChassis = FALSE;
			}
		}
	}
}

void show_error (const char *st)
{
	// Check if MainWin created yet.  If not, delay the error
	if (MainWin == NULL)
	{
		strcpy(gDelayedError, st);
		return;
	}

	fl_alert("%s", st);
	
}

void show_message (const char *st)
{
	fl_message("%s", st);
	
}

/*
=======================================================
Menu Item Callbacks
=======================================================
*/
void cb_1x(Fl_Widget* w, void*)
{
	gpDisp->MultFact = 1;
	MultFact = 1;
	Fullscreen = 0;

	virtualt_prefs.set("MultFact",1);
	resize_window();
}
void cb_2x(Fl_Widget* w, void*)
{
	gpDisp->MultFact = 2;
	MultFact = 2;
	Fullscreen = 0;

	virtualt_prefs.set("MultFact",2);
	resize_window();
}
void cb_3x(Fl_Widget* w, void*)
{
	gpDisp->MultFact = 3;
	MultFact = 3;
	Fullscreen = 0;

	virtualt_prefs.set("MultFact",3);
	resize_window();
}
void cb_4x(Fl_Widget* w, void*)
{
	gpDisp->MultFact = 4;
	MultFact = 4;
	Fullscreen = 0;

	virtualt_prefs.set("MultFact",4);
	resize_window();
}
void cb_5x(Fl_Widget* w, void*)
{
	gpDisp->MultFact = 5;
	MultFact = 5;
	Fullscreen = 0;

	virtualt_prefs.set("MultFact",5);
	resize_window();
}
void cb_fullscreen(Fl_Widget* w, void*)
{
	gpDisp->MultFact = 5;
	MultFact = 5;
	Fullscreen = 1;

	virtualt_prefs.set("MultFact",5);
	resize_window();
}
void cb_framed(Fl_Widget* w, void*)
{
	gpDisp->DisplayMode  ^= 1;
	DisplayMode ^= 1;

	virtualt_prefs.set("DisplayMode",gpDisp->DisplayMode);
	resize_window();
}
void cb_solidchars (Fl_Widget* w, void*)
{
	gpDisp->SolidChars  ^= 1;
	SolidChars ^= 1;
	
	virtualt_prefs.set("SolidChars",gpDisp->SolidChars);
	resize_window();
}
void cb_save_basic(Fl_Widget* w, void*)
{
	BasicSaveMode  ^= 1;
    virtualt_prefs.set("BasicSaveMode",BasicSaveMode);
}
void cb_save_co(Fl_Widget* w, void*)
{
	COSaveMode  ^= 1;
    virtualt_prefs.set("COSaveMode",COSaveMode);
}

void cb_reset (Fl_Widget* w, void*)
{
	int 	a;
	
	if (w != NULL)
		a=fl_choice("Really Reset ?", "No", "Yes", NULL);
	else
		a = 1;

	if(a==1) 
	{
		resetcpu();
		show_remem_mode();

		if (gpDisp != NULL)
			gpDisp->Reset();
		if (gpDebugMonitor != 0)
			gpDebugMonitor->Reset();
	}
}

void remote_reset(void)
{
	cb_reset(NULL, NULL);
}

void cb_coldBoot (Fl_Widget* w, void*)
{
	int a;

	if (gReMem)
	{
		if (w != NULL)
			a = fl_choice("Cold Boot.  Reload System ROM too?", "Cancel", "Yes", "No", NULL);
		else
			a = 2;
		if (a == 1)
			reload_sys_rom();
	}
	else
	{
		if (w != NULL)
			a = fl_choice("Really Cold Boot ?", "No", "Yes", NULL);
		else
			a = 2;
	}
	
	if(a != 0) 
	{
		cold_boot_mem();
		resetcpu();

		show_remem_mode();

		// Reset the main window
		if (gpDisp != NULL)
			gpDisp->Reset();

		// Reset any debug monitor windows
		if (gpDebugMonitor != 0)
			gpDebugMonitor->Reset();
		fileview_model_changed();

		// Refresh the memory editor if it is opened
		if (gmew != NULL)
			memory_monitor_cb();

	}
}

void remote_cold_boot(void)
{
	cb_coldBoot(NULL, NULL);
}

void cb_ReloadOptRom (Fl_Widget* w, void*)
{
	load_opt_rom();
}

void cb_UnloadOptRom (Fl_Widget* w, void*)
{
	char	option_name[32];

	gsOptRomFile[0] = 0;

	// Update user preferences
	get_model_string(option_name, gModel);
	strcat(option_name, "_OptRomFile");
    virtualt_prefs.set(option_name, gsOptRomFile);

	// Clear menu
	strcpy(gsMenuROM, "No ROM loaded");

	// Clear optROM memory
	load_opt_rom();

	// Reset the CPU
	if (w != NULL)
		resetcpu();

}

void cb_M100(Fl_Widget* w, void*)
{
	/* Switch to new model */
	switch_model(MODEL_M100);
}
void cb_M102(Fl_Widget* w, void*)
{
	/* Switch to new model */
	switch_model(MODEL_M102);
}
void cb_M200(Fl_Widget* w, void*)
{
	/* Switch to new model */
	switch_model(MODEL_T200);
}
void cb_M10(Fl_Widget* w, void*)
{
	/* Switch to new model */
	switch_model(MODEL_M10);
}
void cb_PC8201(Fl_Widget* w, void*)
{
	/* Switch to new model */
	switch_model(MODEL_PC8201);
}
void cb_PC8300(Fl_Widget* w, void*)
{
	/* Switch to new model */
	switch_model(MODEL_PC8300);
}

void cb_KC85(Fl_Widget* w, void*)
{
	/* Switch to new model */
	switch_model(MODEL_KC85);
}


#ifdef	__APPLE__
//JV 08/10/05: add an action to choose the working directory
void cb_choosewkdir(Fl_Widget* w, void*)
{
	/* Choose the working directory (ROM, RAM Files... */
	const char *ret;
	char		rompath[512];
	struct stat romStat;
	
	/* Get working directory from chooser */
    ret = ChooseWorkDir();
	if (ret == NULL) 
		return; 
	else 
	{
		// When changing directory, we must preserve RAM if the
		// emulation is running.
		get_rom_path(rompath, gModel);
		if (stat(rompath, &romStat) == 0)
		{
			save_ram();
			save_model_time();
		}

		free_mem();
		strcpy(path, ret);
		virtualt_prefs.set("Path", path);

		// Re-initialize emulation from new directory
		init_pref();
		load_memory_preferences();
		init_mem();
		get_model_time();

		/* Re-initialize the CPU */
		init_cpu();

		/* Update Memory Editor window if any */
		cb_MemoryEditorUpdate();

		// Update the File View window if it is open
		fileview_model_changed();
	}
}
//--JV
#endif

/*
=======================================================
cb_frame_btn:	Process the frame color button.
=======================================================
*/
void cb_frame_btn (Fl_Widget* w, void*)
{
  gFrameButton->color(fl_show_colormap(gFrameButton->color()));
}

/*
=======================================================
cb_detail_btn:	Process the detail color button.
=======================================================
*/
void cb_detail_btn (Fl_Widget* w, void*)
{
  gDetailButton->color(fl_show_colormap(gDetailButton->color()));
}

/*
=======================================================
cb_background_btn:	Process the background color button.
=======================================================
*/
void cb_background_btn (Fl_Widget* w, void*)
{
  gLcdBkButton->color(fl_show_colormap(gLcdBkButton->color()));
}

/*
=======================================================
cb_pixel_btn:	Process the pixel color button.
=======================================================
*/
void cb_pixels_btn (Fl_Widget* w, void*)
{
  gLcdPixelButton->color(fl_show_colormap(gLcdPixelButton->color()));
}

/*
=======================================================
cb_fkey_labels_btn:	Process the pixel color button.
=======================================================
*/
void cb_fkey_labels_btn (Fl_Widget* w, void*)
{
  gFkeyLabelsButton->color(fl_show_colormap(gFkeyLabelsButton->color()));
}

/*
=======================================================
cb_colors_ok_btn:	Process the colors OK button.
=======================================================
*/
void cb_colors_ok_btn (Fl_Widget* w, void*)
{
	if (gDisplayColors != NULL)
		gDisplayColors->hide();

	gFrameColor = gFrameButton->color();
	gDetailColor = gDetailButton->color();
	gBackgroundColor = gLcdBkButton->color();
	gPixelColor = gLcdPixelButton->color();
	gLabelColor = gFkeyLabelsButton->color();
    virtualt_prefs.set("BackgroundColor",gBackgroundColor);
    virtualt_prefs.set("PixelColor",gPixelColor);
    virtualt_prefs.set("FrameColor",gFrameColor);
    virtualt_prefs.set("DetailColor",gDetailColor);
    virtualt_prefs.set("LabelColor",gLabelColor);

	if (gpDisp != NULL)
	{
		gpDisp->m_BackgroundColor = gBackgroundColor;
		gpDisp->m_PixelColor = gPixelColor;
		gpDisp->m_FrameColor = gFrameColor;
		gpDisp->m_DetailColor = gDetailColor;
		gpDisp->m_LabelColor = gLabelColor;
		gpDisp->redraw();
	}
}

/*
=======================================================
cb_colors_cancel_btn:	Process the colors Cancel button
=======================================================
*/
void cb_colors_cancel_btn (Fl_Widget* w, void*)
{
	if (gDisplayColors != NULL)
		gDisplayColors->hide();

	if (gpDisp != NULL)
	{
		gpDisp->m_BackgroundColor = gBackgroundColor;
		gpDisp->m_PixelColor = gPixelColor;
		gpDisp->m_FrameColor = gFrameColor;
		gpDisp->m_DetailColor = gDetailColor;
		gpDisp->m_LabelColor = gLabelColor;
		gpDisp->redraw();
	}
}

/*
=======================================================
cb_colors_cancel_btn:	Process the colors Cancel button
=======================================================
*/
void cb_colors_preview_btn (Fl_Widget* w, void*)
{
	if (gpDisp != NULL)
	{
		gpDisp->m_BackgroundColor = gLcdBkButton->color();
		gpDisp->m_PixelColor = gLcdPixelButton->color();
		gpDisp->m_FrameColor = gFrameButton->color();
		gpDisp->m_DetailColor = gDetailButton->color();
		gpDisp->m_LabelColor = gFkeyLabelsButton->color();
		gpDisp->redraw();
	}
}
/*
=======================================================
cb_colors_defaults_btn:	Process the default colors btn
=======================================================
*/
void cb_colors_defaults_btn (Fl_Widget* w, void*)
{
	gLcdBkButton->color(FL_GRAY);
	gLcdPixelButton->color(FL_BLACK);
	gFrameButton->color(FL_BLACK);
	gDetailButton->color(FL_WHITE);
	gFkeyLabelsButton->color(FL_WHITE);
	gLcdBkButton->redraw();
	gLcdPixelButton->redraw();
	gFrameButton->redraw();
	gDetailButton->redraw();
	gFkeyLabelsButton->redraw();
}

/*
=======================================================
cb_colors:	This routine creates a dialog box for 
selecting the main display colors.
=======================================================
*/
void cb_display_colors (Fl_Widget* w, void*)
{
	gDisplayColors = new Fl_Double_Window(330, 170, "Display Colors");
	gDisplayColors->callback(cb_colors_cancel_btn);

    gDisplayColors->align(FL_ALIGN_TOP_LEFT);

     { Flu_Button* o = new Flu_Button(20, 130, 80, 25, "Cancel");
      o->callback((Fl_Callback*)cb_colors_cancel_btn);
    }
    { Flu_Button* o = new Flu_Button(120, 130, 80, 25, "Preview");
      o->callback((Fl_Callback*)cb_colors_preview_btn);
    }
    { Flu_Return_Button* o = new Flu_Return_Button(220, 130, 85, 25, "OK");
      o->callback((Fl_Callback*)cb_colors_ok_btn);
    }
    { Flu_Button* o = new Flu_Button(220, 60, 85, 25, "Defaults");
      o->callback((Fl_Callback*)cb_colors_defaults_btn);
    }

    { Fl_Button* o = gLcdBkButton = new Fl_Button(30, 20, 15, 15, "LCD Background");
      o->callback((Fl_Callback*)cb_background_btn);
      o->align(FL_ALIGN_RIGHT);
    }
    { Fl_Button* o = gLcdPixelButton = new Fl_Button(30, 40, 15, 15, "LCD Pixels");
      o->callback((Fl_Callback*)cb_pixels_btn);
      o->align(FL_ALIGN_RIGHT);
    }
    { Fl_Button* o = gFrameButton = new Fl_Button(30, 60, 15, 15, "Bezel");
      o->callback((Fl_Callback*)cb_frame_btn);
      o->align(FL_ALIGN_RIGHT);
    }
    { Fl_Button* o = gDetailButton = new Fl_Button(30, 80, 15, 15, "Chassis");
      o->callback((Fl_Callback*)cb_detail_btn);
      o->align(FL_ALIGN_RIGHT);
    }
    { Fl_Button* o = gFkeyLabelsButton = new Fl_Button(30, 100, 15, 15, "FKey Labels");
      o->callback((Fl_Callback*)cb_fkey_labels_btn);
      o->align(FL_ALIGN_RIGHT);
    }

	gFrameButton->color(gFrameColor);
	gDetailButton->color(gDetailColor);
	gLcdBkButton->color(gBackgroundColor);
	gLcdPixelButton->color(gPixelColor);
	gFkeyLabelsButton->color(gLabelColor);

    gDisplayColors->set_modal();
    gDisplayColors->end();
	gDisplayColors->show();

	while (gDisplayColors->visible())
		Fl::wait();

	delete gDisplayColors;
	gDisplayColors = NULL;
}

#ifdef WIN32
void cb_ConsoleDebug(Fl_Widget* w, void* pOpaque)
{
	if (!gConsoleDebug)
	{
#ifndef _DEBUG
		// Allocate a console
		AllocConsole();
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
#endif

		// Start the console control thread
		remote_start_console_thread();

		// Indicate we have console debugging active
		gConsoleDebug = TRUE;
	}
}
#endif

/*
=======================================================
cb_help:	This routine displays the Help Subsystem
=======================================================
*/
void cb_help (Fl_Widget* w, void*)
{

    Fl_Help_Dialog *HelpWin;
	//JV 02/02/2008
	
	
	HelpWin = new Fl_Help_Dialog();
	HelpWin->resize(HelpWin->x(), HelpWin->y(), 760, 550);
	
	#ifdef	__APPLE__
	char helppath[255];
	strcpy(helppath,path); strcat(helppath,"/doc/help.html");
	HelpWin->load(helppath);
	#else
	HelpWin->load("doc/help.html");
	#endif
	
	HelpWin->show();

}

/*
==========================================================
cb_about:	This callback routine displays the about box
==========================================================
*/
void cb_about (Fl_Widget* w, void*)
{
  
   Fl_Window* o = new Fl_Window(420, 340);
  
    { Fl_Box* o = new Fl_Box(20, 0, 345, 95, "Virtual T");
      o->labelfont(11);
      o->labelsize(80);
    }
    { Fl_Box* o = new Fl_Box(50, 105, 305, 40, "A Tandy Model 100/102/200 Emulator");
      o->labelfont(8);
      o->labelsize(24);
    }
    { Fl_Box* o = new Fl_Box(30, 150, 335, 35, "written by: "); 
      o->labelfont(8);
      o->labelsize(18);
    }
    { Fl_Box* o = new Fl_Box(30, 170, 335, 35, "Ken Pettit");
      o->labelfont(8);
      o->labelsize(18);
    }
    { Fl_Box* o = new Fl_Box(25, 190, 340, 35, "Stephen Hurd");
      o->labelfont(8);
      o->labelsize(18);
    }
    { Fl_Box* o = new Fl_Box(25, 210, 340, 35, "Jerome Vernet");
      o->labelfont(8);
      o->labelsize(18);
    }
    { Fl_Box* o = new Fl_Box(25, 230, 340, 35, "John Hogerhuis");
      o->labelfont(8);
      o->labelsize(18);
    }
    { Fl_Box* o = new Fl_Box(95, 265, 195, 25, "V "VERSION);
      o->labelfont(8);
      o->labelsize(18);
    }
    { Fl_Box* o = new Fl_Box(35, 295, 320, 25, "This program may be distributed freely ");
      o->labelfont(8);
      o->labelsize(16);
    }
    { Fl_Box* o = new Fl_Box(35, 315, 320, 25, "under the terms of the BSD license");
      o->labelfont(8);
      o->labelsize(16);
    }
    { Fl_Box* o = new Fl_Box(20, 75, 320, 20, "in FLTK");
      o->labelfont(8);
      o->labelsize(18);
    }
    o->end();
    o->show();   
}

/*
=======================================================
cb_select_copy:	This routine handles the copy menu 
item from the select menu
=======================================================
*/
void cb_select_copy (Fl_Widget* w, void*)
{
	if (gpDisp != NULL)
	{
		gpDisp->m_SimulatedCtrl = TRUE;
		gpDisp->WheelKey(FL_F + 5);
	}
}

/*
=======================================================
cb_select_cut:	This routine handles the copy menu 
item from the select menu
=======================================================
*/
void cb_select_cut (Fl_Widget* w, void*)
{
	if (gpDisp != NULL)
	{
		gpDisp->m_SimulatedCtrl = TRUE;
		gpDisp->WheelKey(FL_F + 6);
	}
}

/*
=======================================================
cb_select_cut:	This routine handles the copy menu 
item from the select menu
=======================================================
*/
void cb_select_cancel (Fl_Widget* w, void*)
{
	if (gpDisp != NULL)
	{
		gpDisp->m_SimulatedCtrl = TRUE;
		gpDisp->WheelKey(FL_Control_L);
		gpDisp->WheelKey('c');
	}
}

void cb_menu_fkey (Fl_Widget* w, void* key)
{
	int		fkey = (intptr_t) key;

	if (gpDisp != NULL)
	{
		gpDisp->WheelKey(fkey);
	}
}

void cb_menu_bank (Fl_Widget* w, void* key)
{
	int		bank =  atoi((char *) key);

    set_ram_bank(bank);
	resetcpu();
}

Fl_Menu_Item	gCopyCutMenu[] = {
	{ " Copy ", 0, cb_select_copy, 0, 0 },
	{ " Cut ", 0, cb_select_cut, 0, FL_MENU_DIVIDER },
	{ " Cancel ", 0, cb_select_cancel, 0, 0 },
	{ 0 }
};

Fl_Menu_Item	gLeftClickMenu[] = {
	{ " Paste ", 0, cb_menu_fkey, (void *) (FL_F + 11), FL_MENU_DIVIDER },
	{ " Label ", 0, cb_menu_fkey, (void *) (FL_F + 9), 0 },
	{ " Print ", 0, cb_menu_fkey, (void *) (FL_F + 10), 0 },
	{ " Pause ", 0, cb_menu_fkey, (void *) (FL_F + 12), 0 },
	{ 0 }
};

Fl_Menu_Item	gLeftClickQuadMenu[] = {
	{ " Paste ", 0, cb_menu_fkey, (void *) (FL_F + 11), FL_MENU_DIVIDER },
	{ " Label ", 0, cb_menu_fkey, (void *) (FL_F + 9), 0 },
	{ " Print ", 0, cb_menu_fkey, (void *) (FL_F + 10), 0 },
	{ " Pause ", 0, cb_menu_fkey, (void *) (FL_F + 12), FL_MENU_DIVIDER },
	{ " Bank 1 ", 0, cb_menu_bank, (void *) "0", 0 },
	{ " Bank 2 ", 0, cb_menu_bank, (void *) "1", 0 },
	{ " Bank 3 ", 0, cb_menu_bank, (void *) "2", 0 },
	{ " Bank 4 ", 0, cb_menu_bank, (void *) "3", 0 },
	{ 0 }
};

Fl_Menu_Item menuitems[] = {
  { "&File", 0, 0, 0, FL_SUBMENU },
	{ "Load file from HD",     0, cb_LoadFromHost, 0 },
	{ "Save file to HD",       0, cb_SaveToHost, 0, 0 },
	{ "Save Options",	       0, 0,  0, FL_SUBMENU | FL_MENU_DIVIDER },
		{ "Save Basic as ASCII",  0, cb_save_basic, (void *) 1, FL_MENU_TOGGLE },
		{ "Save CO as HEX",       0, cb_save_co,    (void *) 2, FL_MENU_TOGGLE },
		{ 0 },
//	{ "Load RAM...",      0,      cb_LoadRam, 0 },
//	{ "Save RAM...",      0,      cb_SaveRam, 0, 0 },
//	{ "RAM Snapshots...", 0,      0, 0, FL_MENU_DIVIDER },
	{ "E&xit",            0, 	  close_disp_cb, 0 },
	{ 0 },

  { "&Emulation",         0, 0,        0, FL_SUBMENU },
	{ "Reset",            0, cb_reset, 0, 0 },
	{ "Cold Boot",        0, cb_coldBoot, 0, FL_MENU_DIVIDER },
	{ "Model",            0, 0,        0, FL_SUBMENU 	 } ,
		{ "M100",   0, cb_M100,   (void *) 1, FL_MENU_RADIO | FL_MENU_VALUE },
		{ "M102",   0, cb_M102,   (void *) 2, FL_MENU_RADIO },
		{ "T200",   0, cb_M200,   (void *) 3, FL_MENU_RADIO },
		{ "PC-8201",  0, cb_PC8201, (void *) 4, FL_MENU_RADIO },
		{ "M10",    0, cb_M10, (void *) 5, FL_MENU_RADIO },
		{ "KC85",    0, cb_KC85, (void *) 6, FL_MENU_RADIO },
		{ "PC-8300",  0, cb_PC8300, (void *) 7, FL_MENU_RADIO },
		{ 0 },
	{ "Speed", 0, 0, 0, FL_SUBMENU },
		{ "2.4 MHz",     0, rspeed, (void *) 1, FL_MENU_RADIO | FL_MENU_VALUE },
		{ "Very CPU Friendly",0, pf1speed, (void *) 2, FL_MENU_RADIO },
		{ "CPU Friendly",0, pf2speed, (void *) 3, FL_MENU_RADIO },
		{ "Max Speed",   0, fspeed, (void *) 4, FL_MENU_RADIO },
		{ 0 },
	{ "Display", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER},
		{ "1x",  0, cb_1x, (void *) 1, FL_MENU_RADIO },
		{ "2x",  0, cb_2x, (void *) 2, FL_MENU_RADIO },
		{ "3x",  0, cb_3x, (void *) 3, FL_MENU_RADIO | FL_MENU_VALUE},
		{ "4x",  0, cb_4x, (void *) 4, FL_MENU_RADIO },
		{ "Fullscreen",  0, cb_fullscreen, (void *) 6, FL_MENU_RADIO | FL_MENU_DIVIDER},
		{ "Framed",  0, cb_framed, (void *) 1, FL_MENU_TOGGLE|FL_MENU_VALUE },
		{ "Solid Chars",  0, cb_solidchars, (void *) 1, FL_MENU_TOGGLE | FL_MENU_DIVIDER},
		{ "Display Colors",  0, cb_display_colors, 0, 0 },
		{ 0 },
	{ "Peripheral Setup...",     0, cb_PeripheralSetup, 0, 0 },
#ifdef	__APPLE__
    { "Directory...",			 0, cb_choosewkdir, 0, 0 }, // JV 08/10/05 menu Directory added
#endif
	{ "Memory Options...",       0, cb_MemorySetup, 0, FL_MENU_DIVIDER },
	{ "Option ROM",              0, 0, 0, FL_SUBMENU },
		{ gsMenuROM,             0, 0, 0, FL_MENU_DIVIDER },
		{ "Load ROM...",         0, cb_LoadOptRom, 0, 0 },
		{ "Unload ROM",          0, cb_UnloadOptRom, 0, 0 },
		{ "Reload ROM",          0, cb_ReloadOptRom, 0, 0 },
		{ 0 },
	{ 0 },

  { "&Tools", 0, 0, 0, FL_SUBMENU },
	{ "CPU Registers",         0, cb_CpuRegs },
	{ "Assembler / IDE",       0, cb_Ide},
	{ "Disassembler",          0, disassembler_cb },
	{ "Memory Editor",         0, cb_MemoryEditor },
#ifdef WIN32
	{ "Enable Console Debug",  0, cb_ConsoleDebug },
#endif
	{ "ReMem Configuration",   0, cb_RememCfg, 0, 0 },
	{ "Peripheral Devices",    0, cb_PeripheralDevices },
	{ "Model T File Viewer",   0, cb_FileView },
	{ "TPDD Server Log",       0, cb_TpddServerLog, 0, FL_MENU_INVISIBLE },
	{ "Socket Configuration",  0, cb_SocketSetup },
//	{ "TPDD Client",           0, cb_TpddClient },
	{ 0 },
  { "&Help", 0, 0, 0, FL_SUBMENU}, // | FL_MENU_DIVIDER},
	{ "Help", 0, cb_help },
	{ "About VirtualT", 0, cb_about },
	{ 0 },
//  { "HomeIcon", 0, 0, 0, 0 },
//  { "Icon2", 0, 0, 0, 0 },
//  { "Icon3", 0, 0, 0, 0 },
//  { "Icon4", 0, 0, 0, 0 },
  { 0 }
};

/*
============================================================================
remote_set_speed:	This function sets the speed of the emulation
============================================================================
*/
void remote_set_speed(int speed)
{
	int		mIndex;
	int		i;
	
	/* Set CPU Speed */
	fullspeed = speed;
	switch (fullspeed)
	{
#ifdef WIN32
		case 0: 
			gMaintCount = 4096; 
			set_target_frequency(VT_SPEED0_FREQ);
			break;
		case 1: 
			set_target_frequency(VT_SPEED1_FREQ);
			gMaintCount = 16384; 
			break;
		case 2: 
			set_target_frequency(VT_SPEED2_FREQ);
			gMaintCount = 100000; 
			break;
		case 3: 
			set_target_frequency(VT_SPEED3_FREQ);
			gMaintCount = 100000; 
			break;
		default: 
			set_target_frequency(VT_SPEED0_FREQ);
			gMaintCount = 4096; 
			break;
#else
		case 0: gMaintCount = 4096; break;
		case 1: gMaintCount = 8192; break;
		case 2: gMaintCount = 16384; break;
#ifdef __APPLE__
		case 3: gMaintCount = 65536; break;
#else
		case 3: gMaintCount = 131072; break;
#endif
		default: gMaintCount = 4096; break;
#endif
	}

	//==================================================
	// Update Speed menu item if not default value
	//==================================================
	mIndex = 0;
	while (menuitems[mIndex].callback_ != rspeed)
		mIndex++;
	for (i = 0; i < 4; i++)
	{
		if (i == fullspeed)
			menuitems[mIndex+i].flags= FL_MENU_RADIO |FL_MENU_VALUE;
		else
			menuitems[mIndex+i].flags= FL_MENU_RADIO;
	}
}

void init_menus(void)
{
	int 	remem_menu_flag = FL_MENU_INVISIBLE;
	int		mIndex;

	if (gReMem)
	{
		remem_menu_flag = 0;
	}

	// Locate the ReMem Configuration menu item
	mIndex = 0;
	while (menuitems[mIndex].callback_ != cb_RememCfg)
		mIndex++;
	menuitems[mIndex].flags= remem_menu_flag;
}

/*
============================================================================
Enables or disables the TPDD Packet Log menu item.
============================================================================
*/
void enable_tpdd_log_menu(int bEnabled)
{
	int 	tpdd_menu_flag = FL_MENU_INVISIBLE;
	int		mIndex;

	if (bEnabled)
	{
		tpdd_menu_flag = 0;
	}

	// Locate the ReMem Configuration menu item
	mIndex = 0;
	while (menuitems[mIndex].callback_ != cb_TpddServerLog)
		mIndex++;
	menuitems[mIndex].flags= tpdd_menu_flag;
}

/*
============================================================================
switch_model:	This function is called by the menu callback routines that 
				manage selection of the current model being emulated. It 
				saves the RAM for the current model, changes the to new 
				model, resizes the display, then reinitializes the system.
============================================================================
*/
void switch_model(int model)
{
	/* Save RAM for current emulation */
	save_ram();
	free_mem();

	/* Save time for current model */
	save_model_time();

	/* Save memory editor region markers and watch variables */
	if (gmew != NULL)
	{
		MemoryEditor_SavePrefs();
	}

	/* Switch to new model */
	gModel = model;
    virtualt_prefs.set("Model",gModel);

	init_pref();

	/* Load Memory preferences */
	load_memory_preferences();
	init_mem();
	if (gmew != NULL)
		MemoryEditor_LoadPrefs();
	get_model_time();

	gRomSize = 32768;
	/* Set pointer to ROM Description */
	if (gModel == MODEL_T200)
	{
		gStdRomDesc = &gM200_Desc;
		gRomSize = 40960;
	}
	else if (gModel == MODEL_PC8201)
		gStdRomDesc = &gN8201_Desc;
	else if (gModel == MODEL_M10)
	{
		gStdRomDesc = &gM10_Desc;
	}
	else if (gModel == MODEL_KC85)
	{
		gStdRomDesc = &gKC85_Desc;
	} 
//	else if (gModel == MODEL_PC8300)
//	{
//		gStdRomDesc = &gN8300_Desc;
//	} 
	else 
	      
		gStdRomDesc = &gM100_Desc;

	/* Clear the LCD */
	if (gpDisp != NULL)
		gpDisp->Clear();
	if (gpDebugMonitor != 0)
		gpDebugMonitor->Clear();

	delete MainWin;
	init_display();
	show_remem_mode();

	/* Resize the window in case of size difference */
	resize_window();

	/* Re-initialize the CPU */
	init_cpu();

	/* Update Memory Editor window if any */
	cb_MemoryEditorUpdate();

	// Update the File View window if it is open
	fileview_model_changed();
}

/*
=======================================================
cb_select_cut:	This routine handles the copy menu 
item from the select menu
=======================================================
*/
void cb_leftclick_cancel (Fl_Widget* w, void*)
{
	if (gpDisp != NULL)
	{
		gpDisp->take_focus();
	}
}

/*
=======================================================
T100:Disp:	This is the class construcor
=======================================================
*/
T100_Disp::T100_Disp(int x, int y, int w, int h) :
  Fl_Widget(x, y, w, h)
{
	int driver, c;

	m_FrameColor = gFrameColor;
	m_DetailColor = gDetailColor;
	m_BackgroundColor = gBackgroundColor;
	m_PixelColor = gPixelColor;
	m_LabelColor = gLabelColor;
	m_HaveMouse = FALSE;
	m_Select = FALSE;
	m_SelectComplete = FALSE;
	m_SimulatedCtrl = FALSE;

	for (driver = 0; driver < 10; driver++)
	{
		for (int c = 0; c < 256; c++)
			  lcd[driver][c] = 0;
		top_row[driver] = 0;
	}
	for (c = 0; c < 32; c++)
		m_simKeys[c] = 0;

	m_MyFocus = 0;
	m_DebugMonitor = 0;

#ifdef ZIPIT_Z2
	MultFact = 1;
	DisplayMode = 1;
	SolidChars = 1;
	DispHeight = 64;
	gRectsize = 1;
#else
	MultFact = 3;
	DisplayMode = 1;
	SolidChars = 0;
	DispHeight = 64;
	gRectsize = 2;
#endif	/* ZIPIT_Z2 */

	m_WheelKeyIn = 0;
	m_WheelKeyOut = 0;
	m_BezelTop = m_BezelLeft = m_BezelBottom = m_BezelRight = 0;
	m_BezelTopH = m_BezelLeftW = m_BezelBottomH = m_BezelRightW = 0;
	m_CopyCut = new Fl_Menu_Button(x, y, w, h, "Action");
	m_CopyCut->type(Fl_Menu_Button::POPUP123);
	m_CopyCut->hide();
	m_CopyCut->callback(cb_select_cancel);

	m_LeftClick = new Fl_Menu_Button(x, y, w, h, "Action");
	m_LeftClick->menu(gLeftClickMenu);
	m_LeftClick->type(0);
	m_LeftClick->callback(cb_leftclick_cancel);
	m_LeftClick->hide();

	m_LeftClickQuad = new Fl_Menu_Button(x, y, w, h, "Action");
	m_LeftClickQuad->menu(gLeftClickQuadMenu);
	m_LeftClickQuad->type(0);
	m_LeftClickQuad->callback(cb_leftclick_cancel);
	m_LeftClickQuad->hide();
}

T100_Disp::~T100_Disp()
{
	delete m_LeftClick;
	delete m_LeftClickQuad;
	delete m_CopyCut;
}

const T100_Disp& T100_Disp::operator=(const T100_Disp& srcDisp)
{
	int		driver, c;

	// Copy the LCD data
	if (this != &srcDisp)
	{
		for (driver = 0; driver < 10; driver++)
		{
			for (c = 0; c < 256; c++)
				lcd[driver][c] = srcDisp.lcd[driver][c];
			top_row[driver] = srcDisp.top_row[driver];		 
		}
	}

	return *this;
}

/*
==========================================================
Command:	This function processes commands sent to 
			Model 100 LCD subsystem.
==========================================================
*/
void T100_Disp::Command(int instruction, uchar data)
{
	int driver = instruction;
	int cmd = data & 0x3F;

	if (cmd == 0x3E)
	{
		if (top_row[driver] != data >> 6)
			damage(1, gXoffset, gYoffset, 240*MultFact, 64*MultFact);
		top_row[driver] = data >> 6;
	}
	else if (cmd == 0x3B)
	{
	}
	else if (cmd == 0x3C)
	{
	}
}

/*
=================================================================
Clear:	This routine clears the "LCD"
=================================================================
*/
void T100_Disp::Clear(void)
{
  int driver;

  for (driver = 0; driver < 10; driver++)
  {
	  for (int c = 0; c < 256; c++)
		  lcd[driver][c] = 0;
  }
}

/*
=================================================================
Reset:	This routine Resets the "LCD"
=================================================================
*/
void T100_Disp::Reset(void)
{
  int driver;

  for (driver = 0; driver < 10; driver++)
  {
	  for (int c = 0; c < 256; c++)
		  lcd[driver][c] = 0;
	  top_row[driver] = 0;
  }
  redraw();
}

/*
=================================================================
drawpixel:	This routine is called by the system to draw a single
			black pixel on the "LCD".
=================================================================
*/
// Draw the black pixels on the LCD
inline void T100_Disp::drawpixel(int x, int y, int color)
{
#ifdef ZIPIT_Z2
	if (color)
		fl_rectf(m_xCoord[x],m_yCoord[y], m_rectSize[x], m_rectSize[y]);
#else
	// Check if the pixel color is black and draw if it is
	if (color)
		fl_rectf(x*MultFact + gXoffset,y*MultFact + gYoffset,
		gRectsize, gRectsize);
#endif
}

/*
=================================================================
draw_static:	This routine draws the static portions of the LCD,
				such as erasing the background, drawing function
				key labls, etc.
=================================================================
*/
void T100_Disp::draw_static()
{
	int c;
	int width;
	int x_pos, inc, start, y_pos;
	int xl_start, xl_end, xr_start, xr_end;
	int	num_labels;

	// Draw gray "screen"
    fl_color(m_BackgroundColor);
    fl_rectf(x(),y(),w(),h());

	/* Check if the user wants the display "framed" */
	if (DisplayMode == 1)
	{
		// Color for outer border
		fl_color(FL_BLACK);

		// Draw border along the top
		if (m_HasTopChassis)
			fl_rectf(x(),y(),w(),m_BezelTop - MENU_HEIGHT - 1);

		// Draw border along the bottom
		if (m_HasBottomChassis)
			fl_rectf(x(),m_BezelBottom + m_BezelBottomH,w(),MainWin->h() - m_BezelBottom - m_BezelBottomH - 20);

		// Draw border along the left
		if (m_HasLeftChassis)
			fl_rectf(x(),y(),m_BezelLeft,h());

		// Draw border along the right
		if (m_HasRightChassis)
			fl_rectf(m_BezelRight + m_BezelRightW,y(),w()-m_BezelRight,h());
        

		// Color for inner border
		fl_color(m_FrameColor);
												    
		// Draw border along the top
		if (m_BezelTopH > 0)
			fl_rectf(m_BezelLeft,m_BezelTop,m_BezelRight - m_BezelLeft + m_BezelRightW,m_BezelTopH);

		// Draw border along the bottom
		if (m_BezelBottomH > 0)
			fl_rectf(m_BezelLeft,m_BezelBottom,m_BezelRight - m_BezelLeft + m_BezelRightW,m_BezelBottomH);

		// Draw border along the left
		if (m_BezelLeftW > 0)
			fl_rectf(m_BezelLeft, m_BezelTop, m_BezelLeftW, m_BezelBottom - m_BezelTop + m_BezelBottomH);

		// Draw border along the right
		if (m_BezelRightW > 0)
			fl_rectf(m_BezelRight,m_BezelTop, m_BezelRightW, m_BezelBottom - m_BezelTop + m_BezelBottomH);


#ifdef ZIPIT_Z2
		width = 320;
#else
		width = 240 * MultFact;
#endif
		num_labels = gModel == MODEL_PC8201 ? 5 : 8;
		inc = width / num_labels;
		start = gXoffset + width/16 - 2 * (5- (MultFact > 5? 5 : MultFact));
		fl_color(m_LabelColor);
		fl_font(FL_COURIER,12);
		char  text[3] = "F1";
//		y_pos = h()+20;
		y_pos = m_BezelBottom + 13; //y()+DispHeight*MultFact+40;
		xl_start = 2*MultFact;
		xl_end = 7*MultFact;

		xr_start = 12 + 2*MultFact;
		xr_end = 12 + 7*MultFact;

		// Draw function key labels
		for (c = 0; c < num_labels; c++)
		{
			// Draw text
			x_pos = start + inc*c;
			text[1] = c + '1';
			fl_draw(text, x_pos, y_pos);

			if (MultFact != 1)
			{
				// Draw lines to left
				fl_line(x_pos - xl_start, y_pos-2, x_pos - xl_end, y_pos-2);
				fl_line(x_pos - xl_start, y_pos-7, x_pos - xl_end, y_pos-7);
				fl_line(x_pos - xl_end, y_pos-2, x_pos - xl_end, y_pos-7);

				// Draw lines to right
				fl_line(x_pos + xr_start, y_pos-2, x_pos + xr_end, y_pos-2);
				fl_line(x_pos + xr_start, y_pos-7, x_pos + xr_end, y_pos-7);
				fl_line(x_pos + xr_end, y_pos-2, x_pos + xr_end, y_pos-7);
			}
		}
	}

	// Draw printer icon
	//printIcon.draw(420, gpGraph->y());

}

/*
=================================================================
draw:	This routine draws the entire LCD.  This is a member 
		function of Fl_Window.
=================================================================
*/
void T100_Disp::draw()
{
	int x=0;
	int y=0;
	int driver, col, row;
	uchar value;

	/* Draw static background stuff */
	draw_static();

	for (driver = 0; driver < 10; driver++)
	{
		for (row = 0; row < 4; row++)
		{
			for (col = 0; col < 50; col++)
			{
				if (((driver == 4) || (driver == 9)) && ((col & 0x3F)  >= 40))
					continue;

				x=(driver % 5) * 50;
				x+=col;

				y = ((row-top_row[driver]+4) % 4) << 3;
				if (driver > 4)
					y += 32;

				if (x > 240)
					x = x + 1;
				value = lcd[driver][row*64+col];

				// Erase line so it is grey, then fill in with black where needed
				fl_color(m_BackgroundColor);
#ifdef ZIPIT_Z2
				fl_rectf(m_xCoord[x],m_yCoord[y], m_rectSize[x],10+m_rectSize[y+8] == 2?0:1);
#else
				fl_rectf(x*MultFact + gXoffset,y*MultFact + 
					gYoffset,gRectsize,8*MultFact);
#endif
				fl_color(m_PixelColor);

				// Draw the black pixels
				drawpixel(x,y++, value&0x01);
				drawpixel(x,y++, value&0x02);
				drawpixel(x,y++, value&0x04);
				drawpixel(x,y++, value&0x08);
				drawpixel(x,y++, value&0x10);
				drawpixel(x,y++, value&0x20);
				drawpixel(x,y++, value&0x40);
				drawpixel(x,y++, value&0x80);
			}
		}
	}
}

/*
================================================================================
SetByte:	Updates the LCD with a byte of data as written from the I/O 
			interface from the 8085.
================================================================================
*/
void T100_Disp::SetByte(int driver, int col, uchar value)
{
	int x;
	int y;

//	if (driver < 10)
//	{
		// Check if LCD already has the value being requested
		if (lcd[driver][col] == value)
			return;

		//if (driver == 4)
			//printf("%d=%d ", col, value);

		// Load new value into lcd "RAM"
		lcd[driver][col] = value;

		// Check if value is an LCD command
		if ((col&0x3f) > 49)
			return;

		// Calculate X position of byte
		x=(driver % 5) * 50 + (col&0x3F);

		// Calcluate y position of byte
		y = ((((col & 0xC0) >> 6) - top_row[driver] + 4) % 4) * 8;
		if (driver > 4)
			y += 32;

		// Set the display
		window()->make_current();

		fl_color(m_BackgroundColor);
		fl_rectf(x*MultFact + gXoffset, y*MultFact + 
			gYoffset, gRectsize,MultFact<<3);
		fl_color(m_PixelColor);

		// Draw each pixel of byte
		drawpixel(x,y++,value&0x01);
		drawpixel(x,y++,(value&0x02)>>1);
		drawpixel(x,y++,(value&0x04)>>2);
		drawpixel(x,y++,(value&0x08)>>3);
		drawpixel(x,y++,(value&0x10)>>4);
		drawpixel(x,y++,(value&0x20)>>5);
		drawpixel(x,y++,(value&0x40)>>6);
		drawpixel(x,y++,(value&0x80)>>7);
}

/*
================================================================================
init_pref:	Reads Preferences File with all saved information.  

			Initial implementation added by J. VERNET
================================================================================
*/
void init_pref(void)
{
	char	option_name[32];

	virtualt_prefs.get("fullspeed",fullspeed,0);
	virtualt_prefs.get("MultFact",MultFact,3);
	virtualt_prefs.get("DisplayMode",DisplayMode,1);
	virtualt_prefs.get("SolidChars",SolidChars,0);
	virtualt_prefs.get("BasicSaveMode",BasicSaveMode,0);
	virtualt_prefs.get("COSaveMode",COSaveMode,0);
	virtualt_prefs.get("Model",gModel, MODEL_M100);
	virtualt_prefs.get("BackgroundColor",gBackgroundColor, FL_GRAY);
	virtualt_prefs.get("PixelColor",gPixelColor, FL_BLACK);
	virtualt_prefs.get("FrameColor",gFrameColor, FL_BLACK);
	virtualt_prefs.get("DetailColor",gDetailColor, FL_WHITE);
	virtualt_prefs.get("LabelColor",gLabelColor, FL_WHITE);
	
	Fullscreen = 0;
	if (MultFact == 5)
		//Fullscreen = 1;

	get_model_string(option_name, gModel);
	strcat(option_name, "_OptRomFile");
	virtualt_prefs.get(option_name,gsOptRomFile,"", 256);

	/* Set CPU Speed */
	switch (fullspeed)
	{
#ifdef WIN32
		case 0: 
			set_target_frequency(VT_SPEED0_FREQ);
			gMaintCount = 4096; 
			break;
		case 1: 
			set_target_frequency(VT_SPEED1_FREQ);
			gMaintCount = 16384; 
			break;
		case 2: 
			set_target_frequency(VT_SPEED2_FREQ);
			gMaintCount = 100000; 
			break;
		case 3: 
			set_target_frequency(VT_SPEED3_FREQ);
			gMaintCount = 100000; 
			break;
		default: 
			set_target_frequency(VT_SPEED0_FREQ);
			gMaintCount = 4096; 
			break;
#else
		case 0: 
			set_target_frequency(VT_SPEED0_FREQ);
			gMaintCount = 4096; 
			break;
		case 1: 
			set_target_frequency(VT_SPEED1_FREQ);
			gMaintCount = 8192; 
			break;
		case 2: 
			set_target_frequency(VT_SPEED2_FREQ);
			gMaintCount = 16384; 
			break;
#ifdef __APPLE__
		case 3: 
			set_target_frequency(VT_SPEED3_FREQ);
			gMaintCount = 65536; 
			break;
#else
		case 3: 
			set_target_frequency(VT_SPEED3_FREQ);
			gMaintCount = 131072; 
			break;
#endif
		default: 
			set_target_frequency(VT_SPEED0_FREQ);
			gMaintCount = 4096; 
			break;
#endif
	}

	if (strlen(gsOptRomFile) == 0)
	{
		strcpy(gsMenuROM, "No ROM loaded");
	}
	else
	{
		strcpy(gsMenuROM, fl_filename_name(gsOptRomFile));
	}
}

/*
================================================================================
deinit_display:	Hide and destroy the main window
================================================================================
*/
void deinit_display(void)
{
	if (MainWin != NULL)
	{
		MainWin->hide();
		delete gpDisp;
		delete MainWin;
		gpDisp = NULL;
		MainWin = NULL;
	}
}

/*
================================================================================
init_display:	Creates the main display window and all children (status, etc.).  
				This routine takes into account the current model, display
				size setting, and the platform (Zipit, etc.).
================================================================================
*/
void init_display(void)
{
	int			mIndex;
	int			i;
	char		temp[20];
#ifdef WIN32
	int			hiddenTaskBarAdjust = 0;
#endif

	Fl::visual(FL_DOUBLE|FL_INDEX);

	if (gModel == MODEL_T200)
		DispHeight = 128;
	else
		DispHeight = 64;
#ifdef ZIPIT_Z2
	MainWin = new Fl_Window(240*MultFact + 90*DisplayMode+2,DispHeight*MultFact +
		50*DisplayMode + MENU_HEIGHT + 22, "Virtual T");
	MainWin->fullscreen();
#else	/* ZIPIT_Z2 */
	MainWin = new Fl_Window(320, 240, "Virtual T");

	// Check if we are running in full screen mode
	if (MultFact == 5)
	{
		MainWin->fullscreen();
#ifdef WIN32
		int sx, sy, sw, sh;
		Fl::screen_xywh(sx, sy, sw, sh);
		if ((sh == 480) || (sh == 800) || (sh == 600) || (sh == 768) || (sh == 1024) || (sh == 1280))
			hiddenTaskBarAdjust = 4;
		MainWin->resize(sx, sy, sw, sh-hiddenTaskBarAdjust);
#endif	/* WIN32 */
		//MultFact = min(MainWin->w()/240, MainWin->h()/128);
	}
	else
	{
		if (MainWin->y() <= 0)
			MainWin->fullscreen_off(32, 32, 
			240*MultFact + 90*DisplayMode+2,DispHeight*MultFact +
			50*DisplayMode + MENU_HEIGHT + 22);
	}
#endif	/* ZIPIT_Z2 */

	Menu = new Fl_Menu_Bar(0, 0, MainWin->w(), MENU_HEIGHT-2);
	if (gModel == MODEL_T200)
		gpDisp = new T200_Disp(0, MENU_HEIGHT, MainWin->w(), MainWin->h() - MENU_HEIGHT - 20);
	else
		gpDisp = new T100_Disp(0, MENU_HEIGHT, MainWin->w(), MainWin->h() - MENU_HEIGHT - 20);

	int subMenuDepth = 0;
	for (i = 0; ; i++)
	{
		if (menuitems[i].text != NULL)
		{
			if (strcmp(menuitems[i].text, "HomeIcon") == 0)
			{
				menuitems[i++].image(littlehome);
				menuitems[i++].image(little_favorites);
				menuitems[i++].image(little_desktop);
				menuitems[i++].image(ram_drive);
				break;
			}
			if (menuitems[i].flags & FL_SUBMENU)
				subMenuDepth++;
		}
		else
		{
			if (subMenuDepth == 0)
				break;
			else
				subMenuDepth--;
		}
	}

	MainWin->callback(close_disp_cb);
	Menu->menu(menuitems);
        
	init_menus();

// Treat Values read in pref files
// J. VERNET 
	//==================================================
	// Update Speed menu item if not default value
	//==================================================
	mIndex = 0;
	while (menuitems[mIndex].callback_ != rspeed)
		mIndex++;
	for (i = 0; i < 4; i++)
	{
		if (i == fullspeed)
			menuitems[mIndex+i].flags= FL_MENU_RADIO |FL_MENU_VALUE;
		else
			menuitems[mIndex+i].flags= FL_MENU_RADIO;
	}

	//==================================================
	// Update Display Size from preference
	//==================================================
	mIndex = 0;
	// Find first display size menu item
	while (menuitems[mIndex].callback_ != cb_1x)
		mIndex++;
	mIndex--;

	int mf = MultFact;
	if (Fullscreen)
		mf = 5;
    for(i=1;i<6;i++)
    {
        if(i==mf) 
		{
            if(i==5) 
                menuitems[i+mIndex].flags=FL_MENU_RADIO | FL_MENU_VALUE | FL_MENU_DIVIDER;
            else menuitems[i+mIndex].flags=FL_MENU_RADIO | FL_MENU_VALUE;  
		}
		else
		{
            if(i==5) 
                menuitems[i+mIndex].flags=FL_MENU_RADIO | FL_MENU_DIVIDER;
            else menuitems[i+mIndex].flags=FL_MENU_RADIO;  
		}
    }

	//==================================================
	// Update Model selection
	//==================================================
	mIndex = 0;
	// Find first model menu item
	while (menuitems[mIndex].callback_ != cb_M100)
		mIndex++;

    for(i=MODEL_M100;i<=MODEL_PC8300;i++)
    {
        if(i==gModel) 
			menuitems[i+mIndex].flags=FL_MENU_RADIO | FL_MENU_VALUE;  
        else
			menuitems[i+mIndex].flags=FL_MENU_RADIO;  
    }

	//==================================================
	// Update DisplayMode parameter 
	//==================================================
    if(DisplayMode==0)
	{
		mIndex = 0;
		while (menuitems[mIndex].callback_ != cb_framed)
			mIndex++;
        menuitems[mIndex].flags=FL_MENU_TOGGLE;
	}

	/*
	==================================================
	Update SolidChars menu item
	==================================================
	*/
    if(SolidChars==1)
	{
		mIndex = 0;
		while (menuitems[mIndex].callback_ != cb_solidchars)
			mIndex++;
        menuitems[mIndex].flags=FL_MENU_TOGGLE|FL_MENU_VALUE|FL_MENU_DIVIDER;
	}
        
	/*
	==================================================
	Update BasicSaveMode parameter 
	==================================================
	*/
    if(BasicSaveMode==1)
	{
		mIndex = 0;
		while (menuitems[mIndex].callback_ != cb_save_basic)
			mIndex++;
        menuitems[mIndex].flags=FL_MENU_TOGGLE|FL_MENU_VALUE;
	}
    
	/*
	==================================================
	 Update COSaveMode parameter 
	==================================================
	*/
    if(COSaveMode==1)
	{
		mIndex = 0;
		while (menuitems[mIndex].callback_ != cb_save_co)
			mIndex++;
        menuitems[mIndex].flags=FL_MENU_TOGGLE|FL_MENU_VALUE;
	}

	/* 
	========================================
	Create Status boxes for various things
	========================================
	*/
	int ctrlY = MainWin->h() - 20;
#ifdef ZIPIT_Z2
	gpGraph = new Fl_Box(FL_DOWN_BOX,0, ctrlY, 60, 20,"GRAPH");
	gpCode = new Fl_Box(FL_DOWN_BOX,60, ctrlY, 60, 20,"CODE");
	gpCaps = new Fl_Box(FL_DOWN_BOX,-120, ctrlY, 60, 20,"CAPS");
	gpKey = new Fl_Box(FL_DOWN_BOX,-180, ctrlY, 120, 20,"");
	gpSpeed = new Fl_Box(FL_DOWN_BOX,120, ctrlY, 60, 20,"");
	gpMap = new Fl_Box(FL_DOWN_BOX,180, ctrlY, 60, 20,"");
	gpPrint = new Fl_Action_Icon(240, ctrlY, 60, 20, "Print Menu");
#else
	gpGraph = new Fl_Box(FL_DOWN_BOX,0, ctrlY, 60, 20,"GRAPH");
	gpCode = new Fl_Box(FL_DOWN_BOX,60, ctrlY, 60, 20,"CODE");
	gpCaps = new Fl_Box(FL_DOWN_BOX,120, ctrlY, 60, 20,"CAPS");
	gpKey = new Fl_Box(FL_DOWN_BOX,180, ctrlY, 120, 20,"");
	gpSpeed = new Fl_Box(FL_DOWN_BOX,300, ctrlY, 60, 20,"");
	gpMap = new Fl_Box(FL_DOWN_BOX,360, ctrlY, 60, 20,"");
	gpPrint = new Fl_Action_Icon(420, ctrlY, 60, 20, "Print Menu");
#endif

	/* 
	=============================================
	Assign labels and label sizes to status boxes
	=============================================
	*/
	gpGraph->labelsize(10);
	gpGraph->deactivate();
	gpCode->labelsize(10);
	gpCode->deactivate();
	gpCaps->labelsize(10);
	gpCaps->deactivate();
	gpKey->labelsize(10);
	gpSpeed->labelsize(10);
	gpMap->labelsize(10);
	gpPrint->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	gpPrint->set_image(&gPrinterIcon);
	gpPrint->menu(gPrintMenu);

#ifdef ZIPIT_Z2
	gpKeyInfo = new Fl_Box(FL_DOWN_BOX,-480, ctrlY, 20, 20, "");
#else
	if (MultFact < 3)
	{
		gpKeyInfo = new Fl_Box(FL_DOWN_BOX,480, ctrlY, MainWin->w()-480, 20,
			"F Keys");
		gpKeyInfo->tooltip("F9:Label  F10:Print  F11:Paste  F12:Pause");
	}
	else
		gpKeyInfo = new Fl_Box(FL_DOWN_BOX,480, ctrlY, MainWin->w()-480, 20,
			"F9:Label  F10:Print  F11:Paste  F12:Pause");
#endif
	gpKeyInfo->labelsize(10);
	gSimKey = 0;

	if (gpDisp != 0)
	{
		gpDisp->DispHeight = DispHeight;
		gpDisp->DisplayMode = DisplayMode;
		gpDisp->MultFact = MultFact;
		gpDisp->SolidChars = SolidChars;
		gpDisp->CalcScreenCoords();
	}

	/* End the Window and show it */
	MainWin->end();

    resize_window();
    
	if (!gNoGUI)
		MainWin->show();

#ifdef WIN32
	// On Win32 platforms, the show() routine causes the window to shrink. Reset it if fullscreen.
	if (Fullscreen)
	{
		int sx, sy, sw, sh;
		Fl::screen_xywh(sx, sy, sw, sh);
		MainWin->resize(sx, sy, sw, sh - hiddenTaskBarAdjust);
	}
#endif
	// Set the initial string for ReMem
	show_remem_mode();
	if (gReMem && !gRex)
	{
		// Check if ReMem is in "Normal" mode or MMU mode
		if (inport(REMEM_MODE_PORT) & 0x01)
		{
			// Read the MMU Map
			sprintf(temp, "Map:%d", (inport(REMEM_MODE_PORT) >> 3) & 0x07);
			display_map_mode(temp);
		}
		else
			display_map_mode((char *) "Normal");
	}
        

	/* Check for Fl_Window event */
    Fl::check();

	/* Check if an error occured prior to Window generation ... */
	/* ...and display it now if any */
	if (strlen(gDelayedError) != 0)
	{
		fl_alert("%s", gDelayedError);
		gDelayedError[0] = '\0';
	}

	gpPrint->label("Idle");

	/* 
	=====================================================================
	For Zipit Z2, create an array of x and y offsets plus pixel sizes for
	"stretched" display mode.
	=====================================================================
	*/
#ifdef ZIPIT_Z2
	int		size = 1;
	int		x, m, cur;

	m = 0;
	cur = 0;
	for (x = 0; x <= 240; x++)
	{
		gpDisp->m_xCoord[x] = cur;
		gpDisp->m_rectSize[x] = size;
		gpDisp->m_yCoord[x] = cur + gYoffset;
		cur += size;
		if (++m == 3)
		{
			size = 1;
			m = 0;
		}
		else if (m == 2)
			size = 2;
	}
#endif
}

/*
================================================================================
display_cpu_speed:	Updates the emulated CPU speed status box.
================================================================================
*/
static char	label[40];
static char mapStr[40];

void display_cpu_speed(void)
{
	// Test if we are in no GUI mode
	if (gNoGUI)
		return;

	// If speed is less than 10 Mhz, then show 2 decimal places, else 1
	if (cpu_speed + 0.5 < 10.0)
		sprintf(label, "%4.2f Mhz", cpu_speed);
	else
		sprintf(label, "%4.1f Mhz", cpu_speed + .05);
	Fl::check();
	gpSpeed->label(label);
}

void display_map_mode(char *str)
{
	if (gpMap == NULL || gNoGUI)
		return;

	strcpy(mapStr, str);
	Fl::check();
	gpMap->label(mapStr);
}

void drawbyte(int driver, int column, int value)
{
	if (gNoGUI)
		return;
	if (gpDisp != NULL)
		gpDisp->SetByte(driver, column, value);
	if (gpDebugMonitor != 0)
		gpDebugMonitor->SetByte(driver, column, value);

	return;
}

void lcdcommand(int driver, int value)
{
	if (gNoGUI)
		return;
	if (gpDisp != NULL)
		gpDisp->Command(driver, value);
	if (gpDebugMonitor != 0)
		gpDebugMonitor->Command(driver, value);

	return;
}

void power_down()
{
	if (gNoGUI)
		return;
	if (gpDisp != NULL)
		gpDisp->PowerDown();
	if (gpDebugMonitor != 0)
		gpDebugMonitor->PowerDown();

	return;
}

void process_windows_event()
{
	if (gOsDelay)
#ifdef WIN32
		Fl::wait(0.001);
//#elif defined(__APPLE__)
//        Fl::check();
#else
		Fl::wait(0.00001);
#endif
	else
        Fl::check();

	return;
}

// Called when the Model T shuts its power off
void T100_Disp::PowerDown()
{
	window()->make_current();
	// Clear display
    fl_color(m_BackgroundColor);
	if (DisplayMode == 1)
	    fl_rectf(45,25+30,w()-90,h()-49);
	else
	    fl_rectf(x(),y(),w(),h());

	//Fl::wait(0);
        Fl::check();
        
	// Print power down message
	char *msg = (char *) "System Powered Down";
	char *msg2 = (char *) "Press any key to reset";
	int x;
	int col;
	int driver, column;
	int addr;

	addr = gStdRomDesc->sCharTable;

	for (driver = 0; driver < 10; driver++)
		for (col = 0; col < 256; col++)
			lcd[driver][col] = 0;

	// Display first line of powerdown message
    fl_color(m_PixelColor);
	col = 20 - strlen(msg) / 2;
	for (x = 0; x < (int) strlen(msg); x++)
	{
		int mem_index = addr + (msg[x] - ' ')*5;
		column = col++ * 6;
		for (int c = 0; c < 6; c++)
		{
			driver = column / 50;		// 50 pixels per driver
			if (c == 5)
				SetByte(driver, (column % 50) | 0xC0, 0);
			else
				SetByte(driver, (column % 50) | 0xC0, gSysROM[mem_index++]);
			column++;
		}
	}

	// Display 2nd line of powerdown message
	col = 20 - strlen(msg2) / 2;
	for (x = 0; x < (int) strlen(msg2); x++)
	{
		int mem_index = addr + (msg2[x] - ' ')*5;
		column = col++ * 6;
		for (int c = 0; c < 6; c++)
		{
			driver = column / 50 + 5;		// 50 pixels per driver
			if (c == 5)
				SetByte(driver, (column % 50), 0);
			else
				SetByte(driver, (column % 50), gSysROM[mem_index++]);
			column++;
		}
	}
}

void simulate_keydown(int key)
{
	if (gpDisp == NULL)
		return;

	gpDisp->SimulateKeydown(key);
}

void simulate_keyup(int key)
{
	if (gpDisp == NULL)
		return;

	gpDisp->SimulateKeyup(key);
}

void handle_simkey(void)
{
	if (gpDisp == NULL)
		return;

	if (gSimKey)
		gpDisp->HandleSimkey();
}

void do_wheel_key(void*)
{
	if (gpDisp == NULL)
		return;

	if (gSimKey)
		gpDisp->HandleSimkey();
}

void T100_Disp::HandleSimkey(void)
{
	int	simkey = gSimKey;
	gSimKey = 0;
	handle(simkey);
}

/*
==========================================================================
siulate_keydown:	Simulates an FL_KEYDOWN event
==========================================================================
*/
void T100_Disp::SimulateKeydown(int key)
{
	int 	c;

	// Loop through array and check if key is already down
	for (c = 0; c < 32; c++)
	{
		if (m_simKeys[c] == key)
			break;
	}

	// Test if key needs to be added to simKeys
	if (c == 32)
	{
		// Find first non-zero entry
		for (c = 0; c < 32; c++)
			if (m_simKeys[c] == 0)
				break;
		if (c != 32)
			m_simKeys[c] = key;
	}
	m_simEventKey = key;
	gSimKey = VT_SIM_KEYDOWN;
	gDelayUpdateKeys = 1;
}

/*
==========================================================================
siulate_keyup:	Simulates an FL_KEYUP event
==========================================================================
*/
void T100_Disp::SimulateKeyup(int key)
{
	int  c;

	// Loop through array and check if key is already down
	for (c = 0; c < 32; c++)
	{
		if (m_simKeys[c] == key)
		{
			m_simKeys[c] = 0;
			break;
		}
	}

	m_simEventKey = key;
	gSimKey = VT_SIM_KEYUP;
	gDelayUpdateKeys = 1;
}

/*
==========================================================================
sim_get_key:	Returns the key that caused the event
==========================================================================
*/
int T100_Disp::sim_get_key(int key)
{
	int  c;

	// Loop through array and check if key is already down
	for (c = 0; c < 32; c++)
	{
		if (m_simKeys[c] == key)
			return TRUE;
	}

	return FALSE;
}

/*
==========================================================================
sim_event_key:	Returns the state of a simulated key
==========================================================================
*/
int T100_Disp::sim_event_key(void)
{
	return m_simEventKey;
}

/*
==========================================================================
Handle simulatged wheel keys (Function buttons, Enter, etc.)
==========================================================================
*/
extern "C"
void handle_wheel_keys(void*)
{

	// If no keys to process, just exit
	if (gpDisp->m_WheelKeyIn == gpDisp->m_WheelKeyOut)
		return;

	// If the key wasn't processed yet, just return
	if (gDelayUpdateKeys)
	{
//		Fl::repeat_timeout(0.01, cb_mousewheel, key);
		return;
	}

	int		which_key = gpDisp->m_WheelKeys[gpDisp->m_WheelKeyOut];
	if ((gpDisp->m_WheelKeyDown == VT_SIM_KEYDOWN) && (which_key != FL_Control_L))
	{
		// Handle CTRL-key sequences
		if (gpDisp->m_SimulatedCtrl)
		{
			gpDisp->SimulateKeyup(FL_Control_L);
			gpDisp->m_SimulatedCtrl = FALSE;
			gpDisp->HandleSimkey();
		}
		gpDisp->SimulateKeyup(which_key);
		gpDisp->m_WheelKeyDown = VT_SIM_KEYUP;
		gpDisp->HandleSimkey();
//		Fl::repeat_timeout(0.01, cb_mousewheel, (void *)VT_SIM_KEYUP);
	}
	else
	{
		if (++gpDisp->m_WheelKeyOut >= 32)
			gpDisp->m_WheelKeyOut = 0;
		if (gpDisp->m_WheelKeyOut != gpDisp->m_WheelKeyIn)
		{
			gpDisp->SimulateKeydown(gpDisp->m_WheelKeys[gpDisp->m_WheelKeyOut]);
			gpDisp->m_WheelKeyDown = VT_SIM_KEYDOWN;
			gpDisp->HandleSimkey();
//			Fl::repeat_timeout(0.0, do_wheel_key, (void *)VT_SIM_KEYDOWN);
		}
	}
}

/*
==========================================================================
Simulate a keystroke as a result of Mouse events (Mouse wheel, clicks, etc)
==========================================================================
*/
void T100_Disp::WheelKey(int key)
{
	int	startTimer = m_WheelKeyIn == m_WheelKeyOut;

	// Add the wheel key event
	m_WheelKeys[m_WheelKeyIn++] = key;
	if (m_WheelKeyIn >= 32)
		m_WheelKeyIn = 0;

	if (startTimer)
	{
		SimulateKeydown(key);
		m_WheelKeyDown = VT_SIM_KEYDOWN;
		handle(VT_SIM_KEYDOWN);
//		Fl::add_timeout(0.01, cb_mousewheel, (void *)VT_SIM_KEYDOWN);
	}
}

/*
==========================================================================
Test if the emulation is currently in the MENU program or TS-DOS menu
==========================================================================
*/
int T100_Disp::IsInMenu(void)
{
	int				lines, c, first_line = 0;
	unsigned short	lcdAddr, curAddr;
	char			lcdStr[21];

	// Validate the gStdRomDesc is valid
	if (gStdRomDesc == NULL)
		return FALSE;

	lines = gModel == MODEL_T200 ? 16 : 8;
	
	// For Model 200, the 1st byte of the LCD character buffer isn't
	// always the first row on the LCD.  There is a 1st row offset
	// variable at FEAEh that tells which row is the 1st row
	if (gModel == MODEL_T200)
		first_line = get_memory8(0xFEAE);
	lcdAddr = gStdRomDesc->sLcdBuf;

	// Now loop through each row and build a string to send
	curAddr = lcdAddr + first_line * 40;

	/* Test for TS-DOS menu */
	/* Get first 20 bytes from LCD to test for " BASIC    TEXT    " */
	for (c = 0; c < 9; c++)
		lcdStr[c] = get_memory8(curAddr++);
	lcdStr[c] = 0;

	if (strcmp(" TS-DOS (", lcdStr) == 0)
		return 2;

	/* Skip to second line to test for main menu */
	curAddr = lcdAddr + first_line * 40 + 40;	
	unsigned short maxAddr = lcdAddr + lines * 40;
	if (curAddr >= maxAddr)
		curAddr = lcdAddr;

	/* Get first 20 bytes from LCD to test for " BASIC    TEXT    " */
	for (c = 0; c < 20; c++)
		lcdStr[c] = get_memory8(curAddr++);
	lcdStr[c] = 0;

	if (strcmp(" BASIC     TEXT     ", lcdStr) == 0)
		return 1;

	return FALSE;
}

/*
==========================================================================
Locate the specified address entry in the StdRom descriptor
==========================================================================
*/
unsigned short find_stdrom_addr(int entry)
{
	int	c;

	for (c = 0; ;c++)
	{
		// Test for end of table
		if (gStdRomDesc->pVars[c].strnum == -1)
		{
			return 0;
		}

		// Search for the Level 6 plotting function
		if (gStdRomDesc->pVars[c].strnum == entry)
		{
			return gStdRomDesc->pVars[c].addr;
		}
	}

	return 0;
}

/*
==========================================================================
Test if the emulation is currently in the TEXT program 
==========================================================================
*/
int T100_Disp::IsInText(void)
{
	int		fkeyAddr, c, x, i;
	char	fkeyStr[41];

	// Find the adress of the FKey label buffer
	fkeyAddr = find_stdrom_addr(R_FKEY_DEF_BUF);

	// Read the Fkey labels
	for (c = 0, i = 0; i < 8; i++)
	{
		for (x = 0; x < 4; x++)
		{
			fkeyStr[c] = get_memory8(fkeyAddr++);
			if (fkeyStr[c] < 'A')
				fkeyStr[c] = ' ';
			c++;
		}
		fkeyStr[c++] = ' ';
		fkeyAddr += 12;
	}

	// Test for TEXT "FIND" label
	if (strncmp(fkeyStr, "Find Load", 9) == 0)
		return 1;
	if (strncmp(fkeyStr, "Find           Edit", 19) == 0)
		return 1;
	if (strncmp(fkeyStr, "Find Next", 9) == 0)
		return 1;
	return 0;
}

/*
==========================================================================
Waits for the selection to be complete and pops up the action menu
==========================================================================
*/
void cb_await_selection_complete(void *pContext)
{
	if (!gpDisp->m_SelectComplete)
	{
		Fl::repeat_timeout(0.02, cb_await_selection_complete, NULL);
		return;
	}

	gpDisp->m_CopyCut->menu(gCopyCutMenu);	
	gpDisp->m_CopyCut->type(Fl_Menu_Button::POPUP123);
	if (gpDisp->m_CopyCut->popup() == NULL)
		cb_select_cancel(gpDisp, NULL);
	gpDisp->m_CopyCut->type(0);
}

/*
==========================================================================
Adds a short delay after a selection to allow the display to update.
==========================================================================
*/
void cb_select_delay(void *pContext)
{
	gpDisp->m_SelectComplete = TRUE;
}

/*
==========================================================================
Process word selection logic after the timeout for display update
==========================================================================
*/
typedef struct
{
	unsigned short	addr;
	char			col;
	char			row;
	char			delay;
} WordSel_t;

void cb_wordsel(void *pContext)
{
	WordSel_t	*pWordSel = (WordSel_t *) pContext;

	if (gpDisp->m_WheelKeyIn != gpDisp->m_WheelKeyOut)
	{
		Fl::repeat_timeout(0.02, cb_wordsel, pContext);
		return;
	}

	if (pWordSel->delay)
	{
		Fl::repeat_timeout(0.2 / (fullspeed + 1), cb_wordsel, pContext);
		pWordSel->delay = 0;
		return;
	}
	// Set the new col based on if the column is zero or not
	if (pWordSel->col == 0)
	{
		set_memory8(pWordSel->addr, pWordSel->col + 1);
		gpDisp->WheelKey(FL_Left);
	}
	else
	{
		set_memory8(pWordSel->addr, pWordSel->col - 1);
		gpDisp->WheelKey(FL_Right);
	}

	Fl::add_timeout(0.2 / (fullspeed + 1), cb_select_delay);
	delete (char *) pContext;
}

void cb_dragsel(void *pContext)
{
	unsigned short	cursorRow;
	WordSel_t	*pWordSel = (WordSel_t *) pContext;

	// Test if this dragsel object is still valid.  If a later dragsel was queued, then
	// The position won't match any longer
	if (pWordSel->row * 40 + pWordSel->col != gpDisp->m_LastPos)
	{
		delete pWordSel;
		return;
	}

	// Test if the simulated keystroke buffer is empty
	if (gpDisp->m_WheelKeyIn != gpDisp->m_WheelKeyOut)
	{
		Fl::repeat_timeout(0.02, cb_dragsel, pContext);
		return;
	}

	// Delay to allow time for the emulation to perform highlighting
	if (pWordSel->delay)
	{
		Fl::repeat_timeout(0.2 / (fullspeed + 1), cb_dragsel, pContext);
		pWordSel->delay = 0;
		return;
	}

	int lines = gModel == MODEL_T200 ? 16 : 8;
	cursorRow = find_stdrom_addr(R_CURSOR_ROW);
	int row = pWordSel->row;
	if (row < 1)
		row = 1;
	if (row > lines)
		row = lines;
	set_memory8(cursorRow, row);
 
	// Set the new col based on if the column is zero or not
	if (pWordSel->col == 0)
	{
		set_memory8(pWordSel->addr, pWordSel->col + 1);
		gpDisp->WheelKey(FL_Left);
	}
	else
	{
		set_memory8(pWordSel->addr, pWordSel->col - 1);
		gpDisp->WheelKey(FL_Right);
	}

	// Test for scroll up
	if (pWordSel->row < 1)
		gpDisp->WheelKey(FL_Up);
	if (pWordSel->row > lines)
		gpDisp->WheelKey(FL_Down);

	Fl::add_timeout(0.2 / (fullspeed + 1), cb_select_delay);
	delete (char *) pContext;
}

/*
==========================================================================
Process mouse click events while the MsPlan ROM is active
==========================================================================
*/
int T100_Disp::ButtonClickInText(int mx, int my)
{
	int		cursorRow, cursorCol;
	int		newRow, newCol;

	// Find address of cursor row and col
	cursorRow = find_stdrom_addr(R_CURSOR_ROW);
	cursorCol = find_stdrom_addr(R_CURSOR_COL);

	// Calculate new row and col 
	newCol = (mx - gXoffset) / MultFact / 6 + 1;
	newRow = (my - gYoffset) / MultFact / 8 + 1;
	m_LastPos = newRow * 40 + newCol;

	// Set the new row
	set_memory8(cursorRow, newRow);

	if (Fl::event_clicks() == 0)
	{
		m_Select = FALSE;
		// Set the new col based on if the column is zero or not
		if (newCol == 0)
		{
			set_memory8(cursorCol, newCol + 1);
			WheelKey(FL_Left);
		}
		else
		{
			set_memory8(cursorCol, newCol - 1);
			WheelKey(FL_Right);
		}
	}
	else
	{
		// Double click in a TEXT document.  Select the word
		Fl::event_clicks(0);
		m_Select = 2;

		// Get the address of the LCD buffer
		int				lines, first_line = 0;
		unsigned short	lcdAddr, curAddr;

		lines = gModel == MODEL_T200 ? 16 : 8;
		
		// For Model 200, the 1st byte of the LCD character buffer isn't
		// always the first row on the LCD.  There is a 1st row offset
		// variable at FEAEh that tells which row is the 1st row
		if (gModel == MODEL_T200)
			first_line = get_memory8(0xFEAE);
		lcdAddr = gStdRomDesc->sLcdBuf;
		unsigned short maxAddr = lcdAddr + lines * 40;
		curAddr = lcdAddr + first_line * 40;
		curAddr += 40 * (newRow-1) + newCol - 1;
		if (curAddr >= maxAddr)
			curAddr -= lines * 40;

		// Read the character under the cursor
		char	ch = get_memory8(curAddr);
		
		// Read backward to find the first character of the word / whitespace
		while (newCol > 1)
		{
			newCol--;
			curAddr--;
			char nextCh = get_memory8(curAddr);
			if (ch == ' ')
			{
				// Group all whitespace togeher
				if (nextCh != ' ')
				{
					newCol++;
					curAddr++;
					break;
				}
			}
			else if (((ch < 'A') || (ch > 'z')) && (ch != '_'))
			{
				// Group all control symbols together
				if ((nextCh == ' ') || ((nextCh >= 'A') && (nextCh <= 'z')))
				{
					newCol++;
					curAddr++;
					break;
				}
			}
			else
			{
				if (((nextCh < 'A') || (nextCh > 'z')) && (nextCh != '_'))
				{
					newCol++;
					curAddr++;
					break;
				}
			}
		}
		if (newCol == 0)
		{
			set_memory8(cursorCol, newCol + 1);
			WheelKey(FL_Left);
		}
		else
		{
			set_memory8(cursorCol, newCol - 1);
			WheelKey(FL_Right);
		}

		// Now send the key for 'Sel'
		WheelKey(FL_F + 7);

		// Now find the end of the word
		while (newCol <= 40)
		{
			newCol++;
			curAddr++;
			char nextCh = get_memory8(curAddr);
//			WheelKey(FL_Right);
			if (ch == ' ')
			{
				// Group all whitespace togeher
				if (nextCh != ' ')
					break;
			}
			else if (((ch < 'A') || (ch > 'z')) && (ch != '_'))
			{
				// Group all control symbols together
				if ((nextCh == ' ') || ((nextCh >= 'A') && (nextCh <= 'z')))
					break;
			}
			else
			{
				if (((nextCh < 'A') || (nextCh > 'z')) && (nextCh != '_'))
					break;
			}
		}
		WordSel_t* pSelWord = new WordSel_t;
		pSelWord->addr = cursorCol;
		pSelWord->col = newCol;
		pSelWord->row = newRow;
		pSelWord->delay = 1;
		m_SelectComplete = FALSE;
		Fl::add_timeout(0.08, cb_wordsel, pSelWord);
	}
	return 1;
}

/*
==========================================================================
Process mouse click events while the MsPlan ROM is active
==========================================================================
*/
int T100_Disp::ButtonClickInMsPlan(int mx, int my)
{
	int				lines, first_line = 0;
	unsigned short	lcdAddr, curAddr;

	lines = gModel == MODEL_T200 ? 16 : 8;
	
	// For Model 200, the 1st byte of the LCD character buffer isn't
	// always the first row on the LCD.  There is a 1st row offset
	// variable at FEAEh that tells which row is the 1st row
	if (gModel == MODEL_T200)
		first_line = get_memory8(0xFEAE);
	lcdAddr = gStdRomDesc->sLcdBuf;

	// Now loop through each row and build a string to send
	curAddr = lcdAddr + first_line * 40;

	// Point to last line
	curAddr += 40 * (lines - 1);
	unsigned short maxAddr = lcdAddr + lines * 40;
	if (curAddr >= maxAddr)
		curAddr -= lines * 40;

	// Point to 2nd column to test for digit
	curAddr++;
	int ch = get_memory8(curAddr);
	int labelActive = TRUE;
	if ((ch >= '0') && (ch <= '9'))
		labelActive = FALSE;

	// Now perform click processing base on labelActive
	if (labelActive && (my > gYoffset + (lines - 1) * 8 * MultFact))
	{
		int num_labels = gModel == MODEL_PC8201 ? 5 : 8;
		int pixPerLabel = 240 / num_labels;
		int fk = (mx - gXoffset) / MultFact / pixPerLabel + 1;
		WheelKey(FL_F + fk);
		Fl::event_clicks(0);
	}
	else
	{
		int topCell = get_memory8(0xEB58);
		int leftCell = get_memory8(0xEB5A);
		int curCellX = get_memory8(0xE910);
		int curCellY = get_memory8(0xE90E);
		int cellWidth = get_memory8(0xE920);
		int charX = (mx - gXoffset) / MultFact / 6;
		int charY = (my- gYoffset) / MultFact / 8;

		if (cellWidth == 0)
			cellWidth = 9;
		int targetCellX = leftCell + (charX - 3) / cellWidth;
		int targetCellY = topCell + charY - 1;
		while (targetCellX > curCellX)
		{
			WheelKey(FL_Right);
			curCellX++;
		}
		while (targetCellX < curCellX)
		{
			WheelKey(FL_Left);
			curCellX--;
		}
		while (targetCellY > curCellY)
		{
			WheelKey(FL_Down);
			curCellY++;
		}
		while (targetCellY < curCellY)
		{
			WheelKey(FL_Up);
			curCellY--;
		}
	}

	return 1;
}

/*
==========================================================================
Process mouse click events while the MENU program is active
==========================================================================
*/
int T100_Disp::ButtonClickInMenu(int mx, int my)
{
	int lineHeight = MultFact * 8;
	int charWidth = MultFact * 6;
	int lines = gModel == MODEL_T200 ? 16 : 8;

	// Determine which menu item was selected
	if ((my > gYoffset + lineHeight) && (my <= gYoffset + lineHeight * (lines - 2)))
	{
		int row = (my - gYoffset - lineHeight) / lineHeight;
		int col = (mx - gXoffset) / (charWidth * 10);
		int entry = row * 4 + col;

		// Test if this entry is larger than
		int maxEntryAddr = find_stdrom_addr(R_MAX_MENU_DIR_LOC);
		int curEntryAddr = find_stdrom_addr(R_CUR_MENU_DIR_LOC);
		if ((maxEntryAddr != 0) && (curEntryAddr != 0))
		{
			// Read the max and current menu entries
			int maxEntry = get_memory8(maxEntryAddr);
			int curEntry = get_memory8(curEntryAddr);

			// Test if clicked on current item and start drag-n-drop
			if (curEntry == entry)
			{
//				Fl::copy("c:\test.do", 11, 1);
//				Fl::dnd();
			}

			// Test if entry is a valid file
			else if (entry <= maxEntry)
			{
				if (m_WheelKeyIn != m_WheelKeyOut)
				{
					m_WheelKeyIn = m_WheelKeyOut;
					if (m_WheelKeyIn >= 32)
						m_WheelKeyIn = 0;
				}
				// Send keystrokes to get to the entry
				int curRow = curEntry / 4;
				int curCol = curEntry - curRow * 4;
				while (row < curRow)
				{
					WheelKey(FL_Up);
					curRow--;
				}
				while (col < curCol)
				{
					WheelKey(FL_Left);
					curCol--;
				}
				while (row > curRow)
				{
					WheelKey(FL_Down);
					curRow++;
				}
				while (col > curCol)
				{
					WheelKey(FL_Right);
					curCol++;
				}
			}
		}
	}

	return 1;
}

/*
==========================================================================
Handle the mouse movement while emulation is in TEXT program.  Performs
drag selection and word drag selection.
==========================================================================
*/
int T100_Disp::MouseMoveInText(int mx, int my)
{
	int		cursorRow, cursorCol;
	int		newCol, newRow, pos;

	// Calculate new row and col 
	newCol = (mx - gXoffset) / MultFact / 6 + 1;
	newRow = (my - gYoffset) / MultFact / 8 + 1;
	if (newCol < 1)
		newCol = 1;
	if (newCol > 40)
		newCol = 40;

	pos = newRow * 40 + newCol;
	// Find address of cursor row and col
	cursorRow = find_stdrom_addr(R_CURSOR_ROW);
	cursorCol = find_stdrom_addr(R_CURSOR_COL);

	// Test if the cursor changed positions 
	if (pos != m_LastPos)
	{
		m_LastPos = pos;

		// If a selection wasn't started yet, then start it now
		if (!m_Select)
		{
			// Simulate an F7 keystroke to start selection
			WheelKey(FL_F + 7);
			m_Select = 1;
		}
		
		// Test if select mode is word select
		if (m_Select == 2)
		{
		}

		// Test if we are in word select mode and update new selection based on this
		WordSel_t* pSelWord = new WordSel_t;
		pSelWord->addr = cursorCol;
		pSelWord->col = newCol;
		pSelWord->row = newRow;
		pSelWord->delay = 1;
		m_SelectComplete = FALSE;
		Fl::add_timeout(0.04, cb_dragsel, pSelWord);
	}

	return 1;
}

/*
==========================================================================
Window handler for all events.
==========================================================================
*/
// Handle mouse events, key events, focus events, etc.
char	keylabel[128];
int T100_Disp::handle(int event)
{
	char	keystr[10];
	char	isSpecialKey = 1;
	int		c;
	int		simulated;
	get_key_t	get_key;
	event_key_t	event_key;

	unsigned int key;

	get_key = Fl::get_key;
	event_key = Fl::event_key;
	simulated = FALSE;

	// Test for simulated key events
	if ((event == VT_SIM_KEYUP) || (event == VT_SIM_KEYDOWN))
	{
		get_key = sim_get_key;
		event_key = sim_event_key;
		simulated = TRUE;

		if (event == VT_SIM_KEYUP)
			event = FL_KEYUP;
		else
			event = FL_KEYDOWN;
	}

	// Test for Mouse Wheel events
	if (event == FL_MOUSEWHEEL)
	{
		int x = Fl::e_dx;
		int y = Fl::e_dy;

		// Test for up arrow
		if (((gSpecialKeys & (MT_GRAPH | MT_CODE)) != (MT_GRAPH | MT_CODE)) || IsInMenu())
		{
			if (y < 0)
				WheelKey(FL_Left);
			else if (y > 0)
				// Must be down arrow
				WheelKey(FL_Right);
		}
		else
		{
			if (y < 0)
				WheelKey(FL_Up);
			else if (y > 0)
				// Must be down arrow
				WheelKey(FL_Down);
		}
		if (x < 0)
			WheelKey(FL_Left);
		else if (x > 0)
			WheelKey(FL_Right);
		return 1;
	}

	switch (event)
	{
	case FL_DND_DRAG:
	case FL_DND_ENTER:
	case FL_DND_RELEASE:
		return 1;
	case FL_PASTE:
		if (IsInMenu() == 1)
			remote_load_from_host(Fl::event_text());
		return 1;
	case FL_FOCUS:
		m_MyFocus = 1;
		break;

	case FL_ENTER:
	case FL_LEAVE:
		return 1;

	case FL_MOVE:
		return 1;

	case FL_DRAG:
		if (m_HaveMouse)
		{
			int mx = Fl::event_x();
			int my = Fl::event_y();

			if (IsInText())
				MouseMoveInText(mx, my);

		}
		break;
		
	case FL_RELEASE:
		// If text was selected, then popup the menu for copy/cut
		if (m_Select)
		{
			if (m_WheelKeyIn == m_WheelKeyOut)
				Fl::add_timeout(0.02, cb_await_selection_complete, NULL);
		}

		if (!Fl::event_clicks())
			m_Select = FALSE;
		m_HaveMouse = FALSE;
		Fl::release();
		break;

	case FL_PUSH:
		Fl::grab();
		m_HaveMouse = TRUE;
		if (Fl::event_button3())
		{
            if (gModel == MODEL_M100 && gQuad)
            {
                m_LeftClickQuad->type(Fl_Menu_Button::POPUP123);
                m_LeftClickQuad->popup();
                m_LeftClickQuad->type(0);
            }
            else
            {
                m_LeftClick->type(Fl_Menu_Button::POPUP123);
                m_LeftClick->popup();
                m_LeftClick->type(0);
            }
			take_focus();
			return 1;
		}

		if (m_MyFocus == 1)
		{
			int mx = Fl::event_x();
			int my = Fl::event_y();
			int procFkey = FALSE;
			int whichMenu;
			if ((mx >= m_BezelLeft  + m_BezelLeftW) && (mx <= m_BezelRight) &&
				(my >= m_BezelTop + m_BezelTopH) && (my <= m_BezelBottom))
			{
				if (Fl::event_clicks())
				{
					if (IsInMenu())
					{
						WheelKey(FL_Enter);
						Fl::event_clicks(0);
						m_Select = FALSE;
						return 1;
					}
				}
				// Test if we are in the menu
				if (whichMenu = IsInMenu())
				{
					m_Select = FALSE;
					// Test if mouse is in FKey area
					if (my >= m_BezelBottom - 1 - 8 * MultFact)
						procFkey = TRUE;
					else
					{
						if (whichMenu == 1)
						{
							return ButtonClickInMenu(mx, my);
						}
					}
				}
				else
				{
					if (gInMsPlanROM)
						return ButtonClickInMsPlan(mx, my);
					int labelEn = get_memory8(gStdRomDesc->sLabelEn);
					if ((gStdRomDesc->sLabelEn && labelEn) &&
						(my >= m_BezelBottom - 1 - 8 * MultFact))
					{
							procFkey = TRUE;
					}

					// Test if emulation is in Text mode
					else if (IsInText())
					{
						return ButtonClickInText(mx, my);
					}
				}
			}
			else if ((mx >= gXoffset) && (mx <= m_BezelRight) && 
				(my >= m_BezelBottom) && (my <= m_BezelBottom + m_BezelBottomH))
			{
				// In FKey area
				procFkey = TRUE;
			}

			if (procFkey)
			{
				int num_labels = gModel == MODEL_PC8201 ? 5 : 8;
				int pixPerLabel = 240 / num_labels;
				int fk = (mx - gXoffset) / MultFact / pixPerLabel + 1;
				WheelKey(FL_F + fk);
				m_Select = FALSE;
				Fl::event_clicks(0);
			}
		}
		m_MyFocus = 1;
		break;

	case FL_UNFOCUS:
		m_MyFocus = 0;
		gSpecialKeys = 0xFFFFFFFF;
		gpCode->deactivate();
		gpGraph->deactivate();
		gpKey->label("");
		for (c = 0; c < 128; c++)
			gKeyStates[c] = 0;
		update_keys();
		break;

	case FL_KEYUP:
		if (!m_MyFocus && !simulated)
			return 1;

		// Get the Key that was pressed
		//key = Fl::event_key();
		key = event_key();
		switch (key)
		{
		case FL_Escape:
			gSpecialKeys |= MT_ESC;
			break;
		case FL_Delete:
			if ((get_key(FL_Shift_L) | get_key(FL_Shift_R)) == 0)
			//if ((Fl::get_key(FL_Shift_L) | Fl::get_key(FL_Shift_R)) == 0)
				gSpecialKeys |= MT_SHIFT;
		case FL_BackSpace:
			gSpecialKeys |= MT_BKSP;
			break;
		case FL_Tab:
			gSpecialKeys |= MT_TAB;
			break;
		case FL_Enter:
			gSpecialKeys |= MT_ENTER;
			break;
		case FL_Print:
			gSpecialKeys |= MT_PRINT;
			break;
		case FL_F+12:
		case FL_Pause:
			gSpecialKeys |= MT_PAUSE;
			break;
		case FL_Left:
			gSpecialKeys |= MT_LEFT;
			break;
		case FL_Up:
			gSpecialKeys |= MT_UP;
			break;
		case FL_Right:
			gSpecialKeys |= MT_RIGHT;
			break;
		case FL_Down:
			gSpecialKeys |= MT_DOWN;
			break;
		case FL_Shift_L:
		case FL_Shift_R:
			gSpecialKeys |= MT_SHIFT;
			break;
		case FL_Control_L:
		case FL_Control_R:
			gSpecialKeys |= MT_CTRL;
			break;
//		case FL_Caps_Lock:
//			gSpecialKeys |= MT_CAP_LOCK;
//			break;
		case FL_Alt_L:
			gSpecialKeys |= MT_GRAPH;
			gpGraph->deactivate();
			break;
		case FL_Alt_R:
			gSpecialKeys |= MT_CODE;
			gpCode->deactivate();
			break;
		case FL_KP_Enter:
			gSpecialKeys |= MT_ENTER;
			break;
		case FL_Insert:
		case FL_Home:
			if ((get_key(FL_Control_L) | get_key(FL_Control_R)) == 0)
				gSpecialKeys |= MT_CTRL;
			if (get_key(FL_Left) == 0)
				gSpecialKeys |= MT_LEFT;
			break;
		case FL_End:
			if ((get_key(FL_Control_L) | get_key(FL_Control_R)) == 0)
				gSpecialKeys |= MT_CTRL;
			if (get_key(FL_Right) == 0)
				gSpecialKeys |= MT_RIGHT;
			break;
		case FL_Page_Up:
			if ((get_key(FL_Shift_L) | get_key(FL_Shift_R)) == 0)
				gSpecialKeys |= MT_SHIFT;
			if (get_key(FL_Up) == 0)
				gSpecialKeys |= MT_UP;
			break;
		case FL_Page_Down:
			if ((get_key(FL_Shift_L) | get_key(FL_Shift_R)) == 0)
				gSpecialKeys |= MT_SHIFT;
			if (get_key(FL_Down) == 0)
				gSpecialKeys |= MT_DOWN;
			break;
		case FL_F+1:
			gSpecialKeys |= MT_F1;
			break;
		case FL_F+2:
			gSpecialKeys |= MT_F2;
			break;
		case FL_F+3:
			gSpecialKeys |= MT_F3;
			break;
		case FL_F+4:
			gSpecialKeys |= MT_F4;
			break;
		case FL_F+5:
			gSpecialKeys |= MT_F5;
			break;
		case FL_F+6:
			gSpecialKeys |= MT_F6;
			break;
		case FL_F+7:
			gSpecialKeys |= MT_F7;
			break;
		case FL_F+8:
			gSpecialKeys |= MT_F8;
			break;
		case FL_F+9:
			gSpecialKeys |= MT_LABEL;
			break;
		case FL_F+10:
			gSpecialKeys |= MT_PRINT;
			break;
		case FL_F+11:
			gSpecialKeys |= MT_PASTE;
			break;
        case FL_Num_Lock:
            gSpecialKeys |= MT_NUM;
            break;
		default:
			key &= 0x7F;
                       // key=(unsigned int)Fl::event_text();
			// fprintf(stderr,"Et aussi %d\n",key);
			gKeyStates[key] = 0;
			if (key == 0x20)
				gSpecialKeys |= MT_SPACE;

			/*
			=========================================
			=========================================
			*/
			if (gModel == MODEL_PC8201 || gModel == MODEL_PC8300)
			{
				// Handle the '^' key (Shift 6)
				if (key == '6')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['6'] = 0;
						gKeyStates['@'] = 0;
					}
					else
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['@'] = 0;
					}
				}
				// Handle the '"' key (shift ')
				if (key == '\'')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['\''] = 0;
						gKeyStates['2'] = 0;
						gKeyStates['7'] = 0;
					}
					else
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['2'] = 0;
						gKeyStates['7'] = 0;
					}
				}
				// Handle the '+' and '=' keys
				if (key == '=')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates[';'] = 0;
						gKeyStates['-'] = 0;
					}
					else
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['-'] = 0;
						gKeyStates[';'] = 0;
					}
				}
				// Handle the '(' key
				if (key == '9')
				{
					gKeyStates['8'] = 0;
					gKeyStates['9'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the ')' key
				if (key == '0')
				{
					gKeyStates['9'] = 0;
					gKeyStates['0'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '_' key
				if (key == '-')
				{
					gKeyStates['-'] = 0;
					gKeyStates['0'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the ':' key
				if (key == ';')
				{
					gKeyStates[';'] = 0;
					gKeyStates[':'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '*' key
				if (key == '8')
				{
					gKeyStates['8'] = 0;
					gKeyStates[':'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '&' key
				if (key == '7')
				{
					gKeyStates['7'] = 0;
					gKeyStates['6'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '@' key
				if (key == '2')
				{
					gKeyStates['2'] = 0;
					gKeyStates['@'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
			}
			else if (gModel == MODEL_M10)
			{
				// Handle the '@' key
				if (key == '2')
				{
					gKeyStates['2'] = 0;
					gKeyStates['@'] = 0;
					gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '#' key
				if (key == '3')
				{
					gSpecialKeys |= MT_GRAPH;
					gKeyStates['h'] = 0;
				}
				// Handle the '^' key (Shift 6)
				if (key == '6')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['6'] = 0;
						gKeyStates['^'] = 0;
					}
					else
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['^'] = 0;
					}
				}
				if (key == '`')
				{
					gKeyStates['@'] = 0;
					gKeyStates['^'] = 0;
				}
				// Handle the '&' key
				if (key == '7')
				{
					gKeyStates['7'] = 0;
					gKeyStates['6'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '(' key
				if (key == '9')
				{
					gKeyStates['8'] = 0;
					gKeyStates['9'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the ')' key
				if (key == '0')
				{
					gKeyStates['9'] = 0;
					gKeyStates['0'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '*' key
				if (key == '8')
				{
					gKeyStates['8'] = 0;
					gKeyStates[':'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '_' key
				if (key == '-')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['-'] = 0;
						gKeyStates['0'] = 0;
						gSpecialKeys |= MT_SHIFT;
					}
				}
				if (key == '=')
				{
					gKeyStates[';'] = 0;
					gKeyStates['-'] = 0;
					gSpecialKeys |= MT_SHIFT;
				}
				// Handle the ':' key
				if (key == ';')
				{
					gKeyStates[';'] = 0;
					gKeyStates[':'] = 0;
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
						gSpecialKeys &= ~MT_SHIFT;
					else
						gSpecialKeys |= MT_SHIFT;
				}
				// Handle the '"' key (shift ')
				if (key == '\'')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['\''] = 0;
						gKeyStates['2'] = 0;
						gKeyStates['7'] = 0;
					}
					else
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['2'] = 0;
						gKeyStates['7'] = 0;
					}
				}
			}
			else
			{
				if (key == ']')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys |= MT_GRAPH;
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates[']'] = 0;
						gKeyStates['0'] = 0;
					}
					else
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['['] = 0;
					}
				}
				if (key == '[')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys |= MT_GRAPH;
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['['] = 0;
						gKeyStates['9'] = 0;
					}
				}
				if (key == '`')
				{
					gSpecialKeys |= MT_GRAPH;
					gKeyStates['['] = 0;
				}
				if (key == '\\')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys |= MT_GRAPH;
						gKeyStates[';'] = 0;
					}
					else
					{
						gSpecialKeys |= MT_GRAPH;
						gKeyStates['-'] = 0;
					}
				}
			}

			isSpecialKey = 0;
			break;
		}
		update_keys();
		break;

	case FL_KEYDOWN:
		if (!m_MyFocus && !simulated)
			return 1;

		if (ioBA & 0x10)
		{
			resetcpu();
			break;
		}

		// Get the Key that was pressed
		key = event_key();
		switch (key)
		{
		case FL_Escape:
			gSpecialKeys &= ~MT_ESC;
			break;
		case FL_Delete:
			gSpecialKeys &= ~MT_SHIFT;
		case FL_BackSpace:
			gSpecialKeys &= ~MT_BKSP;
			break;
		case FL_Tab:
			gSpecialKeys &= ~MT_TAB;
			break;
		case FL_Enter:
			gSpecialKeys &= ~MT_ENTER;
			break;
		case FL_Print:
			gSpecialKeys &= ~MT_PRINT;
			break;
		case FL_F+12:
		case FL_Pause:
			gSpecialKeys &= ~MT_PAUSE;
			break;
		case FL_Left:
			gSpecialKeys &= ~MT_LEFT;
			break;
		case FL_Up:
			gSpecialKeys &= ~MT_UP;
			break;
		case FL_Right:

			gSpecialKeys &= ~MT_RIGHT;
			break;
		case FL_Down:
			gSpecialKeys &= ~MT_DOWN;
			break;
		case FL_Shift_L:
		case FL_Shift_R:
			gSpecialKeys &= ~MT_SHIFT;
			break;
		case FL_Control_L:
		case FL_Control_R:
			gSpecialKeys &= ~MT_CTRL;
			break;
		case FL_Caps_Lock:
			gSpecialKeys ^= MT_CAP_LOCK;
			if (gSpecialKeys & MT_CAP_LOCK)
				gpCaps->deactivate();
			else
				gpCaps->activate();
			break;
		case FL_Alt_L:
			gSpecialKeys &= ~MT_GRAPH;
			gpGraph->activate();
			break;
		case FL_Alt_R:
			gSpecialKeys &= ~MT_CODE;
			gpCode->activate();
			break;
		case FL_KP_Enter:
			gSpecialKeys &= ~MT_ENTER;
			break;
		case FL_Insert:
		case FL_Home:
			gSpecialKeys &= ~(MT_CTRL | MT_LEFT);
			break;
		case FL_End:
			gSpecialKeys &= ~(MT_CTRL | MT_RIGHT);
			break;
		case FL_Page_Up:
			gSpecialKeys &= ~(MT_SHIFT | MT_UP);
			break;
		case FL_Page_Down:
			gSpecialKeys &= ~(MT_SHIFT | MT_DOWN);
			break;
		case FL_F+1:
			gSpecialKeys &= ~MT_F1;
			break;
		case FL_F+2:
			gSpecialKeys &= ~MT_F2;
			break;
		case FL_F+3:
			gSpecialKeys &= ~MT_F3;
			break;
		case FL_F+4:
			gSpecialKeys &= ~MT_F4;
			break;
		case FL_F+5:
			gSpecialKeys &= ~MT_F5;
			break;
		case FL_F+6:
			gSpecialKeys &= ~MT_F6;
			break;
		case FL_F+7:
			gSpecialKeys &= ~MT_F7;
			break;
		case FL_F+8:
			gSpecialKeys &= ~MT_F8;
			break;
		case FL_F+9:
			gSpecialKeys &= ~MT_LABEL;
			break;
		case FL_F+10:
			gSpecialKeys &= ~MT_PRINT;
			break;
		case FL_F+11:
			gSpecialKeys &= ~MT_PASTE;
			break;
         case FL_Num_Lock:
            gSpecialKeys &= ~MT_NUM;
            break;
		default:
			key &= 0x7F;
			gKeyStates[key] = 1;
			if (key == 0x20)
			{
				gSpecialKeys &= ~MT_SPACE;
			}
			if (gModel == MODEL_PC8201 || gModel == MODEL_PC8300)
			{
				// Deal with special keys for the PC-8201

				// Handle the '^' key  (Shift 6)
				if (key == '6')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['6'] = 0;
						gKeyStates['@'] = 1;
					}
					else
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['@'] = 0;
					}
				}
				// Handle the '"' key  (Shift ')
				if (key == '\'')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['\''] = 0;
						gKeyStates['2'] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['2'] = 0;
						gKeyStates['7'] = 1;
					}
				}
				// Handle the '+' and '=' keys
				if (key == '=')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['='] = 0;
						gKeyStates[';'] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['-'] = 1;
						gKeyStates['='] = 0;
					}
				}
				// Handle the '(' key
				if (key == '9')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['9'] = 0;
						gKeyStates['8'] = 1;
					}
				}
				// Handle the '(' key
				if (key == '0')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['0'] = 0;
						gKeyStates['9'] = 1;
					}
				}
				// Handle the '_' key
				if (key == '-')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['-'] = 0;
						gKeyStates['0'] = 1;
					}
				}
				// Handle the ':' key
				if (key == ';')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gSpecialKeys |= MT_SHIFT;
						gKeyStates[';'] = 0;
						gKeyStates[':'] = 1;
					}
				}
				// Handle the '*' key
				if (key == '8')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gKeyStates['8'] = 0;
						gKeyStates[':'] = 1;
					}
				}
				// Handle the '&' key
				if (key == '7')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gKeyStates['7'] = 0;
						gKeyStates['6'] = 1;
					}
				}
				// Handle the '@' key
				if (key == '2')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['2'] = 0;
						gKeyStates['@'] = 1;
					}
				}
			}
			else if (gModel == MODEL_M10)
			{
				// Handle the '@' key
				if (key == '2')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['2'] = 0;
						gKeyStates['@'] = 1;
					}
				}
				// Handle the '#' key
				if (key == '3')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gSpecialKeys &= ~MT_GRAPH;
						gKeyStates['3'] = 0;
						gKeyStates['h'] = 1;
					}
				}
				// Handle the '^' key  (Shift 6)
				if (key == '6')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['6'] = 0;
						gKeyStates['^'] = 1;
					}
					else
					{
						gKeyStates['^'] = 0;
					}
				}
				if (key == '`')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['`'] = 0;
						gKeyStates['^'] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['`'] = 0;
						gKeyStates['@'] = 1;
					}
				}
				// Handle the '&' key
				if (key == '7')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gKeyStates['7'] = 0;
						gKeyStates['6'] = 1;
					}
				}
				// Handle the '(' key
				if (key == '9')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['9'] = 0;
						gKeyStates['8'] = 1;
					}
				}
				// Handle the '(' key
				if (key == '0')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['0'] = 0;
						gKeyStates['9'] = 1;
					}
				}
				// Handle the '*' key
				if (key == '8')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gKeyStates['8'] = 0;
						gKeyStates[':'] = 1;
					}
				}
				// Handle the '_' key
				if (key == '-')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['-'] = 0;
						gKeyStates['0'] = 1;
					}
				}
				if (key == '=')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['='] = 0;
						gKeyStates[';'] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['='] = 0;
						gKeyStates['-'] = 1;
					}
				}
				// Handle the ':' key
				if (key == ';')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{				 
						gSpecialKeys |= MT_SHIFT;
						gKeyStates[';'] = 0;
						gKeyStates[':'] = 1;
					}
				}
				// Handle the '"' key  (Shift ')
				if (key == '\'')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gKeyStates['\''] = 0;
						gKeyStates['2'] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['2'] = 0;
						gKeyStates['7'] = 1;
					}
				}
			}
			else
			{
				if (key == ']')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_GRAPH;
						gSpecialKeys |= MT_SHIFT;
						gKeyStates[']'] = 0;
						gKeyStates['0'] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['['] = 1;
						gKeyStates[key] = 0;
					}
				}
				if (key == '[')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_GRAPH;
						gSpecialKeys |= MT_SHIFT;
						gKeyStates['['] = 0;
						gKeyStates['9'] = 1;
					}
				}
				if (key == '`')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_GRAPH;
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['`'] = 0;
						gKeyStates['['] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_GRAPH;
						gKeyStates['`'] = 0;
						gKeyStates['['] = 1;
					}
				}
				if (key == '\\')
				{
					if (get_key(FL_Shift_L) || get_key(FL_Shift_R))
					{
						gSpecialKeys &= ~MT_GRAPH;
						gSpecialKeys &= ~MT_SHIFT;
						gKeyStates['\\'] = 0;
						gKeyStates[';'] = 1;
					}
					else
					{
						gSpecialKeys &= ~MT_GRAPH;
						gKeyStates['-'] = 1;
						gKeyStates['\\'] = 0;
					}
				}
			}
			isSpecialKey = 0;
			break;
		}
		update_keys();
		break;

	default:
		Fl_Widget::handle(event);
		break;

	}

	// Display keystroke info on status line of display
	if (event == FL_KEYUP || event == FL_KEYDOWN)
	{
		// Start with empty string
		keylabel[0] = 0;

		// Add text for special keys
		for (c = 0; c < 32; c++)
		{
			if ((gSpecialKeys & (0x01 << c)) == 0)
			{
				if ((c == 2) || (c == 3) || (c==5))
					continue;
				if (keylabel[0] != 0)
					strcat(keylabel, "+");
				strcat(keylabel, gSpKeyText[c]);
			}
		}

		// Add text for regular keys
		for (c = ' '+1; c < '~'; c++)
		{
			if (gKeyStates[c])
			{
				if (keylabel[0] != 0)
					strcat(keylabel, "+");
				sprintf(keystr, "'%c'", c);

				// Space shows up better written instead of ' '
				if (c == 0x20)
					strcat(keylabel, "SPACE");
				else
					strcat(keylabel, keystr);
			}
		}

		// For some reason the F1 key causes garbage to print in
		// the status box.  This check fixed it for some reason -- ????
		Fl::check();

		// Update the text
		gpKey->label(keylabel);
	}

	return 1;
}

/*
==========================================================
t200_command:	This function processes commands sent to 
				Model 200 LCD subsystem.
==========================================================
*/
void t200_command(unsigned char ir, unsigned char data)
{
	if (gpDisp != NULL)
		gpDisp->Command(ir, data);
	if (gpDebugMonitor != 0)
		gpDebugMonitor->Command(ir, data);
}

/*
==========================================================
t200_readport:	This function returns the I/O port data
				for the port specified.
==========================================================
*/
unsigned char t200_readport(unsigned char port)
{
	if (gpDisp == NULL)
		return 0;
	return ((T200_Disp*)gpDisp)->ReadPort(port);
}

const T200_Disp& T200_Disp::operator=(const T200_Disp& srcDisp)
{
	int		c;

	// Copy the LCD data
	if (this != &srcDisp)
	{
		for (c = 0; c < 8192; c++)
			m_ram[c] = srcDisp.m_ram[c];

		// Copy control variables
		m_dstarth = srcDisp.m_dstarth;
		m_dstartl = srcDisp.m_dstartl;
		m_mcr = srcDisp.m_mcr;
		m_hpitch = srcDisp.m_hpitch;
		m_hcnt = srcDisp.m_hcnt;
		m_curspos = srcDisp.m_curspos;
		m_cursaddrh = srcDisp.m_cursaddrh;
		m_cursaddrl = srcDisp.m_cursaddrl;
		m_ramrd = srcDisp.m_ramrd;
		m_ramwr = srcDisp.m_ramwr;
		m_ramrd_addr = srcDisp.m_ramrd_addr;
		m_ramrd_dummy = srcDisp.m_ramrd_dummy;
		m_redraw_count = srcDisp.m_redraw_count;
		m_tdiv = srcDisp.m_tdiv;
		m_last_dstart = srcDisp.m_last_dstart;
	}

	return *this;
}

/*
=======================================================
T200:Disp:	This is the class construcor
=======================================================
*/
T200_Disp::T200_Disp(int x, int y, int w, int h) :
  T100_Disp(x, y, w, h)
{
	  int c;

	  /* Clear the display RAM */
	  for (c = 0; c < 8192; c++)
		  m_ram[c] = 0;

	  m_MyFocus = 0;
	  m_dstarth = 0;
	  m_dstartl = 0;
	  m_mcr = 0;
	  m_hpitch = 0;
	  m_hcnt = 0;
	  m_curspos = 0;
	  m_cursaddrh = 0;
	  m_cursaddrl = 0;
	  m_ramrd = 0;
	  m_ramrd_dummy = 0;
	  m_redraw_count = 0;
}

/*
==========================================================
Command:	This function processes commands sent to 
			Model 200 LCD subsystem.
==========================================================
*/
void T200_Disp::Command(int instruction, uchar data)
{
	int		cursaddr, dstart;
	char	mask;
	int		c;

	/* Calculate cursaddr just in case */
	cursaddr = (m_cursaddrh << 8) | m_cursaddrl;

	switch (instruction)
	{
	case SET_MCR:
		m_mcr = data;
		break;

	case SET_HPITCH:
		m_hpitch = data;
		break;

	case SET_HCNT:
		m_hcnt = data;
		break;

	case SET_TDIV:
		m_tdiv = data;
		break;

	case SET_CURSPOS:
		m_curspos = data;
		break;

	case SET_DSTARTL:
		/* Save new value for m_dstartl */
		m_dstartl = data;

		break;

	case SET_DSTARTH:
		/* Save new value of m_dstarth */
		m_dstarth = data;


		/* Calculate location of new page start */
		dstart = (m_dstarth << 8) | m_dstartl;

		/* Test if changing to a totally new page */
		c = abs(dstart - m_last_dstart);
		if (c != 0x140)
			c = dstart - m_last_dstart + 0x2000;
		if (c != 0x140)
			c = m_last_dstart - dstart + 0x2000;

		m_last_dstart = dstart;

		if (c != 0x140)
		{
			/* Clear all of memory to prevent screen "jump" */
			for (c = 0; c < 0x2000; c++)
				m_ram[c] = 0;
		}
		else
		{
			/* Clear RAM "above" display area */
			dstart -= 640;
			if (dstart < 0)
				dstart += 8192;
			for (c = 0; c < 640; c++)
			{
				m_ram[dstart++] = 0;
				if (dstart >= 8192)
					dstart = 0;
			}
		}

		m_redraw_count = 40 * 8;
//		printf("T200:IR=0x%04X  DATA=0x%02X\n", instruction, dstart);

		break;

	case SET_CURSADDRL:
		/* Test for increment of m_dstarth */
		if ((m_cursaddrl & 0x80) && !(data & 0x80))
		{
			m_cursaddrh++;

			/* Test if address >= 8192 */
			if (m_cursaddrh >= 0x20)
				m_cursaddrh = 0;
		}

		/* Save new value of cursor address */
		m_cursaddrl = data;
		break;

	case SET_CURSADDRH:
		/* Save new value of m_cursaddrh */
		m_cursaddrh = data;
		break;

	case RAM_WR:
		/* Draw new data on the LCD */
		SetByte(cursaddr, 0, data);

		/* Increment cursor address */
		if (++m_cursaddrl == 0)
			m_cursaddrh++;

		/* Test if address >= 8192 */
		if (m_cursaddrh >= 0x20)
			m_cursaddrh = 0;

		if (m_redraw_count > 0)
		{
			if (--m_redraw_count == 0)
				redraw_active();
		}
		break;

	case RAM_RD:
		/* Load m_ramrd register from the dummy register */
		m_ramrd_addr = cursaddr;

		break;

	case RAM_BIT_CLR:		/* Clear the bit specified by data at cursaddr */
		/* First calculate the AND mask */
		mask = 0xFE << data;

		/* Draw new data on the LCD */
		SetByte(cursaddr, 0, m_ram[cursaddr] & mask);

		/* Now increment the address */
		if (++m_cursaddrl == 0)
			m_cursaddrh++;
		/* Test if address >= 8192 */

		if (m_cursaddrh >= 0x20)
			m_cursaddrh = 0;

		break;

	case RAM_BIT_SET:		/* Set the bit specified by data at cursaddr */
		/* First calculate the OR mask */
		mask = 0x01 << data;

		/* Draw new data on the LCD */
		SetByte(cursaddr, 0, m_ram[cursaddr] | mask);

		/* Now increment the address */
		if (++m_cursaddrl == 0)
			m_cursaddrh++;

		/* Test if address >= 8192 */
		if (m_cursaddrh >= 0x20)
			m_cursaddrh = 0;

		break;

	}
}

/*
=================================================================
draw:	This routine draws the entire LCD.  This is a member 
		function of Fl_Window.
=================================================================
*/
void T200_Disp::draw()
{

	int		x=0;
	int		y=0;
	int		addr, col, row;
	uchar	value;

	/* Draw the static portions of the LCD / screen */
	draw_static();

	/* Get RAM address where display should start */
	addr = ((m_dstarth << 8) | m_dstartl) & (8192-1);

	fl_color(m_PixelColor);

    window()->make_current();

	/* Check if the driver is in "graphics" mode */
	if (m_mcr & 0x02)
	{
		/* Loop through all 128 display pixel rows */
		for (row = 0; row < 128; row++)
		{
			y = row;

			/* Loop through all 40 LCD columns */
			for (col = 0; col < 40; col++)
			{
				/* Calculate x coordinate (Characters are 6 pixels wide) */
				x= col * 6;


				/* Get data to be displayed on "LCD" */
				value = m_ram[addr++];
				if (addr >= 8192)
					addr = 0;

				// Erase line so it is grey, then fill in with black where needed
//				fl_color(FL_GRAY);
//				fl_rectf(x*MultFact + gXoffset,y*MultFact + 
//					gYoffset,8*MultFact,gRectsize);
//				fl_color(FL_BLACK);

				// Draw the black pixels
				drawpixel(x++,y, value&0x01);
				drawpixel(x++,y, value&0x02);
				drawpixel(x++,y, value&0x04);
				drawpixel(x++,y, value&0x08);
				drawpixel(x++,y, value&0x10);
				drawpixel(x++,y, value&0x20);
			}
		}
	}
}


void T200_Disp::SetByte(int driver, int col, uchar value)
{
	int x;
	int y;
	int addr = driver;
	int diff = addr - ((m_dstarth << 8) | m_dstartl);
	if (diff < 0)
		diff += 8192;

	// Check if LCD already has the value being requested
	if (m_ram[addr] == value)
		return;

	// Load new value into lcd "RAM"
	m_ram[addr] = value;

	// Calculate X position of byte */
	x = (diff % 40) * 6;

	// Calcluate y position of byte
	y = (diff / 40);
	if (y >= 128)
		return;

	// Set the display
	window()->make_current();

	fl_color(m_BackgroundColor);
#ifdef ZIPIT_Z2
	fl_rectf(m_xCoord[x], m_yCoord[y], 8, m_rectSize[y]);
#else
	fl_rectf(x*MultFact + gXoffset, y*MultFact + 
		gYoffset, MultFact*6, gRectsize);
#endif
	fl_color(m_PixelColor);

	// Draw each pixel of byte
	drawpixel(x++,y,value&0x01);
	drawpixel(x++,y,(value&0x02)>>1);
	drawpixel(x++,y,(value&0x04)>>2);
	drawpixel(x++,y,(value&0x08)>>3);
	drawpixel(x++,y,(value&0x10)>>4);
	drawpixel(x++,y,(value&0x20)>>5);
}

unsigned char T200_Disp::ReadPort(unsigned char port)
{
	/* Read RAM data through dummy register */
	m_ramrd = m_ramrd_dummy;
	
	/* Do a dummy read */
	m_ramrd_dummy = m_ram[m_ramrd_addr++];
	if (m_ramrd_addr >= 8192)
		m_ramrd_addr = 0;

	return m_ramrd;
}

/*
=================================================================
redraw_active:	This routine redraws only the active LCD pixel
				portion of the screen.  This prevents screen
				"flicker" that would be caused by erasing the
				entire background and redrawing the static data
				every time the screen scrolls.
=================================================================
*/
void T200_Disp::redraw_active()
{

	int		x=0;
	int		y=0;
	int		addr, col, row;
	uchar	value;

	// Set the display
	window()->make_current();

	/* Get RAM address where display should start */
	addr =( (m_dstarth << 8) | m_dstartl) &(8192-1);

	/* Check if the driver is in "graphics" mode */
	if (m_mcr & 0x02)
	{
		/* Loop through all 128 display pixel rows */
		for (row = 0; row < 128; row++)
		{
			// Erase line so it is grey, then fill in with black where needed
			y = row;
			fl_color(m_BackgroundColor);
#ifdef ZIPIT_Z2
			fl_rectf(m_xCoord[x],m_yCoord[y], 320,m_rectSize[y]);
#else
			fl_rectf(gXoffset,y*MultFact + 
				gYoffset,240*MultFact,gRectsize);
#endif
			fl_color(m_PixelColor);

			/* Loop through all 40 LCD columns */
			for (col = 0; col < 40; col++)
			{
				/* Calculate x coordinate (Characters are 6 pixels wide) */
				x= col * 6;

				/* Get data to be displayed on "LCD" */
				value = m_ram[addr++];
				if (addr >= 8192)
					addr = 0;

				// Draw the black pixels
				drawpixel(x++,y, value&0x01);
				drawpixel(x++,y, value&0x02);
				drawpixel(x++,y, value&0x04);
				drawpixel(x++,y, value&0x08);
				drawpixel(x++,y, value&0x10);
				drawpixel(x++,y, value&0x20);
			}
		}
	}
}

/*
=================================================================
Initialize (open) other windows that were previously opened the
last time the application closed.
=================================================================
*/
void init_other_windows(void)
{
	int		memedit_was_open, cpuregs_was_open, ide_was_open;

	// Test if we are in no GUI mode or not
	if (gNoGUI)
		return;

	/* Load the open state of various windows */
	virtualt_prefs.get("WindowState_MemEdit", memedit_was_open, 0);
	virtualt_prefs.get("WindowState_CpuRegs", cpuregs_was_open, 0);
	virtualt_prefs.get("WindowState_IDE", ide_was_open, 0);

	// If the Memory Editor was opened previously, then re-open it
	if (memedit_was_open && (gmew == NULL))
		cb_MemoryEditor(NULL, NULL);

	// If the CpuRegs window was opened previously, then re-open it
	if (cpuregs_was_open && (gcpuw == NULL))
		cb_CpuRegs(NULL, NULL);

	// If the IDE window was opened previously, then re-open it
	if (ide_was_open && gpIde == NULL)
		cb_Ide(NULL, NULL);

	// Give the focus back to the main VirtualT window
	MainWin->show();
}

