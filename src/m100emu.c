/* m100emu.c */

/* $Id: m100emu.c,v 1.41 2013/03/08 00:41:16 kpettit1 Exp $ */

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

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <mmsystem.h>
#endif

#ifdef __unix__ 
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#endif

#ifdef __APPLE__
#include <semaphore.h>
#endif

#include "VirtualT.h"
#include "io.h"
#include "cpu.h"
#include "doins.h"
#include "display.h"
#include "genwrap.h"
#include "filewrap.h"
#include "roms.h"
#include "intelhex.h"
#include "setup.h"
#include "memory.h"
#include "m100emu.h"
#include "sound.h"
#include "remote.h"
#include "serial.h"
#include "lpt.h"
#include "clock.h"
#include "fileview.h"

int				gModel = MODEL_M100;

volatile uchar	cpu[14];
extern uchar	*gMemory[64];
extern int		gRamBottom;
extern uchar	gMsplanROM[32768];
extern uchar	gOptROM[32768];
extern int		gInMsPlanROM;

mem_monitor_cb	gMemMonitor1 = NULL;
mem_monitor_cb	gMemMonitor2 = NULL;

char					op[26];
int						fullspeed = 0;
volatile UINT64			cycles=0;
volatile int			cycle_delta;
volatile UINT64			instructs = 0;
volatile UINT64			gThrottleCycles = 0;
DWORD					gThrottleDelta = 0;
unsigned int			gThrottlePeriod = 0;
int						gThrottlePeriodCount = 0;
int						gThrottleTerminalCount = 0;
DWORD					gTargetFrequency = 2457600;
unsigned int			gThrottleId = 0;

#ifdef WIN32
static HANDLE			gThrottleEvent;
static CRITICAL_SECTION	gThrottleCritSect;
#else
int                     gThrottleExit = 0;
pthread_t			    gThrottleThread;		// The remote control thread
pthread_mutex_t		    gThrottleLock;		    // Lock to access emulation
sem_t                   gThrottleEvent;         // Throttle event semaphore
#endif

double					last_instruct = 0;
static volatile UINT64	last_isr_cycle = 0;
int						trace=0;
int						starttime;
FILE					*tracefile;
volatile DWORD			rst7cycles = 9830;
static time_t			gLptTime;
static UINT64			one_sec_cycles;
static time_t			one_sec_time;
float					cpu_speed;
float					gCpuSpeedAvg[4];
int						gCpuSpeedAvgIndex = 0;
volatile int			gExitApp = 0;
volatile int			gExitLoop = 0;
char					gsOptRomFile[256];
int						gShowVersion = 0;
int						gMaintCount = 65536;
int						gOsDelay = 0;
int						gNoGUI = 0;
int						gRemoteSwitchModel = -1;

//Added J. VERNET
//char					path[512];
char					file[512];
#ifdef __APPLE__
char					gOsxBundlePath[512];
#endif

extern RomDescription_t		gM100_Desc;
extern RomDescription_t		gM200_Desc;
extern RomDescription_t		gN8201_Desc;
extern RomDescription_t		gM10_Desc;
extern RomDescription_t		gKC85_Desc;


extern uchar				gReMem;
extern int					cROM;
extern unsigned char		ioD0;
RomDescription_t			*gStdRomDesc = NULL;


/* Define Debug global variables */
char					gDebugActive = 0;
char					gDebugInts = TRUE;
char					gIntActive = 0;
unsigned short			gIntSP = 0;
char					gStopped = 0;
char					gSingleStep = 0;
char					gLastWasSingleStep = 0;
debug_monitor_callback	gpDebugMonitors[3] = { NULL, NULL, NULL };
void					periph_mon_update_lpt_log(void);


#ifdef WIN32
void CALLBACK ThrottleProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	/* Update the cycles counter */
	EnterCriticalSection(&gThrottleCritSect);
	
	/* Update the gThrotleCycles count to allow the emulation to run
	   a little during this timer tick.  We want to limit the amount
	   gThrottleCycels is allowed to race beyond cycles in case the
	   emulation is running slower than we think, like during window
	   redraws or menu events
    */
	if ((gThrottleCycles - (gThrottleDelta << 1) <= cycles) || (fullspeed == 3))
	{
		gThrottleCycles += gThrottleDelta;
	}
	gThrottlePeriodCount++;
	LeaveCriticalSection(&gThrottleCritSect);

	/* Now pulse the event to allow the emulation to run */
	PulseEvent(gThrottleEvent);
}

#else /* WIN32 */

