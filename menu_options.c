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
============
Options Menu
============
*/

extern cvar_t brightness;

int		options_cursor;
#define	OPTIONS_ITEMS	4

void M_Menu_Options_f (void)
{
	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;
}

void M_Options_Draw (void)
{
	qpic_t	*p;

	M_DrawPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	M_Print (16, 32, "              Controls");
	M_Print (16, 40, "                 Sound");
	M_Print (16, 48, "                 Video");
	M_Print (16, 56, "                  Misc");
// cursor
	M_DrawCharacter (200, 32 + options_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Options_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		switch (options_cursor)
		{
		case 0:
			M_Menu_Controls_f ();
			break;
		case 1:
			M_Menu_Sound_f ();
			break;
		case 2:
			M_Menu_Video_f ();
			break;
		case 3:
			M_Menu_Misc_f ();
			break;
		}
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor++;
		if (options_cursor >= OPTIONS_ITEMS)
			options_cursor = 0;
		break;
	}
}

/*
==========
Controls Menu
==========
*/

int		controls_cursor;
#define	CONTROLS_ITEMS	6

void M_Menu_Controls_f () 
{
	key_dest = key_menu;
	m_state = m_controls;
	m_entersound = true;
}

void M_ControlsAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (controls_cursor)
	{
	case 0:
		M_Menu_Keys_f ();
		break;

	case 1:	// mouse speed
		sensitivity.value += dir * 0.5;
		if (sensitivity.value < 1)
			sensitivity.value = 1;
		if (sensitivity.value > 11)
			sensitivity.value = 11;
		Cvar_SetValue ("sensitivity", sensitivity.value);
		break;

	case 2:	// allways run
		if (cl_forwardspeed.value > 200)
		{
			Cvar_SetValue ("cl_forwardspeed", 200);
			Cvar_SetValue ("cl_backspeed", 200);
		}
		else
		{
			Cvar_SetValue ("cl_forwardspeed", 400);
			Cvar_SetValue ("cl_backspeed", 400);
		}
		break;

	case 3:	// invert mouse
		Cvar_SetValue ("m_pitch", -m_pitch.value);
		break;

	case 4:	// lookspring
		Cvar_SetValue ("lookspring", !lookspring.value);
		break;

	case 5:	// lookstrafe
		Cvar_SetValue ("lookstrafe", !lookstrafe.value);
		break;
	}
}

void M_Controls_Draw () 
{
	float	r;

	M_Print (16, 32, "             Bind Keys");

	M_Print (16, 40, "           Mouse Speed");
	r = (sensitivity.value - 1) * 0.1;	// Tomaz - Speed
	M_DrawSlider (220, 40, r);

	M_Print (16, 48, "            Always Run");
	M_DrawCheckbox (220, 48, cl_forwardspeed.value > 200);

	M_Print (16, 56, "          Invert Mouse");
	M_DrawCheckbox (220, 56, m_pitch.value < 0);

	M_Print (16, 64, "            Lookspring");
	M_DrawCheckbox (220, 64, lookspring.value);

	M_Print (16, 72, "            Lookstrafe");
	M_DrawCheckbox (220, 72, lookstrafe.value);

	M_DrawCharacter (200, 32 + controls_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Controls_Key (int key) 
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_ControlsAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--controls_cursor < 0)
			controls_cursor = CONTROLS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++controls_cursor >= CONTROLS_ITEMS)
			controls_cursor = 0;
		break;

	case K_LEFTARROW:
		M_ControlsAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_ControlsAdjustSliders (1);
		break;
	}
}

/*
=========
Keys Menu
=========
*/

char *bindnames[][2] =
{
{"+attack", 		"attack"},
{"impulse 10", 		"change weapon"},
{"+jump", 			"jump / swim up"},
{"+forward", 		"walk forward"},
{"+back", 			"backpedal"},
{"+left", 			"turn left"},
{"+right", 			"turn right"},
{"+speed", 			"run"},
{"+moveleft", 		"step left"},
{"+moveright", 		"step right"},
{"+strafe", 		"sidestep"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"centerview", 		"center view"},
{"+mlook", 			"mouse look"},
{"+klook", 			"keyboard look"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"}
};

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))

int		keys_cursor;
int		bind_grab;

void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}

void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}


