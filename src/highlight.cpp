/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "highlight.h"
#include "multieditwin.h"

//Editor colors
#if 0
Fl_Color hl_plain = FL_BLACK;
Fl_Color hl_linecomment = FL_DARK_GREEN;
Fl_Color hl_blockcomment = FL_DARK_GREEN;
Fl_Color hl_string = FL_BLUE;
Fl_Color hl_directive = FL_DARK_MAGENTA;
Fl_Color hl_type = FL_DARK_RED;
Fl_Color hl_keyword = FL_BLUE;
Fl_Color hl_character = FL_DARK_RED;
Fl_Color hl_label = FL_DARK_BLUE;
Fl_Color background_color = FL_WHITE;
#endif

Fl_Color hl_plain = FL_WHITE;
Fl_Color hl_linecomment = (Fl_Color) 95;
Fl_Color hl_blockcomment = (Fl_Color) 93;
Fl_Color hl_string = (Fl_Color) 219;
Fl_Color hl_directive = (Fl_Color) 219;
Fl_Color hl_type = (Fl_Color) 74;
Fl_Color hl_keyword = (Fl_Color) 220;
Fl_Color hl_character = (Fl_Color) 75;
Fl_Color hl_label = (Fl_Color) 116;
Fl_Color background_color = FL_BLACK;

extern int gDisableHl;

const char         *code_keywords[] = {	// List of known C/C++ keywords...
		     "and",
		     "and_eq",
		     "asm",
		     "bitand",
		     "bitor",
		     "break",
		     "case",
		     "catch",
		     "compl",
		     "continue",
		     "default",
		     "delete",
		     "do",
		     "else",
		     "false",
		     "for",
		     "goto",
		     "if",
		     "new",
		     "not",
		     "not_eq",
		     "operator",
		     "or",
		     "or_eq",
		     "return",
		     "switch",
		     "template",
		     "this",
		     "throw",
		     "true",
		     "try",
		     "using",
		     "while",
		     "xor",
		     "xor_eq"
};

const char         *asm_code_keywords[] = {
			 "aci",
			 "adc",
			 "add",
			 "adi",
			 "ana",
			 "ani",
			 "arhl",
			 "ashr",
			 "bc",
			 "bm",
			 "bnc",
			 "bnz",
			 "bp",
			 "bpe",
			 "br",
			 "bra",
			 "bz",
			 "call",
			 "cc",
			 "cm",
			 "cma",
			 "cmc",
			 "cmp",
			 "cnc",
			 "cnz",
			 "cp",
			 "cpe",
			 "cpi",
			 "cpo",
			 "cz",
			 "daa",
			 "dad",
			 "dcr",
			 "dcx",
			 "dehl",
			 "desp",
			 "di",
			 "dsub",
			 "ei",
			 "hlmbc",
			 "hlt",
			 "in",
			 "inr",
			 "inx",
			 "jc",
			 "jd",
			 "jk",
			 "jm",
			 "jmp",
			 "jnc",
			 "jnd",
			 "jnk",
			 "jnx",
			 "jnx5",
			 "jnz",
			 "jp",
			 "jpe",
			 "jpo",
			 "jtm",
			 "jtp",
			 "jx",
			 "jx5",
			 "jz",
			 "lcall",
			 "lda",
			 "ldax",
			 "ldeh",
			 "ldes",
			 "ldhi",
			 "ldsi",
			 "lhld",
			 "lhlde",
			 "lhli",
			 "lhlx",
			 "ljmp",
			 "lpop",
			 "lpush",
			 "lret",
			 "lxi",
			 "mov",
			 "mvi",
			 "nop",
			 "ora",
			 "ori",
			 "out",
			 "pchl",
			 "pop",
			 "push",
			 "ral",
			 "rar",
			 "rc",
			 "rcall",
			 "rdel",
			 "ret",
			 "rim",
			 "rlc",
			 "rlde",
			 "rm",
			 "rnc",
			 "rnz",
			 "rp",
			 "rpe",
			 "rpg",
			 "rpo",
			 "rrc",
			 "rrhl",
			 "rst",
			 "rstv",
			 "rz",
			 "sbb",
			 "sbc",		// Short Branch Carry
			 "sbi",
			 "sbnc",	// Short Branch No Carry
			 "sbnz",	// Short Branch No zero
			 "sbz",		// Short Branch if Zero
			 "shld",
			 "shlde",
			 "shli",
			 "shlr",
			 "shlx",
			 "sim",
			 "spg",
			 "sphl",
			 "spi",
			 "sta",
			 "stax",
			 "stc",
			 "sub",
			 "sui",
			 "xchg",
			 "xra",
			 "xri",
			 "xthl"
		   };