void * ThrottlePeriodProc(void *pParams)
{
	int		lastThrottlePeriod = gThrottlePeriod;
    int     sleepUs = 1000 / gThrottlePeriod * 1000;

    /* Loop until time to exit the app */
    while (!gThrottleExit)
    {
        /* Update the gThrotleCycles count to allow the emulation to run
           a little during this timer tick.  We want to limit the amount
           gThrottleCycels is allowed to race beyond cycles in case the
           emulation is running slower than we think, like during window
           redraws or menu events
        */
        pthread_mutex_lock(&gThrottleLock);
        if ((gThrottleCycles - (gThrottleDelta << 1) <= cycles) || (fullspeed == 3))
        {
            gThrottleCycles += gThrottleDelta;
        }
        gThrottlePeriodCount++;
		if (lastThrottlePeriod != gThrottlePeriod)
		{
			sleepUs = 1000 / gThrottlePeriod * 1000;
			lastThrottlePeriod = gThrottlePeriod;
		}

        pthread_mutex_unlock(&gThrottleLock);

        /* Post the event semaphore to wake up the emulation thread */
        sem_post(&gThrottleEvent);

        /* Sleep the perscribed number of microseconds */
		if(fullspeed != 3) 
			usleep(sleepUs);
		else
			usleep(sleepUs * 2);
    }
	
	return NULL;
}

#endif

/*
=============================================================================
This routine intializes the system timer used for throttling the emulation
speed.
=============================================================================
*/
int init_throttle_timer(void)
{
#ifdef WIN32
	TIMECAPS		tc;

	/* Get the system's multimedia timer capabilties */
	timeGetDevCaps(&tc, sizeof(tc)); 

	/* On older systems, the minimum may be 55ms.  In this case, we just use 55ms as the tick */
	if (tc.wPeriodMin > 50)
	{
		gThrottlePeriod = tc.wPeriodMin;
		gThrottleTerminalCount = 1000 / gThrottlePeriod;
	}
	else if (tc.wPeriodMin <= 10)
	{
		/* We don't need 10ms of resolution.  Let's use 20ms instead */
		gThrottlePeriod = 50;

		/* Set TC for 50 counts = 1sec */
		gThrottleTerminalCount = 20;
	}

	/* Setup the throttle event for signalling */
	gThrottleEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&gThrottleCritSect);

	/* Calculate the period, Throttle Delta time and setup the timer */
	gThrottleDelta = gTargetFrequency * gThrottlePeriod / 1000;
	if (timeBeginPeriod(gThrottlePeriod) == TIMERR_NOCANDO)
		return FALSE;
	gThrottleId = timeSetEvent(gThrottlePeriod, tc.wPeriodMin, ThrottleProc, 
		0, TIME_PERIODIC ); 
#else

    /* We don't need 10ms of resolution.  Let's use 20ms instead */
    gThrottlePeriod = 40;

    /* Set TC for 50 counts = 1sec */
    gThrottleTerminalCount = 25;
    gThrottlePeriodCount = 0;
	gThrottleDelta = gTargetFrequency * gThrottlePeriod / 1000;
	gThrottleCycles = cycles + (gThrottleDelta << 1);

    // Initialize thread processing
    pthread_mutex_init(&gThrottleLock, NULL);
    pthread_create(&gThrottleThread, NULL, ThrottlePeriodProc, NULL);
    sem_init(&gThrottleEvent, 0, 0);

#endif

	return 1;
}

void set_target_frequency(int frequency)
{
	gTargetFrequency = frequency;
	gThrottleDelta = gTargetFrequency * gThrottlePeriod / 1000;
	gThrottleCycles = cycles + gThrottleDelta;
}

/*
=============================================================================
This routine de-intializes the system timer used for throttling the emulation
speed.
=============================================================================
*/
void deinit_throttle_timer(void)
{
#ifdef WIN32
	/* Kill the timer event */
	if (gThrottleId != 0)
		timeKillEvent(gThrottleId);

	/* End the timer period */
	timeEndPeriod(gThrottlePeriod);

	/* Destory the triggering event */
	CloseHandle(gThrottleEvent);
#else
    /* Instruct the throttle thread to terminate */
    gThrottleExit = 1;

    /* Join our throttle thread to ensure a clean shutdown */
    pthread_join(gThrottleThread, NULL);
#endif
}

/*
=============================================================================
This routine "throttles" the emulation speed based on the instruction cycle
count.  This is used for 2.4 Mhz emulation speed.
=============================================================================
*/
void throttle(int cy)
{
    int     sleepUs = 1000 / gThrottlePeriod * 1000;

	/* Update cycles counter */
	cycles+=cy;
	cycle_delta=0;

	if(fullspeed != 3) 
	{
#ifdef WIN32
		if (cycles >= gThrottleCycles)
		{
//			process_windows_event();
			WaitForSingleObject(gThrottleEvent, INFINITE);
		}
#else
		while (cycles >= gThrottleCycles)
		{
            if(fullspeed != 3)
                usleep(sleepUs);
            else
                usleep(sleepUs * 2);
			sem_wait(&gThrottleEvent);
		}
#if 0
		double	hires;

		if(last_instruct==0)
			last_instruct=hirestimer();

		last_instruct+=cy*0.000000416666666667;
		while((hires = hirestimer())<last_instruct) 
		{
			struct timespec ts;

			ts.tv_sec=0;
			ts.tv_nsec=10000;
			nanosleep(&ts,NULL);
		}
#endif
#endif
	}
	else
		last_instruct = 0;
}