void M_Keys_Draw (void)
{
	int		i, l;
	int		keys[2];
	char	*name;
	int		x, y;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	if (bind_grab)
		M_Print (12, 32, "Press a key or button for this action");
	else
		M_Print (18, 32, "Enter to change, backspace to clear");

// search for known bindings
	for (i=0 ; i<NUMCOMMANDS ; i++)
	{
		y = 48 + 8*i;

		M_Print (16, y, bindnames[i][1]);

		l = strlen (bindnames[i][0]);

		M_FindKeysForCommand (bindnames[i][0], keys);

		if (keys[0] == -1)
		{
			M_Print (140, y, "???");
		}
		else
		{
			name = Key_KeynumToString (keys[0]);
			M_Print (140, y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				M_Print (140 + x + 8, y, "or");
				M_Print (140 + x + 32, y, Key_KeynumToString (keys[1]));
			}
		}
	}

	if (bind_grab)
		M_DrawCharacter (130, 48 + keys_cursor*8, '=');
	else
		M_DrawCharacter (130, 48 + keys_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[2];

	if (bind_grab)
	{	// defining a key
		S_LocalSound ("misc/menu1.wav");
		if (k == K_ESCAPE)
		{
			bind_grab = false;
		}
		else if (k != '`')
		{
			_snprintf (cmd, sizeof(cmd),"bind \"%s\" \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor][0]);
			Cbuf_InsertText (cmd);
		}

		bind_grab = false;
		return;
	}

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor++;
		if (keys_cursor >= NUMCOMMANDS)
			keys_cursor = 0;
		break;

	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("misc/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		bind_grab = true;
		break;

	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
		S_LocalSound ("misc/menu2.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}
}

/*
==========
Sound Menu
==========
*/

int	sound_cursor;
#define	SOUND_ITEMS	5

void M_Menu_Sound_f (void)
{
	key_dest = key_menu;
	m_state = m_sound;
	m_entersound = true;
}

void M_SoundAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (sound_cursor)
	{
	case 0:	// music volume
		break;

	case 1:	// sfx volume
		break;

	case 2:	// mp3 or cd
		break;

	case 3: // samplerate
		break;

	case 4: // restart
		break;
	}
}

void M_Sound_Draw () 
{
}
void M_Sound_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_SoundAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--sound_cursor < 0)
			sound_cursor = SOUND_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++sound_cursor >= SOUND_ITEMS)
			sound_cursor = 0;
		break;

	case K_LEFTARROW:
		M_SoundAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_SoundAdjustSliders (1);
		break;
	}
}

/*
==========
Video Menu
==========
*/

int		video_cursor;
#define	VIDEO_ITEMS	18

void M_Menu_Video_f () 
{
	key_dest = key_menu;
	m_state = m_video;
	m_entersound = true;
}

