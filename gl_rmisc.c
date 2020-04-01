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
// r_misc.c

#include "quakedef.h"
#include "gl_mirror.h"

void R_InitParticles (void);
void R_ClearParticles ();
void GL_BuildLightmaps (void);
void SHOWLMP_clear();

byte	dottexture[8][8] =
{
	{0,1,1,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,1,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};

/*
==================
R_InitTextures
==================
*/
void	R_InitTextures (void)
{
	int		x,y, m;
	byte	*dest;

// create a simple checkerboard texture for the default
	r_notexture_mip = Hunk_AllocName (sizeof(texture_t) + 16*16+8*8+4*4+2*2, "notexture");
	
	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;
	
	for (m=0 ; m<4 ; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];
		for (y=0 ; y< (16>>m) ; y++)
			for (x=0 ; x< (16>>m) ; x++)
			{
				if (  (y< (8>>m) ) ^ (x< (8>>m) ) )
					*dest++ = 0;
				else
					*dest++ = 0xff;
			}
	}	
}

/*
===============
R_Init
===============
*/
void R_Init (void)
{	
	extern byte *hunk_base;
	extern cvar_t gl_finish;

	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);	
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f);

	Cvar_RegisterVariable (&r_norefresh);
	Cvar_RegisterVariable (&r_drawviewmodel);
	Cvar_RegisterVariable (&r_shadows);
	Cvar_RegisterVariable (&r_wateralpha);
	Cvar_RegisterVariable (&r_dynamic);
	Cvar_RegisterVariable (&r_mirroralpha);
	Cvar_RegisterVariable (&r_novis);
	Cvar_RegisterVariable (&r_speeds);
	Cvar_RegisterVariable (&gl_finish);
	Cvar_RegisterVariable (&gl_clear);

	Cvar_RegisterVariable (&gl_nocolors);
	Cvar_RegisterVariable (&gl_keeptjunctions);
	Cvar_RegisterVariable (&gl_doubleeyes);
	Cvar_RegisterVariable (&slowmo);		// Tomaz - Slowmo
	Cvar_RegisterVariable (&gl_fogenable); 
	Cvar_RegisterVariable (&gl_fogstart); 
	Cvar_RegisterVariable (&gl_fogend); 
	Cvar_RegisterVariable (&gl_fogred); 
	Cvar_RegisterVariable (&gl_fogblue); 
	Cvar_RegisterVariable (&gl_foggreen);
	Cvar_RegisterVariable (&centerfade);	// Tomaz - Fading CenterPrints
	Cvar_RegisterVariable (&sbar_alpha);	// Tomaz - Sbar Alpha
	Cvar_RegisterVariable (&con_alpha);		// Tomaz - Console Alpha
	Cvar_RegisterVariable (&r_wave);		// Tomaz - Water Wave
	Cvar_RegisterVariable (&gl_glows);		// Tomaz - Glow
	Cvar_RegisterVariable (&r_bobbing);		// Tomaz - Bobbing Items
	Cvar_RegisterVariable (&gl_envmap);		// Tomaz - Enviroment Mapping
	Cvar_RegisterVariable (&gl_caustics);	// Tomaz - Underwater Caustics
	Cvar_RegisterVariable (&gl_fbr);		// Tomaz - Fullbrights
	Cvar_RegisterVariable (&impaim);		// Tomaz - Improved Aiming
	Cvar_RegisterVariable (&skybox_spin);	// Tomaz - Spinning Skyboxes
	Cvar_RegisterVariable (&mapshots);		// Tomaz - MapShots
	Cvar_RegisterVariable (&gl_showpolys);	// Tomaz - Show BSP Polygons
	Cvar_RegisterVariable (&gl_wireframe);	// Tomaz - Draw World as Wireframe and Textures
	Cvar_RegisterVariable (&gl_wireonly);	// Tomaz - Draw World as Wireframe Only
	Cvar_RegisterVariable (&gl_particles);	// Tomaz - Particles
	Cvar_RegisterVariable (&print_center_to_console);	// Tomaz - Prints CenterString's to the console
	Cvar_RegisterVariable (&gl_particle_fire);	// Tomaz - Fire Particles

	R_InitParticles (); // initiate particle engine

	playertextures = texture_extension_number;
	texture_extension_number += 16;
}