/*
=============================================================================
This routine bails from the app in case of a panic.
=============================================================================
*/
void bail(char *msg)
{
	int endtime;

	endtime=msclock();
	puts(msg);
	printf("%-22s A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X PC:%04X SP:%04X IM:%02X *SP:%04X\n",op,A,F,B,C,D,E,H,L,PC,SP,IM,MEM16(SP));
	printf("Start: %d  End: %d\n",starttime,endtime);
	printf("Time: %f\n",((double)(endtime-starttime))/1000);
	printf("MHz: %f\n",((UINT64)cycles/(((double)(endtime-starttime))/1000))/1000000);

	exit(1);
}

/*
=============================================================================
This routine performs a quick "warm" reset with no re-initialization of mem.
=============================================================================
*/
void jump_to_zero(void)
{
	int		i;

	/* Clear all CPU registers */
	for (i = 0; i < sizeof(cpu); i++)
		cpu[i] = 0;
}

/*
=============================================================================
debug_set_monitor_callback:	This function adds a new debug monitor process.
=============================================================================
*/
int debug_set_monitor_callback(debug_monitor_callback pCallback)
{
	int		x;

	for (x = 0; x < 3; x++)
	{
		if (gpDebugMonitors[x] == NULL)
		{
			gpDebugMonitors[x] = pCallback;
			return x;
		}
	}

	return -1;
}

/*
=============================================================================
debug_clear_monitor_callback:	This function deletes the specified callback 
								from the debug monitor list.
=============================================================================
*/
void debug_clear_monitor_callback(debug_monitor_callback pCallback)
{
	int		x;

	for (x = 0; x < 3; x++)
	{
		if (gpDebugMonitors[x] == pCallback)
		{
			gpDebugMonitors[x] = NULL;
			return;
		}
	}
}

/*
=============================================================================
debug_step:	This function performs a CPU step operation and invokes the 
			callback for each debug monitor to inform it of the CPU step 
			operation.
=============================================================================
*/
void debug_step(void)
{
	int		x;

	for (x = 0; x < 3; x++)
	{
		if (gpDebugMonitors[x] != NULL)
			gpDebugMonitors[x](DEBUG_CPU_STEP);
	}
}

/*
=============================================================================
get_model_string:	This function returns the sring name of the specified
					emulation model.
=============================================================================
*/
void get_model_string(char* str, int model)
{
	switch (model)
	{
	case MODEL_M100:
		strcpy(str, "m100");
		break;
	case MODEL_M102:
		strcpy(str, "m102");
		break;
	case MODEL_T200:
		strcpy(str, "t200");
		break;
	case MODEL_M10:
		strcpy(str, "m10");
		break;
	case MODEL_PC8201:
		strcpy(str, "pc8201");
		break;
	case MODEL_KC85:
		strcpy(str, "kc85");
		break;
	case MODEL_PC8300:
		strcpy(str, "pc8300");
		break;
	}
}

/*
=============================================================================
get_model_from_string:	This function returns the model number given the 
						sring name specified.
=============================================================================
*/
int get_model_from_string(char* str)
{
	if (strcmp("m100", str) == 0)
		return MODEL_M100;
	else if (strcmp("m102", str) == 0)
		return MODEL_M102;
	else if (strcmp("t200", str) == 0)
		return MODEL_T200;
	else if (strcmp("pc8201", str) == 0)
		return MODEL_PC8201;
	else if (strcmp("pc8300", str) == 0)
		return MODEL_PC8300;
	else if (strcmp("m10", str) == 0)
	   return MODEL_M10;
	else if (strcmp("kc85", str) == 0)
		return MODEL_KC85;

	return -1;
}

/*
=============================================================================
get_emulation_path:	This function returns the path of the emulation directory
					based on the supplied model.
=============================================================================
*/
void get_emulation_path(char* emu, int model)
{
	strcpy(emu, path);			/* Copy VirtualT path */
	switch (model)
	{
	case MODEL_M100:
		strcat(emu, "M100/");
		break;
	case MODEL_M102:
		strcat(emu, "M102/");
		break;
	case MODEL_T200:
		strcat(emu, "T200/");
		break;
	case MODEL_M10:
		strcat(emu, "M10/");
		break;
	case MODEL_PC8201:
		strcat(emu, "PC8201/");
		break;
	case MODEL_KC85:
		strcat(emu, "KC85/");
		break;	
	case MODEL_PC8300:
		strcat(emu, "PC8300/");
		break;
	}
}

/*
=============================================================================
get_rom_path:	This function returns the path of the ROM file for the 
				model specified.  The path is strcpy(ed) into the string 
				supplied.
=============================================================================
*/
void get_rom_path(char* file, int model)
{
	strcpy(file, path);			/* Copy VirtualT path */
	
	switch (model)
	{
	case MODEL_M100:
		strcat(file, "M100/M100rom.bin");
		break;
	case MODEL_M102:
		strcat(file, "M102/M102rom.bin");
		break;
	case MODEL_T200:
		strcat(file, "T200/T200rom.bin");
		break;
	case MODEL_M10:
		strcat(file, "M10/M10rom.bin");
		break;
	case MODEL_PC8201:
		strcat(file, "PC8201/PC8201rom.bin");
		break;
	case MODEL_KC85:
		strcat(file, "KC85/KC85rom.bin");
		break;
	case MODEL_PC8300:
		strcat(file, "PC8300/PC8300rom.bin");
		break;
	}
}