const char         *code_types[] = {	// List of known C/C++ types...
		     "auto",
		     "bool",
		     "char",
		     "class",
		     "const",
		     "const_cast",
		     "double",
		     "dynamic_cast",
		     "enum",
		     "explicit",
		     "extern",
		     "float",
		     "friend",
		     "inline",
		     "int",
		     "long",
		     "mutable",
		     "namespace",
		     "private",
		     "protected",
		     "public",
		     "register",
		     "short",
		     "signed",
		     "sizeof",
		     "static",
		     "static_cast",
		     "struct",
		     "template",
		     "typedef",
		     "typename",
		     "union",
		     "unsigned",
		     "virtual",
		     "void",
		     "volatile"
};

const char         *asm_code_types[] = {	// List of known C/C++asm types...
			 "and",
			 "aseg",
			 "cseg",
			 "db",
			 "ds",
			 "dseg",
			 "dw",
			 "echo",
			 "else",
			 "elsif",
			 "end",
			 "endif",
			 "equ",
			 "extern",
			 "extrn",
			 "fill",
			 "if",
			 "include",
			 "link",
			 "list",
			 "lsfirst",
			 "maclib",
			 "module",
			 "msfirst",
			 "name",
			 "nopage",
			 "not",
			 "or",
			 "org",
			 "page",
			 "printf",
			 "public",
			 "set",
			 "stkln",
			 "sym",
			 "text",
			 "title",
			 "xor"
		   };
//
// 'style_parse()' - Parse text and produce style data.
//
//
void style_parse(const char *text, char *style, int length) 
{
	char	   current; 
    int	       last;
    char	   buf[255],
               *bufptr;
    const char *temp;
	int			col = 0;

    for (current = *style, last = 0; length > 0; length --, text ++) 
	{
		//if ((current != 'C') && (current != 'D') && (current != 'E')) current = 'A';
		//if ((current == 'B')) current = 'A';
	  
		if (current == 'A') 
		{
		  // Check for directives, comments, strings, and keywords...
      		if (*text == '#') 
			{
        		// Set style to directive
				current = 'E';
			} 
		
			else if ((strncmp(text, "//", 2) == 0) || (*text == ';'))
			{
				current = 'B';
				for (; length > 0 && *text != '\n'; length --, text ++) 
					*style++ = 'B';

				if (length == 0) break;
      		} 

			else if (strncmp(text, "/*", 2) == 0) 
			{
				current = 'C';
				*style++ = current;
				*style--;
			} 

			else if (strncmp(text, "\\\"", 2) == 0) 
			{
				// Quoted quote...
				*style++ = current;
				*style++ = current;
				text ++;
				length --;
				continue;
      		} 

			else if (*text == '\"' /*| *text == '\''*/) 
			{
        		current = 'D';
      		} 

			else if (*text == '\'') 
			{
        		current = 'H';
      		} 

			else if (!last && /*islower(*text) && */ text)// && 
//				!(isalnum(*(text-1)) || *(text-1)=='_')) 
			{
        		// Might be a keyword...
				for (temp = text, bufptr = buf; ((*temp > 0) && (isalnum(*temp) || *temp=='_')) && 
					bufptr < (buf + sizeof(buf) - 1); *bufptr++ = tolower(*temp++));
				{
					//if (!islower(*temp)) 
					{
						*bufptr = '\0';
						bufptr = buf;

						if (bsearch(&bufptr, asm_code_types, sizeof(asm_code_types) / 
							sizeof(asm_code_types[0]), sizeof(asm_code_types[0]), compare_keywords)) 
						{
							while (text < temp) 
							{
					  			*style++ = 'F';
								text ++;
			      				length --;
							}
	    					text --;
	    					length ++;
	    					last = 1;
	    					continue;

		  				} 
						else if (bsearch(&bufptr, asm_code_keywords, sizeof(asm_code_keywords) / 
							sizeof(asm_code_keywords[0]), sizeof(asm_code_keywords[0]), compare_keywords)) 
						{
		   					while (text < temp) 
							{
		     					*style++ = 'G';
		     					text ++;
		     					length --;
		   					}
			
							text --;
							length ++;
							last = 1;
							continue;
		  				}
					}
				}

				/* Test if label */
				if ((col == 0) && !isspace(*text))
				{
					current = 'I';
				}
    		}


 		} 
		else if (current == 'C' && strncmp(text, "*/", 2) == 0) 
		{
   			// Close a C comment... 
	 		*style++ = current;
   			*style++ = current;
    		text ++;
   			length --;
    		current = 'A';
   			continue;
 		} 
		else if (current == 'D') 
		{
   			// Continuing in string...
   			if (strncmp(text, "\\\"", 2) == 0 || strncmp(text, "\\\'", 2) == 0) 
			{
      			// Quoted end quote...
				*style++ = current; 
				*style++ = current;
				text ++;
				length --;
				continue;
   			} 
			else if (*text == '\"'/* || *text == '\''*/) 
			{
       			// End quote...
				*style++ = current;
				current = 'A';
				continue;
   			}
 		} 
		else if (current == 'H') 
		{
   			// Continuing in char...
   			if (strncmp(text, "\\\'", 2) == 0) {
      			// Quoted end quote...
				*style++ = current; 
				*style++ = current;
				text ++;
				length --;
				continue;
   			} else if (*text == '\'') {
     			// End quote...
				*style++ = current;
				current = 'A';
				continue;
   			}
   		}
		else if (current == 'I')
		{
			// Continuing in label...
			col++;
			if ((*text == ':') || (*text == ' ') || (*text == '\t'))
			{
     			// End label...
				*style++ = current;
				current = 'A';
				last = 0;
				continue;
			}
			*style++ = current;
			continue;
		}


   		// Copy style info...
   		if (current == 'A' && (*text == '{' || *text == '}')) *style++ = 'G';
   		/*else if(current == 'E' && strncmp(text, "/*", 2) == 0) {
   			*style++ = 'C';
   			*style++ = 'C';
   			//*style--;
   		}*/
   		else if(current == 'E' && strncmp(text, "/*", 2) == 0)
   		{
   			*style++ = 'C';      			
			current = 'C';
   		}
   		else if(current == 'E' && strncmp(text, "//", 2) == 0)
   		{
   			*style++ = 'B';      			
			current = 'B';
   		}
   		else if((current == 'E') && (text[0] == ';'))
   		{
   			*style++ = 'B';      			
			current = 'B';
   		}
   		else *style++ = current;

   		last = (*text > 0) && (isalnum(*text) ||  *text == '_');
		if (*text != 0x0a)
			col++;

   		if (*text == '\n') 
		{
     		// Reset column and possibly reset the style
     			if (current != 'D' && current != 'C') current = 'A';
			col = 0;
   		}
	} //for
}