void M_VideoAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (video_cursor)
	{
	case 0:	// screen size
		scr_viewsize.value += dir * 10;
		if (scr_viewsize.value < 30)
			scr_viewsize.value = 30;
		if (scr_viewsize.value > 120)
			scr_viewsize.value = 120;
		Cvar_SetValue ("viewsize", scr_viewsize.value);
		break;

	case 1:	// brightness
		brightness.value -= dir * 0.05;
		if (brightness.value < 0.5)
			brightness.value = 0.5;
		if (brightness.value > 1)
			brightness.value = 1;

		Cvar_SetValue ("brightness", brightness.value);
		break;

	case 2:		// shadows
		Cvar_SetValue ("r_shadows", !r_shadows.value);
		break;

	case 3:		// water alpha
		r_wateralpha.value += dir * 0.05; 
		if (r_wateralpha.value < 0)
			r_wateralpha.value = 0;
		if (r_wateralpha.value > 1)
			r_wateralpha.value = 1;
		Cvar_SetValue ("r_wateralpha", r_wateralpha.value);
		break;

	case 4:		// console alpha
		con_alpha.value += dir * 0.05; 
		if (con_alpha.value < 0)
			con_alpha.value = 0;
		if (con_alpha.value > 1)
			con_alpha.value = 1;
		Cvar_SetValue ("con_alpha", con_alpha.value);
		break;

	case 5:		// sbar alpha
		sbar_alpha.value += dir * 0.05; 
		if (sbar_alpha.value < 0)
			sbar_alpha.value = 0;
		if (sbar_alpha.value > 1)
			sbar_alpha.value = 1;
		Cvar_SetValue ("sbar_alpha", sbar_alpha.value);
		break;

	case 6:	// Glows
		Cvar_SetValue ("gl_glows", !gl_glows.value);
		break;

	case 7:	// Fullbrights
		Cvar_SetValue ("gl_fbr", !gl_fbr.value);
		break;

	case 8:	// Enviroment Mapping
		Cvar_SetValue ("gl_envmap", !gl_envmap.value);
		break;

	case 9:	// Underwater Caustics
		Cvar_SetValue ("gl_caustics", !gl_caustics.value);
		break;

	case 10:	// Fading CenterPrints
		Cvar_SetValue ("centerfade", !centerfade.value);
		break;

	case 11:		// fog
		Cvar_SetValue ("gl_fogenable", !gl_fogenable.value);
		break;

	case 12:	// fog start
		gl_fogstart.value += dir * 50.0f;
		if (gl_fogstart.value < 0.0f)
			gl_fogstart.value = 0.0f;
		if (gl_fogstart.value > 4096.0f)
			gl_fogstart.value = 4096.0f;
		if ((gl_fogend.value - gl_fogstart.value) < 500)
			 gl_fogstart.value = gl_fogend.value - 500;
		Cvar_SetValue ("gl_fogstart", gl_fogstart.value);
		break;

	case 13:	// fog end
		gl_fogend.value += dir * 50.0f;
		if (gl_fogend.value < 500.0f)
			gl_fogend.value = 500.0f;
		if (gl_fogend.value > 4096.0f)
			gl_fogend.value = 4096.0f;
		if ((gl_fogend.value - gl_fogstart.value) < 500)
			 gl_fogend.value = gl_fogstart.value + 500;
		Cvar_SetValue ("gl_fogend", gl_fogend.value);
		break;

	case 14:	// fog red
		gl_fogred.value += dir * 0.05f;
		if (gl_fogred.value < 0.0f)
			gl_fogred.value = 0.0f;
		if (gl_fogred.value > 1.0f)
			gl_fogred.value = 1.0f;
		Cvar_SetValue ("gl_fogred", gl_fogred.value);
		break;

	case 15:	// fog green 
		gl_foggreen.value += dir * 0.05f;
		if (gl_foggreen.value < 0.0f)
			gl_foggreen.value = 0.0f;
		if (gl_foggreen.value > 1.0f)
			gl_foggreen.value = 1.0f;
		Cvar_SetValue ("gl_foggreen", gl_foggreen.value);
		break;

	case 16:	// fog blue
		gl_fogblue.value += dir * 0.05f;
		if (gl_fogblue.value < 0.0f)
			gl_fogblue.value = 0.0f;
		if (gl_fogblue.value > 1.0f)
			gl_fogblue.value = 1.0f;
		Cvar_SetValue ("gl_fogblue", gl_fogblue.value);
		break;

	case 17:	// Crosshair graphic
		crosshair.value += dir * 1.0f;
		if (crosshair.value < 0.0f)
			crosshair.value = 0.0f;
		if (crosshair.value > 10.0f)
			crosshair.value = 10.0f;
		Cvar_SetValue ("crosshair", crosshair.value);
		break;
	}
}

void M_Video_Draw () 
{
	float	r;

	M_Print (16, 32, "           Screen size");
	r = (scr_viewsize.value - 30) * 0.0111111111;
	M_DrawSlider (220, 32, r);

	M_Print (16, 40, "            Brightness");
	r = (1.0 - brightness.value) * 2;
	M_DrawSlider (220, 40, r);

	M_Print (16, 48,"               Shadows");
	M_DrawCheckbox (220, 48, r_shadows.value);

	M_Print (16, 56, "           Water Alpha");
	r = r_wateralpha.value;
	M_DrawSlider (220, 56, r);

	M_Print (16, 64, "         Console Alpha");
	r = con_alpha.value;
	M_DrawSlider (220, 64, r);

	M_Print (16, 72, "      Status Bar Alpha");
	r = sbar_alpha.value;
	M_DrawSlider (220, 72, r);

	M_Print (16, 80, "                 Glows");
	M_DrawCheckbox (220, 80, gl_glows.value);

	M_Print (16, 88, "           Fullbrights");
	M_DrawCheckbox (220, 88, gl_fbr.value);

	M_Print (16, 96, "    Enviroment Mapping");
	M_DrawCheckbox (220, 96, gl_envmap.value);

	M_Print (16, 104,"   Underwater Caustics");
	M_DrawCheckbox (220, 104, gl_caustics.value);

	M_Print (16, 112,"   Fading CenterPrints");
	M_DrawCheckbox (220, 112, centerfade.value);

	M_Print (16, 120,"                   Fog");
	M_DrawCheckbox (220, 120, gl_fogenable.value);

	M_Print (16, 128,"             Fog Start");
	r = gl_fogstart.value * 0.000244140625;	// Tomaz - Speed
	M_DrawSlider (220, 128, r);	

	M_Print (16, 136,"               Fog End");
	r = gl_fogend.value * 0.000244140625;	// Tomaz - Speed
	M_DrawSlider (220, 136, r);

	M_Print (16, 144,"               Red Fog");

	M_Print (16, 152,"             Green Fog");

	M_Print (16, 160,"              Blue Fog");

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	

	r = gl_fogred.value;
	glColor3f(1,0,0);
	M_DrawSlider (220, 144, r);

	r = gl_foggreen.value;
	glColor3f(0,1,0);
	M_DrawSlider (220, 152, r);

	r = gl_fogblue.value;
	glColor3f(0,0,1);
	M_DrawSlider (220, 160, r);
	glColor3f(1,1,1);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	M_Print (16, 168,"             Crosshair");
	r = crosshair.value * 0.1;	// Tomaz - Speed
	M_DrawSlider (220, 168, r);

	M_DrawCharacter (200, 32 + video_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Video_Key (int key) 
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_VideoAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--video_cursor < 0)
			video_cursor = VIDEO_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++video_cursor >= VIDEO_ITEMS)
			video_cursor = 0;
		break;

	case K_LEFTARROW:
		M_VideoAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_VideoAdjustSliders (1);
		break;
	}
}