/*
=============================================================================
check_model_support:	This function checks for support for the specified 
						Model and returns TRUE if that model is supported.

						Model Support is determined by checking for the 
						appropriate directory structure with the correct ROM
						file.
=============================================================================
*/
int check_model_support(int model)
{
	char	file[256];
	FILE*	fd;

	/* Get the path for the model supplied */
	get_rom_path(file, model);

	/* Attempt to open the ROM file */
	if ((fd = fopen(file, "r")) == NULL)
		return FALSE;

	/* Open successful, close the file and return */
	fclose(fd);
	return TRUE;
}

/*
=============================================================================
check_installation:	This routine checks that VirtualT is properly installed
					with model directories and appropriate rom files.

					If the files are not installed, it attempts to create
					them using files in the ROMs directory.
=============================================================================
*/
void check_installation(void)
{
	int 	model, len;
	char	localpath[256];
	char	roms_path[512];
	char	errors[256];
	FILE	*fd, *fd2;

	/* Test if Mac OSX and no path specified */
#ifdef __APPLE__
	struct stat romStat;
	if (strlen(path) == 0)
		return;
#endif

	errors[0] = 0;

	/* Check each model */
	for (model = MODEL_M100; model <= MODEL_PC8300; model++)
	{
		/* Check if ROM file exists for this model */
		if (check_model_support(model))
			continue;

		/* ROM file doesn't exist.  Try to open in ROMs dir */
		get_rom_path(localpath, model);
#if defined(__APPLE__)
		sprintf(roms_path, "%sROMs%s", path, strrchr(localpath, '/'));
		
		/* Test if the ROM file exists in the working directory */
		if (stat(roms_path, &romStat) != 0)
		{
			/* Test if running from a bundle & get the bundle path */
			if (strlen(gOsxBundlePath) > 0)
			{
				sprintf(roms_path, "%s/Resources%s", gOsxBundlePath, strrchr(localpath, '/'));
			}
		}
#else
		sprintf(roms_path, "ROMs%s", strrchr(localpath, '/'));
#endif

		if ((fd = fopen(roms_path, "rb")) == NULL)
		{
			/* Error - ROM file not in ROMs dir */
			if (strlen(errors) != 0)
				strcat(errors, ", ");
			get_model_string(localpath, model);
			strcat(errors, localpath);
			continue;		
		}

		/* Create the emulation directory */
		get_emulation_path(localpath, model);
#ifdef WIN32
		_mkdir(localpath);
#else
		mkdir(localpath, 0755);
#endif
		/* Create ROM in the emulation directory */
		get_rom_path(localpath, model);
		fd2 = fopen(localpath, "wb");
		if (fd2 == NULL)
		{
			if (strlen(errors) != 0)
				strcat(errors, ", ");
			get_model_string(localpath, model);
			strcat(errors, localpath);
			fclose(fd);
			continue;
		}

		/* Copy the ROM file from ROMs dir */
		while (1)
		{
			/* Read the ROM contents so we can save in Model directory */
			len = fread(gOptROM, 1, 32768, fd);
			if (len == 0)
				break;
			fwrite(gOptROM, 1, len, fd2);
		}

		/* Close both files */
		fclose(fd);
		fclose(fd2);
	}

	if (strlen(errors) > 0)
	{
		sprintf(localpath, "No ROM file for %s", errors);
		show_error(localpath);
	}
}

void model_8201_bug_workaround(int reason)
{
	// Remove debugging after we get the to main menu
	if (PC == 0x6d91)
	{
		lock_remote();
		debug_clear_monitor_callback(model_8201_bug_workaround);
		gDebugActive--;
		unlock_remote();
	}
}
/*
======================================================================
initcpu:	This function initializes the CPU and the system memories
			including loading the ROM and RAM files.
======================================================================
*/
void init_cpu(void)
{
	int		i;

	/* Initialize CPU registers */
	A = F = B = C = D = E = H = L = 0;
	SPH = SPL = 0;
	PCH = PCL = 0;
	IM=0x08;
	cpuMISC=0;
	
	load_sys_rom();

	/* Clear the system memory RAM area */
	for (i = 0; i < RAMSIZE; i++)
		gBaseMemory[RAMSTART + i] = 0;
	
	/* Read RAM from file in emulation directory */
	load_ram();

	/* Load option ROM if any */
	load_opt_rom();

	/* 
		Work around a stupid PC-8201 bug that couldn't be found.  For some reason,
		in the Windows Release build, the ROM gets stuck looking for RS232 data and
		waiting for SHIFT-BREAK to be pressed in the boot routine.  If I try to trace
		the ROM, the problem goes away, so it has to be timing related somewhere.  I
		found that simply adding delay by "debug tracing" each PC location fixes the
		issue.  The code below adds a 1-time debug hook during boot do add delay that
		is later removed.  This allows delay to be added at boot without adding any 
		additional delay (if statements) during the main portion of the code. This is a 
		horrible kludge, but it fixes the problem.
	*/
#ifdef WIN32
	if (gModel == MODEL_PC8201 || gModel == MODEL_PC8300)
	{
		lock_remote();
		debug_set_monitor_callback(model_8201_bug_workaround);
		gDebugActive++;
		unlock_remote();
	}
#endif

	gExitLoop = 1;
}