int compare_keywords(const void *a, const void *b) 
{
  return (strcmp(*((const char **)a), *((const char **)b)));
}


/*
==========================================================================
style_init() - Initialize the style buffer...
==========================================================================
*/
void style_init(HighlightCtrl_t *pHlCtrl) 
{
  char *style = new char[pHlCtrl->textbuf->length() + 1];
  char *text = pHlCtrl->textbuf->text();
  

  memset(style, 'A', pHlCtrl->textbuf->length());
  style[pHlCtrl->textbuf->length()] = '\0';

  if (!pHlCtrl->stylebuf) 
	  pHlCtrl->stylebuf = new My_Text_Buffer(pHlCtrl->textbuf->length());

  if (pHlCtrl->cppfile) 
	  style_parse(text, style, pHlCtrl->textbuf->length());

  pHlCtrl->stylebuf->text(style);
  pHlCtrl->stylebuf->canUndo(0);
  delete[] style;
  free(text);
}


/*
==========================================================================
style_unfinished_cb() - Update unfinished styles
==========================================================================
*/
void style_unfinished_cb(int, void*) 
{
}


//
// 'style_update()' - Update the style buffer...
//

void
old_style_update(	int        pos,		// I - Position of update
             	int        nInserted,	// I - Number of inserted chars
	     		int        nDeleted,	// I - Number of deleted chars
             	int        /*nRestyled*/,	// I - Number of restyled chars
	     		const char * /*deletedText*/,// I - Text that was deleted
             	void       *cbArg) {	// I - Callback data
/*  int	start,				// Start of text
	end;				// End of text
  char	last,				// Last style on line
	stringdeleted=0;
  char *style,				// Style data
	*text;				// Text data
*/
/*
  // If this is just a selection change, just unselect the style buffer...
  if (nInserted == 0 && nDeleted == 0) {
    stylebuf->unselect();
    return;
  }

  // Track changes in the text buffer...
  if (nInserted > 0) {
    // Insert characters into the style buffer...
    style = (char*)malloc(nInserted + 1);
    memset(style, 'A', nInserted);
    style[nInserted] = '\0'; 

    stylebuf->replace(pos, pos + nDeleted, style);
    free(style);
  } else {
    // Just delete characters in the style buffer...
    if((stylebuf->character(pos) == 'D') || (stylebuf->character(pos) == 'C')) stringdeleted = 1;
    
    stylebuf->remove(pos, pos + nDeleted);
    if(pos < 2) style_init();
  }

	 
	
  // Select the area that was just updated to avoid unnecessary
  // callbacks...
  stylebuf->select(pos, pos + nInserted - nDeleted);

  // Re-parse the changed region; we do this by parsing from the
  // beginning of the line of the changed region to the end of
  // the line of the changed region...  Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character...
  start = textbuf->line_start(pos);
  end   = textbuf->line_end(pos + nInserted);
  text  = textbuf->text_range(start, end);
  style = stylebuf->text_range(start, end);
  last  = style[end - start - 1];


  style_parse(text, style, end - start);


  stylebuf->replace(start, end, style);
  ((Fl_Text_Editor_ext *)cbArg)->redisplay_range(start, end);

  if ((last != style[end - start - 1]) || nDeleted || style[end - start - 1] == 'D') 
  if (last == 'C' || last == 'D') 
  //if(update_count > 10)
  {
    // The last character on the line changed styles, so reparse the
    // remainder of the buffer...
    free(text);
    free(style);

    start = 0;
    end   = textbuf->length();
    text  = textbuf->text_range(start, end);
    style = stylebuf->text_range(start, end);

    style_parse(text, style, end - start);
    
  	//if(update_count > 10)
  	{
  		update_count = 0;
  		if(browser_nav_grp->visible()) navigator_update(text,style,end - start);
  	} 

    stylebuf->replace(start, end, style);
    //((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);
    
	te->redraw();
  }
  update_count+= nInserted > nDeleted ? nInserted : nDeleted;

  free(text);
  free(style);
*/
}