/*
=========
Misc Menu
=========
*/

int		misc_cursor;
#define	MISC_ITEMS	9

void M_Menu_Misc_f ()
{
	key_dest = key_menu;
	m_state = m_misc;
	m_entersound = true;
}

void M_MiscAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (misc_cursor)
	{
	case 0:	// improved aiming
		Cvar_SetValue ("impaim", !impaim.value);
		break;

	case 1:		// chase mode
		Cvar_SetValue ("chase_active", !chase_active.value);
		break;
	case 2:		// clear our of bounds
		Cvar_SetValue ("gl_clear", !gl_clear.value);
		break;
	case 3:		// show weapon
		Cvar_SetValue ("r_drawviewmodel", !r_drawviewmodel.value);
		break;
	
	case 4:	// Mouse Look
		Cvar_SetValue ("in_mlook", !in_mlook.value);
		break;

	case 5:	// Fps Counter
		Cvar_SetValue ("show_fps", !show_fps.value);
		break;

	case 6:	// Bobbing Items
		Cvar_SetValue ("r_bobbing", !r_bobbing.value);
		break;

	case 7:	// Wavesize
		r_wave.value += dir * 1.0f;
		if (r_wave.value < 0.0f)
			r_wave.value = 0.0f;
		if (r_wave.value > 20.0f)
			r_wave.value = 20.0f;
		Cvar_SetValue ("r_wave", r_wave.value);
		break;

	case 8:// MapShots
		Cvar_SetValue ("mapshots", !mapshots.value);
		break;
	}
}

void M_Misc_Draw () 
{
	float	r;

	M_Print (16, 32, "       Improved Aiming");
	M_DrawCheckbox (220, 32, impaim.value);

	M_Print (16, 40, "          Chase Camera");
	M_DrawCheckbox (220, 40, chase_active.value);

	M_Print (16, 48, "       Clear Clip Area");
	M_DrawCheckbox (220, 48, gl_clear.value);

	M_Print (16, 56, "           Show Weapon");
	M_DrawCheckbox (220, 56, r_drawviewmodel.value);

	M_Print (16, 64, "            Mouse Look");
	M_DrawCheckbox (220, 64, in_mlook.value);

	M_Print (16, 72, "           Fps Counter");
	M_DrawCheckbox (220, 72, show_fps.value);

	M_Print (16, 80, "         Bobbing Items");
	M_DrawCheckbox (220, 80, r_bobbing.value);

	M_Print (16, 88, "             Wave Size");
	r = r_wave.value * 0.05;	// Tomaz - Speed
	M_DrawSlider (220, 88, r);

	M_Print (16, 96, "              MapShots");
	M_DrawCheckbox (220, 96, mapshots.value);

	M_DrawCharacter (200, 32 + misc_cursor*8, 12+((int)(realtime*4)&1));
}
void M_Misc_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_MiscAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--misc_cursor < 0)
			misc_cursor = MISC_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++misc_cursor >= MISC_ITEMS)
			misc_cursor = 0;
		break;

	case K_LEFTARROW:
		M_MiscAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_MiscAdjustSliders (1);
		break;
	}
}