/*
========================================================================
This routine peforms a CPU reset and reinitializes memory.
========================================================================
*/
void resetcpu(void)
{
	int		i;

	/* Clear all CPU registers */
	for (i = 0; i < sizeof(cpu); i++)
		cpu[i] = 0;

	/* Set interrupt mask */
	IM=0x08;
	cpuMISC=0;

	/* Re-initialize memory */
	reinit_mem();

	/* Check if debug monitor active and updae if there are */
	if (gDebugActive)
	{
		/* Call all debug monitor callback routines */
		for (i = 0; i < 3; i++)
		{
			if (gpDebugMonitors[i] != NULL)
				gpDebugMonitors[i](DEBUG_PC_CHANGED);
		}
	}
	/* 
		Work around a stupid PC-8201 bug that couldn't be found.  For some reason,
		in the Windows Release build, the ROM gets stuck looking for RS232 data and
		waiting for SHIFT-BREAK to be pressed in the boot routine.  If I try to trace
		the ROM, the problem goes away, so it has to be timing related somewhere.  I
		found that simply adding delay by "debug tracing" each PC location fixes the
		issue.  The code below adds a 1-time debug hook during boot do add delay that
		is later removed.  This allows delay to be added at boot without adding any 
		additional delay (if statements) during the main portion of the code. This is a 
		horrible kludge, but it fixes the problem.
	*/
#ifdef WIN32
	if (gModel == MODEL_PC8201 || gModel == MODEL_PC8300)
	{
		lock_remote();
		debug_set_monitor_callback(model_8201_bug_workaround);
		gDebugActive++;
		unlock_remote();
	}
#endif
}
void cb_int65(int pinLevel)
{
	/* The INT6.5 pin is level sensitive.  Set the interrupt pending bit based on current level */
	if (pinLevel)
	{
		/* Set the INT6.5 pending bit high */
		IM|=0x20;
		if(trace && tracefile != NULL)
			fprintf(tracefile,"RST 6.5 Issued\n");
	}
	else
	{
		/* Clear the INT6.5 pending bit */
		IM &= 0xDF;
	}
}


/*
========================================================================
This routine processes CPU interrupts.
========================================================================
*/
void check_interrupts(void)
{
	static volatile UINT64	last_rst75=0;

	if (((last_rst75 + rst7cycles) < cycles) && !INTDIS)
	{
		IM |= 0x40;
//		if(trace && tracefile != NULL)
//			fprintf(tracefile,"RST 7.5 Issued diff = %d\n", (DWORD) (cycles - last_rst75));
		last_rst75=cycles;
	}

	/* TRAP should be first */

	if(RST75PEND && !INTDIS && !RST75MASK) {
		if(trace && tracefile != NULL)
			fprintf(tracefile,"RST 7.5 CALLed\n");

		if (gDebugInts)
			gIntActive = TRUE;
		gIntSP = SP;
		DECSP2;
		if (gReMem)
		{
			MEMSET(SP, PCL);
			MEMSET(SP+1, PCH);
		}
		else
		{
			gBaseMemory[SP] = PCL;
			gBaseMemory[SP+1] = PCH;
		}
		/* MEM16(SP)=PC; */
		PCL=60;
		PCH=0;
		/* PC=60; */
		cycle_delta += 10;	/* This may not be correct */
		IM=IM&0xBF;
		last_isr_cycle = cycles;

		if (gDelayUpdateKeys == 1)
			update_keys();
	}
	else if(RST65PEND && !INTDIS && !RST65MASK) 
	{
		if(trace && tracefile != NULL)
			fprintf(tracefile,"RST 6.5 CALLed\n");

		if (gDebugInts)
			gIntActive = TRUE;
		gIntSP = SP;
		DECSP2;
		if (gReMem)
		{
			MEMSET(SP, PCL);
			MEMSET(SP+1, PCH);
		}
		else
		{
			gBaseMemory[SP] = PCL;
			gBaseMemory[SP+1] = PCH;
		}

		/* MEM16(SP)=PC; */
		PCL=52;
		PCH=0;
		/* PC=52; */
		cycle_delta += 10;	/* This may not be correct */
//		IM=IM&0xDF;
		last_isr_cycle = cycles;
	}
	return;
}

void	mem_set_monitor_callback(mem_monitor_cb cb)
{
	if (gMemMonitor1 == NULL)
		gMemMonitor1 = cb;
	else if (gMemMonitor2 == NULL)
		gMemMonitor2 = cb;
}