void
style_update(	int        pos,		// I - Position of update
             	int        nInserted,	// I - Number of inserted chars
	     		int        nDeleted,	// I - Number of deleted chars
             	int        /*nRestyled*/,	// I - Number of restyled chars
	     		const char * /*deletedText*/,// I - Text that was deleted
             	void       *cbArg) 
{
	int		start,				// Start of text
			end;				// End of text
	char	last,				// Last style on line
			stringdeleted=0;
	char	*style,				// Style data
			*text;				// Text data

	if (gDisableHl)
		return;

	HighlightCtrl_t	*pHlCtrl;

	pHlCtrl = (HighlightCtrl_t *) cbArg;

    /* Test if highlight disabled for this buffer */
    if (pHlCtrl->stylebuf == NULL)
        return;

	// If this is just a selection change, just unselect the style buffer...
	if (nInserted == 0 && nDeleted == 0) 
	{
		pHlCtrl->stylebuf->unselect();
		return;
	}

	// Track changes in the text buffer...
	if (nInserted > 0) 
	{
	    // Insert characters into the style buffer...
	    style = (char*) malloc(nInserted + 1);
	    memset(style, 'A', nInserted);
	    style[nInserted] = '\0'; 

	    pHlCtrl->stylebuf->replace(pos, pos + nDeleted, style);
	    free(style);
	} 
	else 
	{
	    // Just delete characters in the style buffer...
	    if((pHlCtrl->stylebuf->character(pos) == 'D') || (pHlCtrl->stylebuf->character(pos) == 'C')) stringdeleted = 1;
    
	    pHlCtrl->stylebuf->remove(pos, pos + nDeleted);
	    if(pos < 2) 
			style_init(pHlCtrl);
	}

  // Select the area that was just updated to avoid unnecessary
  // callbacks...
  pHlCtrl->stylebuf->select(pos, pos + nInserted - nDeleted);

  // Re-parse the changed region; we do this by parsing from the
  // beginning of the line of the changed region to the end of
  // the line of the changed region...  Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character...
  start = pHlCtrl->textbuf->line_start(pos);
  end   = pHlCtrl->textbuf->line_end(pos + nInserted);
  text  = pHlCtrl->textbuf->text_range(start, end);
  style = pHlCtrl->stylebuf->text_range(start, end);
  last  = style[end - start - 1];

  style_parse(text, style, end - start);

  pHlCtrl->stylebuf->replace(start, end, style);
  pHlCtrl->te->redisplay_range(start, end);

  //if ((last != style[end - start - 1]) || nDeleted || style[end - start - 1] == 'D') 
  //if(update_count > 10)
  {
    // The last character on the line changed styles, so reparse the
    // remainder of the buffer...
    free(text);
    free(style);

    start = 0;
    end   = pHlCtrl->textbuf->length();
    text  = pHlCtrl->textbuf->text_range(start, end);
    style = pHlCtrl->stylebuf->text_range(start, end);

    style_parse(text, style, end - start);
    
//	add_nav_timeout_handler();
  	//if(update_count > 10)
  	{
  		//update_count = 0;
  		//if(browser_nav_grp->visible()) navigator_update(text,style,end - start);
  	} 

    pHlCtrl->stylebuf->replace(start, end, style);
    //((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);
    
	pHlCtrl->te->redraw();
  }
  //update_count+= nInserted > nDeleted ? nInserted : nDeleted;

  free(text);
  free(style);
}




