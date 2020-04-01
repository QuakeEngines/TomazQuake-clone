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
#include "quakedef.h"
#include "winquake.h"
#include "menu.h"

/*
=========
Quit Menu
=========
*/

int			msgNumber;
int			m_quit_prevstate;
qboolean	wasInMenus;

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = rand()&7;
}

void M_Quit_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case 'Y':
	case 'y':
		key_dest = key_console;
		Host_Quit_f ();
		break;

	default:
		break;
	}
}

void M_Quit_Draw (void)
{
	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

	M_DrawTextBox (0, 0, 38, 23);
	M_PrintWhite (16, 12,	"         TQ 1.48 By Tomaz         \n");
	M_PrintWhite (16, 28,	"         Special Thanks To        \n");
	M_Print	(16, 44,		"  LordHavoc     Heffo     Fett    \n");
	M_Print (16, 52,		"  Phoenix       Radix     MrG     \n");
	M_Print (16, 60,		"  FrikaC        Muff      Vic     \n");
	M_PrintWhite (16, 76,	"        Additional Thanks To      \n");
	M_Print (16, 92,		"  scar3crow     CocoT     BramBo  \n");
	M_Print (16, 100,		"  ArchAngel     Koolio    Quest   \n");
	M_Print (16, 108,		"  MrPeanut      P5YCHO    Ender   \n");
	M_Print (16, 116,		"  Atomizer      Deetee    Topaz   \n");
	M_Print (16, 124,		"  illusion      Midgar    jAGO    \n");
	M_Print (16, 132,		"  KrimZon       Akuma     Horn    \n");
	M_Print (16, 140,		"  dakilla       Krust     Harb    \n");
	M_PrintWhite (16, 180,	"       Press Y To Stop Reading    \n");
}