void	mem_clear_monitor_callback(mem_monitor_cb cb)
{
	if (gMemMonitor1 == cb)
		gMemMonitor1 = NULL;
	else if (gMemMonitor2 == cb)
		gMemMonitor2 = NULL;
}

void remote_switch_model(int model)
{
	gRemoteSwitchModel = model;
}

/*
========================================================================
This routine performs periodic maintenance operations, such as checking
interrupts and processing FLTK window events.
========================================================================
*/
void maint(void)
{
	static int		rst7UpdateCnt = 0;
	DWORD			new_rst7cycles;
	static int		lpt_update = 0;
	int				count;
	static int		secsPeriod = 0;
	static	int		secsCycles = 0;
	DWORD			deltaCycles, oneSecCycleEstimate;
	static UINT64	lastDeltaCycles = 0;

	/* Adjust the background tick cycle estimate 25 times a second */
	if (gThrottlePeriodCount > 0)
	{
		/* Get and reset the period count */
		#ifdef WIN32
			EnterCriticalSection(&gThrottleCritSect);
        #else
            pthread_mutex_lock(&gThrottleLock);
		#endif

		count = gThrottlePeriodCount;
		gThrottlePeriodCount = 0;

		#ifdef WIN32
			LeaveCriticalSection(&gThrottleCritSect);
        #else
            pthread_mutex_unlock(&gThrottleLock);
		#endif

		// Calculate the delta in cycles since the last time we checked
		deltaCycles = (DWORD) (cycles - lastDeltaCycles);
		lastDeltaCycles = cycles;
		oneSecCycleEstimate = (DWORD) ((UINT64) deltaCycles * gThrottleTerminalCount / count);
		secsPeriod += count;
		secsCycles += deltaCycles;
		if (secsPeriod >= gThrottleTerminalCount)
		{
			one_sec_cycles = cycles;

			/* Every second we want to update the CPU frequencey and do some maintenance */
			gCpuSpeedAvg[gCpuSpeedAvgIndex++] = (float) secsCycles / (float) 1000000.0;
			gCpuSpeedAvgIndex %= 4;
			cpu_speed = (gCpuSpeedAvg[0] + gCpuSpeedAvg[1] + gCpuSpeedAvg[2] + gCpuSpeedAvg[3]) / (float) 4.0;
			display_cpu_speed();

			/* Check for errors on the LPT port and update the Peripherial Display if visible */
			lpt_check_errors();
			periph_mon_update_lpt_log();
			time(&one_sec_time);

			/* Clear variables so we only enter here once a second */
			secsCycles = 0;
			secsPeriod = 0;
		}

#ifdef WIN32
		new_rst7cycles = (DWORD) (0.0039998 * oneSecCycleEstimate);	  /* 0.0039998 = 9830 / 2457600 */
#else
		if (fullspeed == 3)
			new_rst7cycles = (DWORD) (oneSecCycleEstimate / 256);	  /* Fullspeed sleeps twice as long */
		else
			new_rst7cycles = (DWORD) (oneSecCycleEstimate / 128);
#endif

		// Check if the rst7cycles needs to be updated.  We only update this every 3rd time through
		// The rst7cycles value is what controls the keyboard-timer interrupt rate and it
		// changes based on host CPU speed to try to match the M100 scan rate
		if ((new_rst7cycles > (rst7cycles * 0.8)) || (rst7UpdateCnt == 2))
		{
			rst7cycles = new_rst7cycles;
			if (rst7cycles < 9830)
				rst7cycles = 9830;
			rst7UpdateCnt = 0;

			// Check if we need to update the Memory Editor monitor
			if (gMemMonitor1 != NULL)
				gMemMonitor1();
			if (gMemMonitor2 != NULL)
				gMemMonitor2();
		}
		else
			rst7UpdateCnt++;

		// Check if we need to do lpt animation
		if (++lpt_update >= 7)
		{
			lpt_perodic_update();
			lpt_update = 0;
		}

		// Test if socket interface simulating a key press
		handle_simkey();
		process_windows_event();
	}

	// Test if socket interface requested model switch
	if (gRemoteSwitchModel != -1)
	{
		switch_model(gRemoteSwitchModel);
		gRemoteSwitchModel = -1;
	}

	// Handle LPT Timeout Activity
	if (one_sec_time != gLptTime)
	{
		gLptTime = one_sec_time;
		handle_lpt_timeout((DWORD) one_sec_time);
	}

	if (gInMsPlanROM)
		gInMsPlanROM--;
}


/*
========================================================================
This routine checks if there are any active debug monitors and calls
each attached monitor so it can perform it's debug tasks.
========================================================================
*/
void do_debug_stuff(void)
{
	int		i;

	/* Call all debug monitor callback routines */
	unlock_remote();
	for (i = 0; i < 3; i++)
	{
		if (gpDebugMonitors[i] != NULL)
			gpDebugMonitors[i](DEBUG_PC_CHANGED);
	}

	if (gStopped)
	{
		if (gLastWasSingleStep)
		{
			for (i = 0; i < 3; i++)
			{
				if (gpDebugMonitors[i] != NULL)
					gpDebugMonitors[i](DEBUG_CPU_STEP);
			}
		}

		gOsDelay = 1;
		/* Loop until not stopped or single step */
		while (gStopped && !gSingleStep && !gExitApp)
		{
			process_windows_event();
		}
		gOsDelay = 0;
		gLastWasSingleStep = gSingleStep;
		gSingleStep = 0;
	}
	lock_remote();
}