/*
===============
R_TranslatePlayerSkin

Translates a skin texture by the per-player color lookup
===============
*/
void R_TranslatePlayerSkin (int playernum)
{
	int				top, bottom;
	byte			translate[256];
	unsigned		translate32[256];
	unsigned int	i, j, s;
	model_t			*model;
	aliashdr_t		*paliashdr;
	byte			*original;
	unsigned		pixels[512*256], *out;
	unsigned		scaled_width, scaled_height;
	int				inwidth, inheight;
	byte			*inrow;
	unsigned		frac, fracstep;

	currententity = &cl_entities[1+playernum];
	model = currententity->model;
	if (!model)
		return;		// player doesn't have a model yet

	if (model->type != mod_alias)
		return; // only translate skins on alias models

	if (model->aliastype != ALIASTYPE_MDL) 
		return; // only translate skins on .mdl models 

	top = cl.scores[playernum].colors & 0xf0;
	bottom = (cl.scores[playernum].colors &15)<<4;

	for (i=0 ; i<256 ; i++)
		translate[i] = i;

	for (i=0 ; i<16 ; i++)
	{
		if (top < 128)	// the artists made some backwards ranges.  sigh.
			translate[TOP_RANGE+i] = top+i;
		else
			translate[TOP_RANGE+i] = top+15-i;
				
		if (bottom < 128)
			translate[BOTTOM_RANGE+i] = bottom+i;
		else
			translate[BOTTOM_RANGE+i] = bottom+15-i;
	}

	//
	// locate the original skin pixels
	//

	paliashdr = (aliashdr_t *)Mod_Extradata (model);
	s = paliashdr->skinwidth * paliashdr->skinheight;
	if (currententity->skinnum < 0 || currententity->skinnum >= paliashdr->numskins) {
		Con_Printf("(%d): Invalid player skin #%d\n", playernum, currententity->skinnum);
		original = (byte *)paliashdr + paliashdr->texels[0];
	} else
		original = (byte *)paliashdr + paliashdr->texels[currententity->skinnum];
	if (s & 3)
		Sys_Error ("R_TranslateSkin: s&3");

	inwidth = paliashdr->skinwidth;
	inheight = paliashdr->skinheight;

	// because this happens during gameplay, do it fast
	// instead of sending it through gl_upload 8
	glBindTexture (GL_TEXTURE_2D, playertextures + playernum);

	scaled_width = gl_max_size.value < 512 ? gl_max_size.value : 512;
	scaled_height = gl_max_size.value < 256 ? gl_max_size.value : 256;

	for (i=0 ; i<256 ; i++)
		translate32[i] = d_8to24table[translate[i]];

	out = pixels;
	fracstep = inwidth*0x10000/scaled_width;
	for (i=0 ; i<scaled_height ; i++, out += scaled_width)
	{
		inrow = original + inwidth*(i*inheight/scaled_height);
		frac = fracstep >> 1;
		for (j=0 ; j<scaled_width ; j+=4)
		{
			out[j] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+1] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+2] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+3] = translate32[inrow[frac>>16]];
			frac += fracstep;
		}
	}
	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*
===============
R_NewMap
===============
*/
void R_NewMap (void)
{
	int		i;
	extern int r_dlightframecount;

	for (i=0 ; i<256 ; i++)
		d_lightstylevalue[i] = 264;		// normal light value

	memset (&r_worldentity, 0, sizeof(r_worldentity));
	r_worldentity.model = cl.worldmodel;

// clear out efrags in case the level hasn't been reloaded
// FIXME: is this one short?
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
		cl.worldmodel->leafs[i].efrags = NULL;
		 	
	r_viewleaf = NULL;
	R_ClearParticles ();

	r_dlightframecount = 0;

	GL_BuildLightmaps ();

	SHOWLMP_clear();	// Tomaz - Show Hide LMP
}

/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
void R_TimeRefresh_f (void)
{
	int			i;
	float		start, stop, time;

	glDrawBuffer  (GL_FRONT);
	glFinish ();

	start = Sys_FloatTime ();
	for (i=0 ; i<128 ; i++)
	{
		r_refdef.viewangles[1] = i/128.0*360.0;
		R_RenderView ();
	}

	glFinish ();
	stop = Sys_FloatTime ();
	time = stop-start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128/time);

	glDrawBuffer  (GL_BACK);
	GL_EndRendering ();
}