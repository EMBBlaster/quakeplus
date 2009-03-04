/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#include <ctype.h>

#include "quakedef.h"

/*

key up events are sent even if in console mode

*/


#define		MAXCMDLINE	256
char		key_lines[32][MAXCMDLINE];
int		key_linepos;
int		shift_down=false;
int		key_lastpress;

int		edit_line=0;
int		history_line=0;

keydest_t	key_dest;

int		key_count;		// incremented every key event

char		*keybindings[256];
bool	consolekeys[256];	// if true, can't be rebound while in console
bool	menubound[256];		// if true, can't be rebound while in menu
int		keyshift[256];		// key to map to if shift held down in console
int		key_repeats[256];	// if > 1, it is autorepeating
bool	keydown[256];

typedef struct
{
	char	*name;
	int	keynum;
} keyname_t;

keyname_t keynames[] =
{
	{(char*)"TAB", K_TAB},
	{(char*)"ENTER", K_ENTER},
	{(char*)"ESCAPE", K_ESCAPE},
	{(char*)"SPACE", K_SPACE},
	{(char*)"BACKSPACE", K_BACKSPACE},
	{(char*)"UPARROW", K_UPARROW},
	{(char*)"DOWNARROW", K_DOWNARROW},
	{(char*)"LEFTARROW", K_LEFTARROW},
	{(char*)"RIGHTARROW", K_RIGHTARROW},

	{(char*)"OPTION", K_ALT},

	{(char*)"CTRL", K_CTRL},
	{(char*)"SHIFT", K_SHIFT},
	
	{(char*)"F1", K_F1},
	{(char*)"F2", K_F2},
	{(char*)"F3", K_F3},
	{(char*)"F4", K_F4},
	{(char*)"F5", K_F5},
	{(char*)"F6", K_F6},
	{(char*)"F7", K_F7},
	{(char*)"F8", K_F8},
	{(char*)"F9", K_F9},
	{(char*)"F10", K_F10},
	{(char*)"F11", K_F11},
	{(char*)"F12", K_F12},

	{(char*)"INS", K_INS},
	{(char*)"DEL", K_DEL},
	{(char*)"PGDN", K_PGDN},
	{(char*)"PGUP", K_PGUP},
	{(char*)"HOME", K_HOME},
	{(char*)"END", K_END},

	{(char*)"MOUSE1", K_MOUSE1},
	{(char*)"MOUSE2", K_MOUSE2},
	{(char*)"MOUSE3", K_MOUSE3},


	{(char*)"MOUSE4", K_MOUSE4},
	{(char*)"MOUSE5", K_MOUSE5},

	{(char*)"CAPSLOCK", K_CAPSLOCK},
	{(char*)"COMMAND",  K_COMMAND},
	{(char*)"NUMLOCK",  K_NUMLOCK},
	{(char*)"F13",      K_F13},
	{(char*)"F14",      K_F14},
	{(char*)"F15",      K_F15},

	{(char*)"EQUAL_PAD",     K_EQUAL_PAD},
	{(char*)"SLASH_PAD",     K_SLASH_PAD},
	{(char*)"ASTERISK_PAD",  K_ASTERISK_PAD},
	{(char*)"MINUS_PAD",     K_MINUS_PAD},
	{(char*)"PLUS_PAD",      K_PLUS_PAD},
	{(char*)"ENTER_PAD",     K_ENTER_PAD},
	{(char*)"PERIOD_PAD",    K_PERIOD_PAD},
	{(char*)"NUM_0",         K_0_PAD},
	{(char*)"NUM_1",         K_1_PAD},
	{(char*)"NUM_2",         K_2_PAD},
	{(char*)"NUM_3",         K_3_PAD},
	{(char*)"NUM_4",         K_4_PAD},
	{(char*)"NUM_5",         K_5_PAD},
	{(char*)"NUM_6",         K_6_PAD},
	{(char*)"NUM_7",         K_7_PAD},
	{(char*)"NUM_8",         K_8_PAD},
	{(char*)"NUM_9",         K_9_PAD},

	{(char*)"JOY1", K_JOY1},
	{(char*)"JOY2", K_JOY2},
	{(char*)"JOY3", K_JOY3},
	{(char*)"JOY4", K_JOY4},

	{(char*)"AUX1", K_AUX1},
	{(char*)"AUX2", K_AUX2},
	{(char*)"AUX3", K_AUX3},
	{(char*)"AUX4", K_AUX4},
	{(char*)"AUX5", K_AUX5},
	{(char*)"AUX6", K_AUX6},
	{(char*)"AUX7", K_AUX7},
	{(char*)"AUX8", K_AUX8},
	{(char*)"AUX9", K_AUX9},
	{(char*)"AUX10", K_AUX10},
	{(char*)"AUX11", K_AUX11},
	{(char*)"AUX12", K_AUX12},
	{(char*)"AUX13", K_AUX13},
	{(char*)"AUX14", K_AUX14},
	{(char*)"AUX15", K_AUX15},
	{(char*)"AUX16", K_AUX16},
	{(char*)"AUX17", K_AUX17},
	{(char*)"AUX18", K_AUX18},
	{(char*)"AUX19", K_AUX19},
	{(char*)"AUX20", K_AUX20},
	{(char*)"AUX21", K_AUX21},
	{(char*)"AUX22", K_AUX22},
	{(char*)"AUX23", K_AUX23},
	{(char*)"AUX24", K_AUX24},
	{(char*)"AUX25", K_AUX25},
	{(char*)"AUX26", K_AUX26},
	{(char*)"AUX27", K_AUX27},
	{(char*)"AUX28", K_AUX28},
	{(char*)"AUX29", K_AUX29},
	{(char*)"AUX30", K_AUX30},
	{(char*)"AUX31", K_AUX31},
	{(char*)"AUX32", K_AUX32},

	{(char*)"PAUSE", K_PAUSE},

	{(char*)"MWHEELUP", K_MWHEELUP},
	{(char*)"MWHEELDOWN", K_MWHEELDOWN},

	{(char*)"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL,0}
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/


/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
void Key_Console (int key)
{
	char	*cmd;

	switch ( key )
	{
            case K_SLASH_PAD:
                    key = '/';
                    break;
            case K_MINUS_PAD:
                    key = '-';
                    break;
            case K_PLUS_PAD:
                    key = '+';
                    break;
            case K_0_PAD:
                    key = '0';
                    break;
            case K_1_PAD:
                    key = '1';
                    break;
            case K_2_PAD:
                    key = '2';
                    break;
            case K_3_PAD:
                    key = '3';
                    break;
            case K_4_PAD:
                    key = '4';
                    break;
            case K_5_PAD:
                    key = '5';
                    break;
            case K_6_PAD:
                    key = '6';
                    break;
            case K_7_PAD:
                    key = '7';
                    break;
            case K_8_PAD:
                    key = '8';
                    break;
            case K_9_PAD:
                    key = '9';
                    break;
            case K_PERIOD_PAD:
                    key = '.';
                    break;
            case K_ENTER_PAD:
                    key = K_ENTER;
                    break;
            case K_ASTERISK_PAD:
                    key = '*';
                    break;
            case K_EQUAL_PAD:
                    key = '=';
                    break;
	}
        
	if ((toupper (key) == 'V' && keydown[K_COMMAND]) || ((key == K_INS) && keydown[K_SHIFT]))
	{
                extern char *	Sys_GetClipboardData (void);
		char *		cbd;
		
		if ((cbd = Sys_GetClipboardData ()) != 0)
		{
			int i;

			strtok (cbd, "\n\r\b");

			i = strlen (cbd);
			if (i + key_linepos >= MAXCMDLINE)
                            i = MAXCMDLINE - key_linepos;

			if (i > 0)
			{
                            cbd[i]=0;
                            strcat (key_lines[edit_line], cbd);
                            key_linepos += i;
			}
//			free (cbd);
			delete [] cbd;
		}

		return;
	}

	if (key == K_ENTER)
	{
		Cbuf_AddText (key_lines[edit_line]+1);	// skip the >
		Cbuf_AddText ((char*)"\n");
		Con_Printf ((char*)"%s\n",key_lines[edit_line]);
		edit_line = (edit_line + 1) & 31;
		history_line = edit_line;
		key_lines[edit_line][0] = ']';
		key_linepos = 1;
		if (cls.state == ca_disconnected)
			SCR_UpdateScreen ();	// force an update, because the command
									// may take some time
		return;
	}

	if (key == K_TAB)
	{	// command completion
		cmd = Cmd_CompleteCommand (key_lines[edit_line]+1);
		if (!cmd)
			cmd = Cvar_CompleteVariable (key_lines[edit_line]+1);
		if (cmd)
		{
			strcpy (key_lines[edit_line]+1, cmd);
			key_linepos = strlen(cmd)+1;
			key_lines[edit_line][key_linepos] = ' ';
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
			return;
		}
	}
	
	if (key == K_BACKSPACE || key == K_LEFTARROW)
	{
		if (key_linepos > 1)
			key_linepos--;
		return;
	}

	if (key == K_UPARROW)
	{
		do
		{
			history_line = (history_line - 1) & 31;
		} while (history_line != edit_line
				&& !key_lines[history_line][1]);
		if (history_line == edit_line)
			history_line = (edit_line+1)&31;
		strcpy(key_lines[edit_line], key_lines[history_line]);
		key_linepos = strlen(key_lines[edit_line]);
		return;
	}

	if (key == K_DOWNARROW)
	{
		if (history_line == edit_line) return;
		do
		{
			history_line = (history_line + 1) & 31;
		}
		while (history_line != edit_line
			&& !key_lines[history_line][1]);
		if (history_line == edit_line)
		{
			key_lines[edit_line][0] = ']';
			key_linepos = 1;
		}
		else
		{
			strcpy(key_lines[edit_line], key_lines[history_line]);
			key_linepos = strlen(key_lines[edit_line]);
		}
		return;
	}

	if (key == K_PGUP || key==K_MWHEELUP)
	{
		con_backscroll += 2;
		if (con_backscroll > con_totallines - (vid.height>>3) - 1)
			con_backscroll = con_totallines - (vid.height>>3) - 1;
		return;
	}

	if (key == K_PGDN || key==K_MWHEELDOWN)
	{
		con_backscroll -= 2;
		if (con_backscroll < 0)
			con_backscroll = 0;
		return;
	}

	if (key == K_HOME)
	{
		con_backscroll = con_totallines - (vid.height>>3) - 1;
		return;
	}

	if (key == K_END)
	{
		con_backscroll = 0;
		return;
	}
	
	if (key < 32 || key > 127)
		return;	// non printable
		
	if (key_linepos < MAXCMDLINE-1)
	{
		key_lines[edit_line][key_linepos] = key;
		key_linepos++;
		key_lines[edit_line][key_linepos] = 0;
	}

}

//============================================================================

char chat_buffer[32];
bool team_message = false;

void Key_Message (int key)
{
	static int chat_bufferlen = 0;

	if (key == K_ENTER)
	{
		if (team_message)
			Cbuf_AddText ((char*)"say_team \"");
		else
			Cbuf_AddText ((char*)"say \"");
		Cbuf_AddText(chat_buffer);
		Cbuf_AddText((char*)"\"\n");

		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key == K_ESCAPE)
	{
		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
		{
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
		return;
	}

	if (chat_bufferlen == 31)
		return; // all full

	chat_buffer[chat_bufferlen++] = key;
	chat_buffer[chat_bufferlen] = 0;
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum (char *str)
{
	keyname_t	*kn;
	
	if (!str || !str[0])
		return -1;
	if (!str[1])
		return str[0];

        if(!strcasecmp(str, (char*)"ALT"))
        {
            for(kn=keynames ; kn->name ; kn++)
            {
                if(!strcasecmp((char*)"OPTION", kn->name))
                    return(kn->keynum);
            }
        }

	for (kn=keynames ; kn->name ; kn++)
	{
		if (!strcasecmp(str,kn->name))
			return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;	
	static	char	tinystr[2];
	
	if (keynum == -1)
		return (char*)"<KEY NOT FOUND>";
	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}
	
	for (kn=keynames ; kn->name ; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return (char*)"<UNKNOWN KEYNUM>";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, char *binding)
{
	char	*_new;
	int		l;
			
	if (keynum == -1)
		return;

// free old bindings
	if (keybindings[keynum])
	{
		Z_Free (keybindings[keynum]);
		keybindings[keynum] = NULL;
	}
			
// allocate memory for new binding
	l = strlen (binding);	
	_new = (char*)Z_Malloc (l+1);
	strcpy (_new, binding);
	_new[l] = 0;
	keybindings[keynum] = _new;
        if (keynum == K_F12)
        {
            extern void	IN_SetF12EjectEnabled (bool theState);
            
            IN_SetF12EjectEnabled (keybindings[keynum][0] == 0x00);
        }
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ((char*)"unbind <key> : remove commands from a key\n");
		return;
	}
	
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Con_Printf ((char*)"\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, (char*)"");
}

void Key_Unbindall_f (void)
{
	int		i;
	
	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			Key_SetBinding (i, (char*)"");
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];
	
	c = Cmd_Argc();

	if (c != 2 && c != 3)
	{
		Con_Printf ((char*)"bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Con_Printf ((char*)"\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Con_Printf ((char*)"\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b] );
		else
			Con_Printf ((char*)"\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}
	
// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
		if (i > 2)
			strcat (cmd, " ");
		strcat (cmd, Cmd_Argv(i));
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int		i;

	for (i=0 ; i<256 ; i++)
		if (keybindings[i])
			if (*keybindings[i])
				fprintf (f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
}


/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;

	for (i=0 ; i<32 ; i++)
	{
		key_lines[i][0] = ']';
		key_lines[i][1] = 0;
	}
	key_linepos = 1;
	
//
// init ascii characters in console mode
//
	for (i=32 ; i<128 ; i++)
		consolekeys[i] = true;
	consolekeys[K_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;
	consolekeys['`'] = false;
	consolekeys['~'] = false;
        
        consolekeys[K_EQUAL_PAD] = true;
        consolekeys[K_SLASH_PAD] = true;
	consolekeys[K_ASTERISK_PAD] = true;
	consolekeys[K_MINUS_PAD] = true;
	consolekeys[K_PLUS_PAD] = true;
	consolekeys[K_ENTER_PAD] = true;
	consolekeys[K_PERIOD_PAD] = true;
	consolekeys[K_0_PAD] = true;
	consolekeys[K_1_PAD] = true;
	consolekeys[K_2_PAD] = true;
	consolekeys[K_3_PAD] = true;
	consolekeys[K_4_PAD] = true;
	consolekeys[K_5_PAD] = true;
	consolekeys[K_6_PAD] = true;
	consolekeys[K_7_PAD] = true;
	consolekeys[K_8_PAD] = true;
	consolekeys[K_9_PAD] = true;

	for (i=0 ; i<256 ; i++)
		keyshift[i] = i;
	for (i='a' ; i<='z' ; i++)
		keyshift[i] = i - 'a' + 'A';
	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';

	menubound[K_ESCAPE] = true;
	for (i=0 ; i<12 ; i++)
		menubound[K_F1+i] = true;

//
// register our functions
//
	Cmd_AddCommand ((char*)"bind",Key_Bind_f);
	Cmd_AddCommand ((char*)"unbind",Key_Unbind_f);
	Cmd_AddCommand ((char*)"unbindall",Key_Unbindall_f);


}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
void Key_Event (int key, bool down)
{
	char	*kb;
	char	cmd[1024];

	keydown[key] = down;

	if (!down)
		key_repeats[key] = 0;

	key_lastpress = key;
	key_count++;
	if (key_count <= 0)
	{
		return;		// just catching keys for Con_NotifyBox
	}

// update auto-repeat status
	if (down)
	{
                extern int	Sys_CheckSpecialKeys (int theKey);
            
                if (Sys_CheckSpecialKeys (key) != 0)
                {
                    return;
                }
		key_repeats[key]++;
		if (key != K_BACKSPACE && key != K_PAUSE && key_repeats[key] > 1)
		{
			return;	// ignore most autorepeats
		}
			
		if (key >= 200 && !keybindings[key])
			Con_Printf ((char*)"%s is unbound, hit F4 to set.\n", Key_KeynumToString (key) );
	}

	if (key == K_SHIFT)
		shift_down = down;

//
// handle escape specialy, so the user can never unbind it
//
	if (key == K_ESCAPE)
	{
		if (!down)
			return;
		switch (key_dest)
		{
		case key_message:
			Key_Message (key);
			break;
		case key_menu:
			M_Keydown (key);
			break;
		case key_game:
		case key_console:
			M_ToggleMenu_f ();
			break;
		default:
			Sys_Error ((char*)"Bad key_dest");
		}
		return;
	}

//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
	if (!down)
	{
		kb = keybindings[key];
		if (kb && kb[0] == '+')
		{
			snprintf (cmd, 1024, "-%s %i\n", kb+1, key);
			Cbuf_AddText (cmd);
		}
		if (keyshift[key] != key)
		{
			kb = keybindings[keyshift[key]];
			if (kb && kb[0] == '+')
			{
				snprintf (cmd, 1024, "-%s %i\n", kb+1, key);
				Cbuf_AddText (cmd);
			}
		}
		return;
	}

//
// during demo playback, most keys bring up the main menu
//
	if (cls.demoplayback && down && consolekeys[key] && key_dest == key_game)
	{
		M_ToggleMenu_f ();
		return;
	}

//
// if not a consolekey, send to the interpreter no matter what mode is
//
	if ( (key_dest == key_menu && menubound[key])
	|| (key_dest == key_console && !consolekeys[key])
	|| (key_dest == key_game && ( !con_forcedup || !consolekeys[key] ) ) )
	{
		kb = keybindings[key];
		if (kb)
		{
			if (kb[0] == '+')
			{	// button commands add keynum as a parm
				snprintf (cmd, 1024, "%s %i\n", kb, key);
				Cbuf_AddText (cmd);
			}
			else
			{
				Cbuf_AddText (kb);
				Cbuf_AddText ((char*)"\n");
			}
		}
		return;
	}

	if (!down)
		return;		// other systems only care about key down events

	if (shift_down)
	{
		key = keyshift[key];
	}

	switch (key_dest)
	{
	case key_message:
		Key_Message (key);
		break;
	case key_menu:
		M_Keydown (key);
		break;

	case key_game:
	case key_console:
		Key_Console (key);
		break;
	default:
		Sys_Error ((char*)"Bad key_dest");
	}
}


/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	for (i=0 ; i<256 ; i++)
	{
		keydown[i] = false;
		key_repeats[i] = 0;
	}
}