/*
========================================================================
emulate:	This routine contains the main loop that handles active 
			emulation and maintenance tasks.
========================================================================
*/
void emulate(void)
{
	unsigned int	i,j;
	unsigned int	v;
	int				nxtmaint=1;
	int				ins;

	starttime=msclock();
	lock_remote();
	while(!gExitApp) 
	{
		/* Run appropriate emulation loop base on ReMem support */
		if (gReMem)
			while (!gExitLoop)
			{
				/* Check for active debug monitor windows */
				if (gDebugActive)
				{
					do_debug_stuff();
					if (gExitLoop) 
						break;
				}

				/* Instruction emulation contained in header file */
				#undef NO_REMEM
				#include "cpu.h"
				#include "do_instruct.h"

				// Check if next inst is SIM
				if ((get_memory8(PC) == 0xF3) || ((IM & 0x2A) == 0x20) ||
					((IM & 0x4C) == 0x40))
				{
					check_interrupts();
				}

				// Check for return from interrupt
				if (gIntActive)
				{
					if (SP == gIntSP)
						gIntActive = FALSE;
				}
				if (gSingleStep)
				{
					if (!gIntActive)
					{
						gSingleStep = 0;
					}
				}

				/* Do maintenance tasks (Windows events, interrupts, etc.) */
#ifdef __APPLE__
				if(!(--nxtmaint)) 
#else
				if(!(--nxtmaint & 0x3FF)) 
#endif
				{
					unlock_remote();
					gOsDelay = nxtmaint == 0;
					throttle(cycle_delta);
					maint();
					ser_poll();
					check_interrupts();
					if (gOsDelay)
						nxtmaint=gMaintCount;
					lock_remote();
				}
#if 0
				exec_remem_instruction();

					if (gExitLoop) 
						break;
#endif
			}

		if (!gReMem)
		{
			while (!gExitLoop)
			{
				/* Check for active debug monitor windows */
				if (gDebugActive)
				{
					do_debug_stuff();
					if (gExitLoop)
						break;
				}

				/* Instruction emulation contained in header file */
				#define NO_REMEM
				#include "cpu.h"
				#include "do_instruct.h"

				// Check if next inst is SIM or RS-232 waiting
				if ((get_memory8(PC) == 0xF3) || ((IM & 0x2A) == 0x20) ||
					((IM & 0x4C) == 0x40))
				{
					check_interrupts();
				}

				// Check for return from interrupt
				if (gIntActive)
				{
					if (SP == gIntSP)
						gIntActive = FALSE;
				}
				if (gSingleStep)
				{
					if (!gIntActive)
					{
						gSingleStep = 0;
					}
				}

				/* Do maintenance tasks (Windows events, interrupts, etc.) */
#ifdef __APPLE__
				if(!(--nxtmaint)) 
#else
				if(!(--nxtmaint & 0x3FF)) 
#endif
				{
					unlock_remote();
					gOsDelay = nxtmaint == 0;
					throttle(cycle_delta);
					maint();
					ser_poll();
					check_interrupts();
					if (gOsDelay)
						nxtmaint=gMaintCount;
					lock_remote();
				}
			}
		}

		gExitLoop = 0;
	}
	unlock_remote();
}

/*
========================================================================
handle_sig:	This routine handles unix signals and kills the app if there
			is a panic.
========================================================================
*/
#ifdef __unix__
void handle_sig(int sig)
{
	bail("Kill by signal");
}
#endif

/*
========================================================================
process_args:	This routine processes the command-line arguments.
========================================================================
*/
int process_args(int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++)
	{
		// Seach for trace flag
		if (!strcmp(argv[i], "-t"))
		{
			trace = 1;				/* Turn on trace mode */
			tracefile = fopen("trace.txt", "w+");
		}

		// Search for No-GUI flag
		if (!strcmp(argv[i], "-nogui"))
			gNoGUI = 1;

		if (!strcmp(argv[i], "-t"))
			set_remote_cmdline_telnet(TRUE);

		// Search for Socket Port flag
		if (!strncmp(argv[i], "-p", 2))
		{
			// Check if port number is part of flag
			if (strlen(argv[i]) != 2)
			{
				set_remote_cmdline_port(atoi(&argv[i][2]));
			}
			else
			{
				// Socket number is next argument
				if (i + 1 >= argc)
				{
					printf("%s: -p port not specified\n", argv[0]);
					return 1;
				}
				else
				{
					set_remote_cmdline_port(atoi(argv[i+1]));
					i++;
				}
			}
		}
	}

	return 0;
}

