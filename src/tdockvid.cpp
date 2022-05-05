/* tdockvid.cpp */

/* $Id: tdockvid.cpp,v 1.1 2015/02/24 20:19:17 kpettit1 Exp $ */

/*
 * Copyright 2015 Ken Pettit
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
#include "tdockvid.h"
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
}


extern int				MultFact;
extern int				DisplayMode;
extern int				Fullscreen;
extern int				SolidChars;
extern int				DispHeight;
extern int				gRectsize;
extern int				gXoffset;
extern int				gYoffset;
extern int				gSimKey;
extern int				gFrameColor;
extern int				gDetailColor;
extern int				gBackgroundColor;
extern int				gPixelColor;
extern int				gLabelColor;
extern int				gConsoleDebug;

#ifndef	WIN32
#define	min(a,b)	((a)<(b) ? (a):(b))
#endif
#ifdef WIN32
extern "C"
{
int strcasecmp(const char* s1, const char* s2);
}
#endif

extern T100_Disp* gpDisp;

typedef struct namedColors
{
    const char *    pName;
    unsigned char   cid;
    unsigned char   red;
    unsigned char   green;
    unsigned char   blue;
} namedColors_t;

static namedColors_t gNamedColors[] = {
#include "colors.h"
};
static int gColorCount = sizeof(gNamedColors) / sizeof(namedColors_t);

/*
=======================================================
T100:Disp:	This is the class construcor
=======================================================
*/
VTTDockVid::VTTDockVid(int x, int y, int w, int h) :
  Fl_Widget(x, y, w, h)
{
	int c, red, green, blue;

	m_FrameColor = gFrameColor;
	m_DetailColor = gDetailColor;
	m_BackgroundColor = gBackgroundColor;
	m_PixelColor = gPixelColor;
	m_LabelColor = gLabelColor;
	m_HaveMouse = FALSE;

    memset(pixdata, 0, 200*480);

	m_MyFocus = 0;
    m_CurX = 0;
    m_CurY = 0;
    m_EscSeq = 0;
    m_Reverse = 0;
    m_Locked = 0;
    m_Width = 40;
    m_Color = 255;

    m_Colors[1] = FL_DARK_RED;
    m_Colors[2] = FL_DARK_GREEN;
    m_Colors[3] = FL_DARK_YELLOW;
    m_Colors[4] = FL_DARK_BLUE;
    m_Colors[5] = FL_DARK_MAGENTA;
    m_Colors[6] = FL_DARK_CYAN;
    m_Colors[7] = FL_LIGHT1;

    m_Colors[8] = FL_DARK2;
    m_Colors[9] = FL_RED;
    m_Colors[10] = FL_GREEN;
    m_Colors[11] = FL_YELLOW;
    m_Colors[12] = FL_BLUE;
    m_Colors[13] = FL_MAGENTA;
    m_Colors[14] = FL_CYAN;
    m_Colors[15] = FL_WHITE;

    /* 6x6x6 Color Cube */
    c = 16;
    for (red = 0; red <=255; red += 51)
        for (green = 0; green <=255; green += 51)
            for (blue = 0; blue <=255; blue += 51)
            {
                m_Colors[c++] = fl_rgb_color(red , green, blue);
            }

    /* Gray scale */
    red = 255;
    for (c = 254; c >= 232; c--)
    {
        m_Colors[c] = fl_rgb_color(red, red, red);
        red -= 11;
    }

    m_Colors[0] = gBackgroundColor;
    m_Colors[255] = gPixelColor;

	MultFact = 2;
	DisplayMode = 1;
	SolidChars = 0;
	DispHeight = 64;
	gRectsize = 2;

	m_BezelTop = m_BezelLeft = m_BezelBottom = m_BezelRight = 0;
	m_BezelTopH = m_BezelLeftW = m_BezelBottomH = m_BezelRightW = 0;

    CalcScreenCoords();
}

VTTDockVid::~VTTDockVid()
{
}

/*
=================================================================
Clear:	This routine clears the "LCD"
=================================================================
*/
void VTTDockVid::Clear(void)
{
  memset(pixdata, 0, 200*480);

  m_CurX = m_CurY = 0;
  redraw();
  Fl::check();
}

/*
=================================================================
drawpixel:	This routine is called by the system to draw a single
			black pixel on the "LCD".
=================================================================
*/
// Draw the black pixels on the LCD
__inline void VTTDockVid::drawpixel(int x, int y, int color)
{
	// Check if the pixel color is black and draw if it is
	if (color)
    {
		fl_rectf(x*MultFact + gXoffset,y*MultFact + gYoffset,
		    gRectsize, gRectsize);
    }
}