/*
========================================================================
This routine sets unix signals and links them to a 'bail' routine.
========================================================================
*/
void setup_unix_signals(void)
{
#ifdef __unix__
	signal(SIGTERM,handle_sig);
	signal(SIGHUP,handle_sig);
	signal(SIGQUIT,handle_sig);
	signal(SIGINT,handle_sig);
#endif
}

/*
========================================================================
This routine sets the working path.  On Linux and MacOS platforms, this
is done by looking at the argv[0] argument and removing the app name.
========================================================================
*/
void setup_working_path(char **argv)
{
#if defined(__unix__) || defined(__APPLE__)
	int			i; 
	char*		pCwd;
    
#if defined(__APPLE__)
    int         found;
	char		temp[512];
	struct stat romStat;
	char*		pContents, *pStr;
#endif

	pCwd = getcwd(path, sizeof(path));
	(void) pCwd;

#if defined(__APPLE__)
	/* On MacOSX, getcwd returns nothing when launched from the Finder.  So
	   we must perform an intelligent search for the working directory
	   based on argv[0].
	*/
	found = FALSE;
		
	/* Search for the /VirtualT.app/Contents directory */
	pContents = strstr(argv[0], "/Contents/MacOS");

	// First search for "/VT Emulation" folder in the same directory as the VirtualT.app
	if (pContents != NULL)
	{
		for (i = 0, pStr = argv[0]; pStr != pContents; pStr++)
			path[i++] = *pStr;
		path[i++] = '/';
		path[i] = '\0';

		/* Save a copy of the bundle path */
		strcpy(gOsxBundlePath, path);
		strcat(gOsxBundlePath, "/Contents");

		strcat(path, "/VT Emulation");
		if (stat(path, &romStat) == 0)
			found = TRUE;
		else
			path[0] = '\0';
	}

	// Search in the User's Home/Documents directory if not found
	if (!found)
	{
		// Get the HOME directory from the environement
		char* ptr = getenv("HOME");
		if (ptr != NULL)
		{
			strcpy(temp, ptr);
			strcat(temp, "/Documents/VT Emulation");
			if (stat(temp, &romStat) == 0)
			{
				found = TRUE;
				strcpy(path, temp);
			}
		}
	}

	// Search for a ROMs directory
	if (!found)
	{
		/* Recursively search up the path looking for a ROMs directory */
		i = strlen(argv[0])-1;
		strcpy(temp, argv[0]);
		while (!found)
		{
			// Find last '/' in path to remove app name from the path
			while ((argv[0][i] != '/') && (i > 0))
				i--;
			if (i < 0)
				break;
			temp[i+1] = 0;

			/* Now append "ROMs" and check if the directory exists */
			strcat(temp, "ROMs");
			if (stat(temp, &romStat) == 0)
			{
				if (S_ISDIR(romStat.st_mode))
				{
					found = TRUE;
					strncpy(path, argv[0], i+1);
					path[i+1] = 0;
				}
				i--;
			}
			else
			{
				/* Not this directory segment...skip the '/' and find next higher */
				i--;
			}
		}
	}
#endif
	
	// Check if the last character is '/'
	i = strlen(path);
	if ((i > 0) && (path[i-1] != '/'))
		strcat(path, "/");
# else
	(void) _getcwd(path, sizeof(path));
	strcat(path,"\\");
#endif
}

/*
========================================================================
main:	This is the main entry point for the VirtualT application.
========================================================================
*/
int main(int argc, char **argv)
{
	if (process_args(argc, argv))	/* Parse command line args */
		return 1;

	setup_working_path(argv);	/* Create a working dir path */
	setup_unix_signals();		/* Setup Unix signal handling */

	// Added by JV for prefs
	init_pref();				/* load user Menu preferences */
	check_installation();		/* Test if install needs to be performed */
	load_setup_preferences();	/* Load user Peripheral setup preferences */
	load_memory_preferences();	/* Load user Memory setup preferences */
	load_remote_preferences();  /* Load user Remote Socket preferences */
	
	/* Perform initialization */
	init_mem();					/* Initialize Memory */
	init_io();					/* Initialize I/O structures */
	init_sound();				/* Initialize Sound system */
	init_display();				/* Initialize the Display */
	init_cpu();					/* Initialize the CPU */
	init_throttle_timer();		/* Initialize the throttle timer */
	init_remote();				/* Initialize the remote control */
	init_lpt();					/* Initialize the printer subsystem */
	init_other_windows();		/* Initialize other windows that were opened (memedit, regs, etc. */
	get_model_time();			/* Load the emulated time for current model */

	/* Perform Emulation */
	emulate();					/* Main emulation loop */

	/* Save RAM contents after emulation */
	save_ram();
	save_model_time();			/* Save the emulated time */

	/* Cleanup */
	deinit_io();				/* Deinitialize I/O */
	deinit_sound();				/* Deinitialize sound */
	deinit_lpt();				/* Deinitialize the printer */
	deinit_throttle_timer();	/* Deinitialize the throttle timer */
	deinit_display();			/* Deinitialze and free the main window */
	free_mem();					/* Free memory used by ReMem and/or Rampac */

	return 0;
}