/*
=======================================================
Calculate the xoffset, yoffset, border locations, etc.
=======================================================
*/
void VTTDockVid::CalcScreenCoords(void)
{
	// Calculatet the pixel rectangle size
	::gRectsize = MultFact;
	if (::gRectsize == 0)
		::gRectsize = 1;

	// Calculate xoffset and yoffset
	//if (Fullscreen)
	//{
		//::gXoffset = parent()->w() / 2 - 240 * MultFact;
		//::gYoffset = (parent()->h() - MENU_HEIGHT - 20 - 200 * MultFact) / 3 + MENU_HEIGHT+1;
	//}
	//else
	//{
		::gXoffset = 5;
		::gYoffset = MENU_HEIGHT+5;
	//}

	gRectsize = ::gRectsize;
	gXoffset = ::gXoffset;
	gYoffset = ::gYoffset;

	// If the display is framed, then calculate the frame coords
    
	if (DisplayMode && 0)
	{
		// Calculate the Bezel location
		int		wantedH = 20;
		int		wantedW = 40;
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
		m_BezelBottom = gYoffset + 200 * MultFact + 1;
		bottomSpace = parent()->h() - m_BezelBottom - 20;
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

/*
=================================================================
draw_static:	This routine draws the static portions of the LCD,
				such as erasing the background, drawing function
				key labls, etc.
=================================================================
*/
void VTTDockVid::draw_static()
{
	int c;
	int width;
	int x_pos, inc, start, y_pos;
	int xl_start, xl_end, xr_start, xr_end;
	int	num_labels;

	// Draw gray "screen"
    fl_color(m_BackgroundColor);
    fl_rectf(x(),y(),w(),h());

    return;

	/* Check if the user wants the display "framed" */
	if (DisplayMode == 1)
	{
		// Color for outer border
		fl_color(m_DetailColor);

		// Draw border along the top
		if (m_HasTopChassis)
			fl_rectf(x(),y(),w(),m_BezelTop - MENU_HEIGHT - 1);

		// Draw border along the bottom
		if (m_HasBottomChassis)
			fl_rectf(x(),m_BezelBottom + m_BezelBottomH,w(),parent()->h() - m_BezelBottom - m_BezelBottomH - 20);

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
}

/*
=================================================================
draw:	This routine draws the entire LCD.  This is a member 
		function of Fl_Window.
=================================================================
*/
void VTTDockVid::draw()
{
	/* Draw static background stuff */
	draw_static();

    /* Draw the pixels */
    draw_pixels();
}

/*
=================================================================
draw_pixels:	This routine draws the pixels on the display.
=================================================================
*/
void VTTDockVid::draw_pixels()
{
	int x=0;
	int y=0;
	int line;
	uchar value;
    int  lastColor = -1;
    int  color;

    window()->make_current();

	for (line = 0; line < 200; line++)
	{
        for (x = 0; x < 480; x++)
        {
            value = pixdata[line][x];
            //y = line << 3;

            // Erase line so it is grey, then fill in with black where needed
            if ((line & 0x07) == 0)
            {
                fl_color(m_BackgroundColor);
                fl_rectf(x*MultFact + gXoffset,line*MultFact + 
                    gYoffset,gRectsize,8*MultFact);
            }
            if (value == 0)
                continue;

            // Draw the black pixels
            //if (value)
            {
                color = m_Colors[value];
                //if (color != lastColor)
                {
                    fl_color(color);
                    lastColor = color;
                }
                drawpixel(x,line,1);
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
void VTTDockVid::SetByte(int line, int col, uchar value)
{
	int y;

    if (line > 24 || col > 479)
        return;
    if (line < 0 || col < 0)
        return;

    // Check if LCD already has the value being requested
    //if (pixdata[line][col] == value)
    //    return;

    // Load new value into lcd "RAM"
    y = line << 3;
    //pixdata[line][col] = value;

    pixdata[y++][col] = value&0x01 ? m_Color : 0;
    pixdata[y++][col] = value&0x02 ? m_Color : 0;
    pixdata[y++][col] = value&0x04 ? m_Color : 0;
    pixdata[y++][col] = value&0x08 ? m_Color : 0;
    pixdata[y++][col] = value&0x10 ? m_Color : 0;
    pixdata[y++][col] = value&0x20 ? m_Color : 0;
    pixdata[y++][col] = value&0x40 ? m_Color : 0;
    pixdata[y++][col] = value&0x80 ? m_Color : 0;

    // Calcluate y position of byte
    y = line << 3;

    // Set the display
    window()->make_current();

    fl_color(m_BackgroundColor);
    fl_rectf(col*MultFact + gXoffset, y*MultFact + 
        gYoffset, gRectsize,MultFact<<3);

    if (value == 0)
        return;

    fl_color(m_Colors[m_Color]);

    for (y = line << 3; y < (line+1) << 3; y++)
        drawpixel(col,y,pixdata[y][col]);
}

/*
================================================================================
WriteData:	Routine that receives data from the VDock interface when the 
			Model T sends data / commands to our device ID.
================================================================================
*/
void VTTDockVid::WriteData(uchar data)
{
    int     line, col, y;
    int     color;

#if 0
    if (m_EscSeq)
    {
        printf("%c(%02x),  X=%d, Y=%d\n", data, data, m_CurX, m_CurY);
    }
    else if (data == 0x1B)
        printf("ESC ");
#if 1
    else if (data <= ' ')
        printf("%02X,  X=%d, Y=%d\n", data, m_CurX, m_CurY);
    else
        printf("%c,  X=%d, Y=%d\n", data, m_CurX, m_CurY);
#endif
#endif

    window()->make_current();

    /* Test if we are processing an ESC Sequence */
    if (m_EscSeq)
    {
        /* Add the byte to the escape sequence */
        m_EscData[m_EscSeq++ - 1] = data;

        /* Test if time to process the ESC sequence */
        if (m_EscData[0] == 'Y')
        {
            //printf("ESCY:%d\n", m_EscSeq);
            if (m_EscSeq == 4)
            {
                /* Process the cursor position sequence */
                m_CurX = (m_EscData[2] - 0x20) * 6;
                if (m_Width == 40)
                    m_CurX <<= 1;
                m_CurY = (m_EscData[1] - 0x20) * 8;
                if (m_CurX < 0)
                    m_CurX = 0;
                if (m_CurY < 0)
                    m_CurY = 0;
                if (m_CurX > 474)
                    m_CurX = 474;
                if (m_CurY > 192)
                    m_CurY = 192;
                m_EscSeq = 0;
            }
        }
        else if (m_EscData[0] == ESC_SET_COLOR)
        {
            if (m_EscSeq == 3)
            {
                m_Color = (unsigned char) m_EscData[1];
                m_EscSeq = 0;
            }
        }
        else if ((m_EscData[0] == ESC_SET_COLOR_NAME) || m_EscData[0] == ESC_SET_BG_NAME)
        {
            if (data == 0x0A)
            {
                /* Do a color name lookup */
                /* First remove the 0D and 0A */
                m_EscSeq -= 2;
                while (m_EscData[m_EscSeq] == 0x0D || m_EscData[m_EscSeq] == 0x0A)
                {
                    m_EscData[m_EscSeq--] = 0;
                }

                color = FindColorByName(&m_EscData[1]);
                if (color != -1)
                {
                    if (m_EscData[0] == ESC_SET_COLOR_NAME)
                        m_Color = gNamedColors[color].cid;
                    else
                    {
                        m_BackgroundColor = fl_rgb_color(gNamedColors[color].red,
                                gNamedColors[color].green, gNamedColors[color].blue);
                        m_Colors[0] = m_BackgroundColor;
                        redraw();
                        Fl::check();
                    }
                }
                m_EscSeq = 0;
            }
        }
        else
        {
            /* Process the escape code */
            switch (m_EscData[0])
            {
            /* Turn Cursor ON */
            case ESC_CURSOR_ON:
                if (m_Cursor == 1)
                    break;

                /* Turn cursor on */
                m_Cursor = 1;
                if (m_CurY >= 200)
                    Scroll();
                XorCursor();
                break;

            /* Turn Cursor OFF */
            case ESC_CURSOR_OFF:
                if (m_Cursor == 0)
                    break;

                /* Turn cursor off */
                m_Cursor = 0;
                XorCursor();
                break;

            /* Test for Home Cursor */
            case ESC_CURSOR_HOME:
                m_CurX = m_CurY = 0;
                break;

            /* Test for Erase to EOL */
            case ESC_ERASE_EOL:
            case ESC_CLEAR_EOL:
                line = m_CurY >> 3;
                for (col = m_CurX; col < 480; col++)
                    SetByte(line, col, 0);
                break;

            case ESC_ERASE_SCREEN:
                Clear();
                break;

            case ESC_ERASE_LINE:
                line = m_CurY >> 3;
                for (col = 0; col < 480; col++)
                    SetByte(line, col, 0);
                break;

            case ESC_INS_LINE:
                for (y = 199; y > m_CurY+8; y--)
                //for (line = 24; line > (m_CurY >> 3); line--)
                {
                    for (col = 0; col < 480; col++)
                    {
                        //SetByte(line, col, pixdata[line-1][col]);
                        pixdata[y][col] = pixdata[y-8][col];
                        fl_color(m_Colors[pixdata[y][col]]);
                        drawpixel(col, y, 1);
                    }
                }
                line = m_CurY >> 3;
                for (col = 0; col < 480; col++)
                    SetByte(line, col, 0);
                break;

            case ESC_DEL_LINE:
                for (y = m_CurY; y < 200-8; y++)
                //for (line = m_CurY >> 3; line < 24; line++)
                {
                    for (col = 0; col < 480; col++)
                    {
                        //SetByte(line, col, pixdata[line+1][col]);
                        pixdata[y][col] = pixdata[y+8][col];
                        fl_color(m_Colors[pixdata[y][col]]);
                        drawpixel(col, y, 1);
                    }
                }

                for (col = 0; col < 480; col++)
                    SetByte(24, col, 0);
                break;

            case ESC_CURSOR_UP:
                m_CurY -= 8;
                break;

            case ESC_CURSOR_DOWN:
                m_CurY += 8;
                break;

            case ESC_CURSOR_LEFT:
                if (m_Width == 40)
                    m_CurX -= 12;
                else
                    m_CurX -= 6;
                break;

            case ESC_CURSOR_RIGHT:
                if (m_Width == 40)
                    m_CurX += 12;
                else
                    m_CurX += 6;
                break;

            case ESC_START_REVERSE:
                m_Reverse = 1;
                break;

            case ESC_END_REVERSE:
                m_Reverse = 0;
                break;

            case ESC_LOCK_DISPLAY:
                /* Not sure we need this */
                m_Locked = 1;
                break;

            case ESC_UNLOCK_DISPLAY:
                m_Locked = 0;
                break;

            case ESC_WIDTH_40:
                Clear();
                m_Width = 40;
                break;

            case ESC_WIDTH_80:
                Clear();
                m_Width = 80;
                break;

            case ESC_HELP:
                m_EscSeq = 0;
                Help();
                break;

            /* Not sure what this ESC does exactly */
            case 0x58:
                break;

            default:
                printf("ESC %02x\n", m_EscData[0]);
                break;
            }

            m_EscSeq = 0;
        }
    }
    else if (data == 0x1B)
    {
        m_EscSeq = 1;
    }

    /* Test for CLS */
    else if (data == 0x0C)
    {
        Clear();
    }

    /* Test for TAB */
    else if (data == 0x09)
    {
    }

    /* Test for BKSP */
    else if (data == 0x08)
    {
        if (m_CurX > 0)
        {
            if (m_Width == 40)
                m_CurX -= 12;
            else
                m_CurX -= 6;
        }
        else
        {
            if (m_Width == 40)
                m_CurX = 240-12;
            else
                m_CurX = 240-6;
            m_CurY -= 8;
        }
    }

    /* Test for DEL.  Nothing to do really */
    else if (data == 0x7F)
    {
    }

    /* Test for CR */
    else if (data == 0x0D)
    {
        /* Go to start of line */
        m_CurX = 0;
    }

    /* Test for "HOME" cursor */
    else if (data == 0x0B)
    {
        m_CurX = m_CurY = 0;
    }

    /* Test for End of Document */
    else if (data == 0x1A)
    {
    }

    /* Test for LF */
    else if (data == 0x0A)
    {
        /* Go to next line */
        m_CurY += 8;
    }

    else if (data < ' ')
    {
        printf("Unhandled %0X\n", data);
    }

    /* Draw the received byte */
    else //if (data >= ' ');
    {
        int addr, line, c, mem_index, column, lastcolor = -1;

        if (m_CurY >= 200)
            Scroll();

        addr = gStdRomDesc->sCharTable;
        if (data > 127)
            mem_index = addr + (128 - ' ') * 5 + (data - 128)*6;
        else
            mem_index = addr + (data - ' ') * 5;

        column = m_CurX;
        line = m_CurY / 8;

        /* Draw the byte */
        for (c = 0; c < 5; c++)
        {
            if (m_Reverse)
            {
                if (m_Width == 40)
                {
                    SetByte(line, column, ~gSysROM[mem_index]);
                    column++;
                }
                SetByte(line, column, ~gSysROM[mem_index++]);
            }
            else
            {
                if (m_Width == 40)
                {
                    SetByte(line, column, gSysROM[mem_index]);
                    column++;
                }
                SetByte(line, column, gSysROM[mem_index++]);
            }
            column++;
        }

        /* Now draw space between chars if needed */
        if (data > 127)
        {
            if (m_Reverse)
            {
                if (m_Width == 40)
                {
                    SetByte(line, column, ~gSysROM[mem_index]);
                }
                SetByte(line, column, ~gSysROM[mem_index++]);
            }
            else
            {
                if (m_Width == 40)
                {
                    SetByte(line, column, gSysROM[mem_index]);
                }
                SetByte(line, column, gSysROM[mem_index++]);
            }
        }
        else 
        {
            if (m_Reverse)
            {
                if (m_Width == 40)
                    SetByte(line, column, 0xFF);
                SetByte(line, column, 0xFF);
            }
            else
            {
                if (m_Width == 40)
                    SetByte(line, column, 0x0);
                SetByte(line, column, 0);
            }
        }

        /* Advance to the next character */
        if (m_Width == 40)
            m_CurX += 12;
        else
            m_CurX += 6;

        if (m_CurX >= 480)
        {
            m_CurX = 0;
            m_CurY += 8;
        }

        /* Draw cursor if it is on */
        if (m_Cursor)
        {
            XorCursor();
        }
    }
}

void VTTDockVid::XorCursor(void)
{
    int  line = m_CurY >> 3;
    int  col = m_CurX;
    int  num, x,y;

    if (m_CurY > 192)
        return;

    if (m_Width == 40)
        num = 12;
    else
        num = 6;

    window()->make_current();

    for (x = 0; x < num; x++)
    {
        for (y = m_CurY; y < m_CurY + 8; y++)
        {
            pixdata[y][col] = ~pixdata[y][col];
            fl_color(m_Colors[pixdata[y][col]]);
            drawpixel(col,y,1);
        }
        col++;
    }
}

void VTTDockVid::Scroll(void)
{
    int  y, col, color, lastcolor = -1;

    /* Test if at bottom of display */
    if (m_CurY >= 200)
    {
        /* We need to perform a scroll */
        for (y = 8; y < 200; y++)
        //for (line = 1; line < 25; line++)
        {
            for (col = 0; col < 480; col++)
            {
                color = pixdata[y][col];
                if (color != 0 || pixdata[y-8][col] != 0)
                {
                    pixdata[y-8][col] = color;
                    if (lastcolor != color)
                    {
                        fl_color(m_Colors[color]);
                        lastcolor = color;
                    }
                    drawpixel(col,y-8,1);
                }
                //if (pixdata[line][col] != 0 || pixdata[line-1][col] != 0)
                //    SetByte(line-1, col, pixdata[line][col]);
            }
        }

        /* Now zero out the bottom line */
        for (col = 0; col < 480; col++)
        {
            SetByte(24,col,0);
        }

        m_CurY -= 8;

        Fl::check();
    }
}

int VTTDockVid::handle(int event)
{
    return gpDisp->handle(event);
}

int VTTDockVid::FindColorByName(char *pName)
{
    int     x;

    /* Search all named colors for a match */
    for (x = 0; x < gColorCount; x++)
    {
        if (strcasecmp(gNamedColors[x].pName, pName) == 0)
        {
            return x;
        }
    }

    return -1;
}

void VTTDockVid::Help(void)
{
    int     x, c, len;

    Clear();

    /* Display all named colors */
    for (x = 0; x < gColorCount; x++)
    {
        len = strlen(gNamedColors[x].pName);
        if (m_CurX + len*6 > 480)
        {
            WriteData(0x0D);
            WriteData(0x0A);
        }

        for (c = 0; c < len; c++)
            WriteData(gNamedColors[x].pName[c]);
        if (x+1 != gColorCount)
        {
            if (m_CurX != 0)
                WriteData(',');
            if (m_CurX != 0)
                WriteData(' ');
        }
    }
}

