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

//
// all MDL code goes into this file
//

#include "quakedef.h"

stvert_t	stverts[MAXALIASVERTS];
trivertx_t	*poseverts[MAXALIASFRAMES];
aliashdr_t	*pheader;
aliashdr_t	*paliashdr;
mtriangle_t	triangles[MAXALIASTRIS];
model_t		*loadmodel;
model_t		*aliasmodel;


// precalculated dot products for quantized angles
float	r_avertexnormal_dots_mdl[16][256] =
#include "anorm_dots.h"
;
float			*shadedots_mdl = r_avertexnormal_dots_mdl[0];
float			*shadedots2_mdl = r_avertexnormal_dots_mdl[0];
extern float	lightlerpoffset;

int		posenum;
int		lastposenum;
int		lastposenum0;
int		commands[65536];
int		numcommands;
int		vertexorder[8192];
int		numorder;
int		allverts, alltris;
int		stripverts[8192];	
int		striptris[8192];
int		stripcount;

extern	vec3_t  lightcolor;
extern	vec3_t	lightspot;
extern unsigned d_8to24table[];

char	loadname[32];	// for hunk tags

qboolean	used[8192];

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)
#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}
#define BOUNDI(VALUE,MIN,MAX) if (VALUE < MIN || VALUE >= MAX) Host_Error("model %s has an invalid VALUE (%d exceeds %d - %d)\n", mod->name, VALUE, MIN, MAX);
#define BOUNDF(VALUE,MIN,MAX) if (VALUE < MIN || VALUE >= MAX) Host_Error("model %s has an invalid VALUE (%f exceeds %f - %f)\n", mod->name, VALUE, MIN, MAX);

typedef struct
{
	short		x, y;
} floodfill_t;


/*
=================================================================

ALIAS MODEL DISPLAY LIST GENERATION

=================================================================
*/

/*
================
StripLength
================
*/
int	StripLength (int starttri, int startv)
{
	int			m1, m2;
	int			j;
	mtriangle_t	*last, *check;
	int			k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripverts[1] = last->vertindex[(startv+1)%3];
	stripverts[2] = last->vertindex[(startv+2)%3];

	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+2)%3];
	m2 = last->vertindex[(startv+1)%3];

	// look for a matching triangle
nexttri:
	for (j=starttri+1, check=&triangles[starttri+1] ; j<pheader->numtris ; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k=0 ; k<3 ; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			if (stripcount & 1)
				m2 = check->vertindex[ (k+2)%3 ];
			else
				m1 = check->vertindex[ (k+2)%3 ];

			stripverts[stripcount+2] = check->vertindex[ (k+2)%3 ];
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for (j=starttri+1 ; j<pheader->numtris ; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}

/*
===========
FanLength
===========
*/
int	FanLength (int starttri, int startv)
{
	int		m1, m2;
	int		j;
	mtriangle_t	*last, *check;
	int		k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripverts[1] = last->vertindex[(startv+1)%3];
	stripverts[2] = last->vertindex[(startv+2)%3];

	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+0)%3];
	m2 = last->vertindex[(startv+2)%3];


	// look for a matching triangle
nexttri:
	for (j=starttri+1, check=&triangles[starttri+1] ; j<pheader->numtris ; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k=0 ; k<3 ; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			m2 = check->vertindex[ (k+2)%3 ];

			stripverts[stripcount+2] = m2;
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for (j=starttri+1 ; j<pheader->numtris ; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}

/*
================
BuildTris

Generate a list of trifans or strips
for the model, which holds for all frames
================
*/
void BuildTris (void)
{
	int		i, j, k;
	int		startv;
	float	s, t;
	int		len, bestlen, besttype;
	int		bestverts[MAXALIASVERTS];	// Test
	int		besttris[MAXALIASVERTS];		// Test
	int		type;

	//
	// build tristrips
	//
	numorder = 0;
	numcommands = 0;
	memset (used, 0, sizeof(used));
	for (i=0 ; i<pheader->numtris ; i++)
	{
		// pick an unused triangle and start the trifan
		if (used[i])
			continue;

		bestlen = 0;
		for (type = 0 ; type < 2 ; type++)
//	type = 1;
		{
			for (startv =0 ; startv < 3 ; startv++)
			{
				if (type == 1)
					len = StripLength (i, startv);
				else
					len = FanLength (i, startv);
				if (len > bestlen)
				{
					besttype = type;
					bestlen = len;
					for (j=0 ; j<bestlen+2 ; j++)
						bestverts[j] = stripverts[j];
					for (j=0 ; j<bestlen ; j++)
						besttris[j] = striptris[j];
				}
			}
		}

		// mark the tris on the best strip as used
		for (j=0 ; j<bestlen ; j++)
			used[besttris[j]] = 1;

		if (besttype == 1)
			commands[numcommands++] = (bestlen+2);
		else
			commands[numcommands++] = -(bestlen+2);

		for (j=0 ; j<bestlen+2 ; j++)
		{
			// emit a vertex into the reorder buffer
			k = bestverts[j];
			vertexorder[numorder++] = k;

			// emit s/t coords into the commands stream
			s = stverts[k].s;
			t = stverts[k].t;
			if (!triangles[besttris[0]].facesfront && stverts[k].onseam)
				s += pheader->skinwidth * 0.5;	// Tomaz - Speed
			s = (s + 0.5) / pheader->skinwidth;
			t = (t + 0.5) / pheader->skinheight;

			*(float *)&commands[numcommands++] = s;
			*(float *)&commands[numcommands++] = t;
		}
	}

	commands[numcommands++] = 0;		// end of list marker

	Con_DPrintf ("%3i tri %3i vert %3i cmd\n", pheader->numtris, numorder, numcommands);

	allverts += numorder;
	alltris += pheader->numtris;
}

qboolean started_loading;

/*
================
GL_MakeAliasModelDisplayLists
================
*/
void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr)
{
	// Tomaz - Removed ms2 Begin
	int			i, j;
	int			*cmds;
	trivertx_t	*verts;

	aliasmodel	=	m;
	paliashdr	=	hdr;

	if (!started_loading)
	{
		started_loading = true;
//		Con_Printf ("Meshing models");
	}

//	Con_Printf (".");

	BuildTris ();		// trifans or lists

	// save the data out

	paliashdr->poseverts = numorder;

	cmds = Hunk_Alloc (numcommands * 4);
	paliashdr->commands = (byte *)cmds - (byte *)paliashdr;
	memcpy (cmds, commands, (numcommands * 4));

	verts = Hunk_Alloc (paliashdr->numposes * paliashdr->poseverts * sizeof(trivertx_t));
	paliashdr->posedata = (byte *)verts - (byte *)paliashdr;
	for (i=0 ; i<paliashdr->numposes ; i++)
		for (j=0 ; j<numorder ; j++)
			*verts++ = poseverts[i][vertexorder[j]];
	// Tomaz - Removed ms2 End
}

/*
=================================================================

ALIAS MODEL SKIN

=================================================================
*/

/*
=================
Mod_FloodFillSkin
=================
*/
void Mod_FloodFillSkin( byte *skin, int skinwidth, int skinheight )
{
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t			fifo[FLOODFILL_FIFO_SIZE];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP( -1, -1, 0 );
		if (x < skinwidth - 1)	FLOODFILL_STEP( 1, 1, 0 );
		if (y > 0)				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if (y < skinheight - 1)	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	}
}
/*
===============
Mod_LoadAllSkins
===============
*/
void *Mod_LoadAllSkins (int numskins, daliasskintype_t *pskintype)
{
	int		i, j, k, size;
	char	name[64], model[64];
	int		s;
	byte	*skin, *data = NULL, *data2;
	byte	*texels;
	daliasskingroup_t		*pinskingroup;
	int		groupskins;
	daliasskininterval_t	*pinskinintervals;
//	int		modelname;
	
	skin = (byte *)(pskintype + 1);

	if (numskins < 1 || numskins > MAX_SKINS)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of skins: %d\n", numskins);

	s = pheader->skinwidth * pheader->skinheight;

	for (i=0 ; i<numskins ; i++)
	{
		if (pskintype->type == ALIAS_SKIN_SINGLE)
		{
			Mod_FloodFillSkin( skin, pheader->skinwidth, pheader->skinheight );
			
			texels = Hunk_AllocName(s, loadname);
			pheader->texels[i] = texels - (byte *)pheader;
			memcpy (texels, (byte *)(pskintype + 1), s);

			// TGA Begin
			// we check to see if a tga version of the skin exists, drawing happens elsewhere
			COM_StripExtension(loadmodel->name, model);
			_snprintf (name, sizeof(name),"%s_%i", model, i);

			pheader->transparent = true;

			pheader->gl_texturenum[i][0] =
			pheader->gl_texturenum[i][1] =
			pheader->gl_texturenum[i][2] =
			pheader->gl_texturenum[i][3] =

			loadtextureimage3 (name, false, true, data); // load texture "name"

			if (pheader->gl_texturenum[i][0] == 0)// did not find a matching TGA...		
			{
				data = (byte *)(pskintype + 1);
				pheader->transparent = false;
				pheader->gl_texturenum[i][0] =
				pheader->gl_texturenum[i][1] =
				pheader->gl_texturenum[i][2] =
				pheader->gl_texturenum[i][3] =

				GL_LoadTexture (name, pheader->skinwidth, pheader->skinheight, data, true, false, 1);

				size = pheader->skinwidth*pheader->skinheight;

				if (Has_Fullbrights (data, size))
				{
					data2 = malloc (size);
					
					for (j = 0;j < size;j++)
					{
						if (data[j] > 223)
							data2[j] = data[j];
						else
							data2[j] = 255;
					}
					
					pheader->fb_texturenum[i][0] =
					pheader->fb_texturenum[i][1] =
					pheader->fb_texturenum[i][2] =
					pheader->fb_texturenum[i][3] =
						GL_LoadTexture (va("fbrm_%s",name), pheader->skinwidth, pheader->skinheight, data2, true, true, 1);

					free(data2);
				}
			}
			// TGA End

			pskintype = (daliasskintype_t *)((byte *)(pskintype+1) + s);
		}
		else
		{
			// animating skin group.  yuck.
			pskintype++;
			pinskingroup = (daliasskingroup_t *)pskintype;
			groupskins = LittleLong (pinskingroup->numskins);
			pinskinintervals = (daliasskininterval_t *)(pinskingroup + 1);

			pskintype = (void *)(pinskinintervals + groupskins);

			for (j=0 ; j<groupskins ; j++)
			{
				Mod_FloodFillSkin( skin, pheader->skinwidth, pheader->skinheight );
				if (j == 0) {
					texels = Hunk_AllocName(s, loadname);
					pheader->texels[i] = texels - (byte *)pheader;
					data = (byte *)(pskintype);
					memcpy (texels, data, s);
				}
				_snprintf (name, sizeof(name),"%s_%i_%i", loadmodel->name, i,j);
				pheader->gl_texturenum[i][j&3] = 
					GL_LoadTexture (name, pheader->skinwidth, pheader->skinheight, data, true, false, 1);
				
				size = pheader->skinwidth*pheader->skinheight;
				
				if (Has_Fullbrights (data, size))
				{
					data2 = malloc (size);
					
					for (j = 0;j < size;j++)
					{
						if (data[j] > 223)
							data2[j] = data[j];
						else
							data2[j] = 255;
					}
					
					pheader->fb_texturenum[i][j&3] =
						GL_LoadTexture (va("fbrm_%s",name), pheader->skinwidth, pheader->skinheight, data2, true, true, 1);
					
					free(data2);
				}
				
				pskintype = (daliasskintype_t *)((byte *)(pskintype) + s);
			}
			k = j;
			for (/* */; j < 4; j++)
				pheader->gl_texturenum[i][j&3] = 
				pheader->gl_texturenum[i][j - k]; 
		}
	}


	return (void *)pskintype;
}

/*
=================================================================

ALIAS MODEL FRAMES

=================================================================
*/

int		aliasbboxmins[3], aliasbboxmaxs[3];

/*
=================
Mod_LoadAliasFrame
=================
*/
void * Mod_LoadAliasFrame (void * pin, maliasframedesc_t *frame)
{
	trivertx_t		*pinframe;
	int				i;
	daliasframe_t	*pdaliasframe;
	
	pdaliasframe = (daliasframe_t *)pin;

	strcpy (frame->name, pdaliasframe->name);
	frame->firstpose = posenum;
	frame->numposes = 1;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about
	// endianness
		frame->bboxmin.v[i] = pdaliasframe->bboxmin.v[i];
		frame->bboxmax.v[i] = pdaliasframe->bboxmax.v[i];

		aliasbboxmins[i] = min (frame->bboxmin.v[i], aliasbboxmins[i]);
		aliasbboxmaxs[i] = max (frame->bboxmax.v[i], aliasbboxmaxs[i]);
	}

	pinframe = (trivertx_t *)(pdaliasframe + 1);

	poseverts[posenum] = pinframe;
	posenum++;

	pinframe += pheader->numverts;

	return (void *)pinframe;
}

/*
=================
Mod_LoadAliasGroup
=================
*/
void *Mod_LoadAliasGroup (void * pin,  maliasframedesc_t *frame)
{
	daliasgroup_t		*pingroup;
	int					i, numframes;
	daliasinterval_t	*pin_intervals;
	void				*ptemp;
	
	pingroup = (daliasgroup_t *)pin;

	numframes = LittleLong (pingroup->numframes);

	frame->firstpose = posenum;
	frame->numposes = numframes;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about endianness
		frame->bboxmin.v[i] = pingroup->bboxmin.v[i];
		frame->bboxmax.v[i] = pingroup->bboxmax.v[i];

		aliasbboxmins[i] = min (frame->bboxmin.v[i], aliasbboxmins[i]);
		aliasbboxmaxs[i] = max (frame->bboxmax.v[i], aliasbboxmaxs[i]);
	}

	pin_intervals = (daliasinterval_t *)(pingroup + 1);

	frame->interval = LittleFloat (pin_intervals->interval);

	pin_intervals += numframes;

	ptemp = (void *)pin_intervals;

	for (i=0 ; i<numframes ; i++)
	{
		poseverts[posenum] = (trivertx_t *)((daliasframe_t *)ptemp + 1);
		posenum++;

		ptemp = (trivertx_t *)((daliasframe_t *)ptemp + 1) + pheader->numverts;
	}

	return ptemp;
}
/*
=============================================================

  ALIAS MODELS

=============================================================
*/
/*
=================
Mod_LoadAliasModel
=================
*/
void Mod_LoadAliasModel (model_t *mod, void *buffer)
{
	int					i, j;
	mdl_t				*pinmodel;
	stvert_t			*pinstverts;
	dtriangle_t			*pintriangles;
	int					version, numframes;
	int					size;
	daliasframetype_t	*pframetype;
	daliasskintype_t	*pskintype;
	int					start, end, total;
	
	start = Hunk_LowMark ();

	pinmodel = (mdl_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != ALIAS_VERSION)
		Host_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);

//
// allocate space for a working header, plus all the data except the frames,
// skin and group info
//
	size = 	sizeof (aliashdr_t) + (LittleLong (pinmodel->numframes) - 1) * sizeof (pheader->frames[0]);
	pheader = Hunk_AllocName (size, loadname);
	
	mod->flags = LittleLong (pinmodel->flags);

//
// endian-adjust and copy the data, starting with the alias model header
//
	pheader->boundingradius = LittleFloat (pinmodel->boundingradius);
	pheader->numskins		= LittleLong  (pinmodel->numskins);
	pheader->skinwidth		= LittleLong  (pinmodel->skinwidth);
	pheader->skinheight		= LittleLong  (pinmodel->skinheight);
	pheader->numverts		= LittleLong  (pinmodel->numverts);
	pheader->numtris		= LittleLong  (pinmodel->numtris);
	pheader->numframes		= LittleLong  (pinmodel->numframes);

	BOUNDI(pheader->skinheight, 0, MAX_LBM_HEIGHT);
	BOUNDI(pheader->numverts  , 0, MAXALIASVERTS);
	BOUNDI(pheader->numtris   , 0, MAXALIASTRIS);
	BOUNDI(pheader->numframes , 1, MAXALIASFRAMES);

	numframes = pheader->numframes;

	pheader->size = LittleFloat (pinmodel->size) * ALIAS_BASE_SIZE_RATIO;
	mod->synctype = LittleLong (pinmodel->synctype);
	mod->numframes = pheader->numframes;

	for (i=0 ; i<3 ; i++)
	{
		pheader->scale[i] = LittleFloat (pinmodel->scale[i]);
		pheader->scale_origin[i] = LittleFloat (pinmodel->scale_origin[i]);
		pheader->eyeposition[i] = LittleFloat (pinmodel->eyeposition[i]);
	}


//
// load the skins
//

	pskintype = (daliasskintype_t *)&pinmodel[1];
	pskintype = Mod_LoadAllSkins (pheader->numskins, pskintype);		


//
// load base s and t vertices
//
	pinstverts = (stvert_t *)pskintype;

	for (i=0 ; i<pheader->numverts ; i++)
	{
		stverts[i].onseam = LittleLong (pinstverts[i].onseam);
		stverts[i].s = LittleLong (pinstverts[i].s);
		stverts[i].t = LittleLong (pinstverts[i].t);
	}

//
// load triangle lists
//
	pintriangles = (dtriangle_t *)&pinstverts[pheader->numverts];

	for (i=0 ; i<pheader->numtris ; i++)
	{
		triangles[i].facesfront = LittleLong (pintriangles[i].facesfront);

		for (j=0 ; j<3 ; j++)
			triangles[i].vertindex[j] =	LittleLong (pintriangles[i].vertindex[j]);
	}

//
// load the frames
//
	posenum = 0;
	pframetype = (daliasframetype_t *)&pintriangles[pheader->numtris];

	aliasbboxmins[0] = aliasbboxmins[1] = aliasbboxmins[2] = 255;
	aliasbboxmaxs[0] = aliasbboxmaxs[1] = aliasbboxmaxs[2] = -255;

	for (i=0 ; i<numframes ; i++)
	{
		if ((aliasframetype_t) LittleLong (pframetype->type) == ALIAS_SINGLE)
			pframetype = (daliasframetype_t *) Mod_LoadAliasFrame (pframetype + 1, &pheader->frames[i]);
		else
			pframetype = (daliasframetype_t *) Mod_LoadAliasGroup (pframetype + 1, &pheader->frames[i]);
	}

	pheader->numposes = posenum;

	mod->type = mod_alias;
	mod->aliastype = ALIASTYPE_MDL;

	for (i = 0; i < 3; i++)
	{
		mod->mins[i] = aliasbboxmins[i] * pheader->scale[i] + pheader->scale_origin[i];
		mod->maxs[i] = aliasbboxmaxs[i] * pheader->scale[i] + pheader->scale_origin[i];
	}

	mod->glow_radius = 0.0f;
	VectorClear (mod->glow_color);

	if ((!strcmp (mod->name, "progs/missile.mdl")) ||
		(!strcmp (mod->name, "progs/plasma.mdl")))
		mod->glow_radius = 6.0f;
	else if ((!strncmp (mod->name, "progs/glow_", 11)) ||
			(!strncmp (mod->name, "progs/bolt", 10))  ||
			(!strcmp (mod->name, "progs/laser.mdl")))
		mod->glow_radius = 24.0f;

	if (!strcmp (mod->name, "progs/missile.mdl"))
		VectorSet(mod->glow_color, 0.7f, 0.49f, 0.28f);
	else if (!strcmp (mod->name, "progs/plasma.mdl"))
		VectorSet(mod->glow_color, 0.0f, 0.7f, 0.0f);
	else if ((!strcmp (mod->name, "progs/bolt.mdl"))	||
			 (!strcmp (mod->name, "progs/laser.mdl")))
		VectorSet(mod->glow_color, 0.2f, 0.06f, 0.06f);
	else if ((!strcmp (mod->name, "progs/bolt2.mdl"))	||
			 (!strcmp (mod->name, "progs/bolt3.mdl")))
		VectorSet(mod->glow_color, 0.06f, 0.06f, 0.2f);

	mod->noshadow = false;

	if ((!strcmp (mod->name, "progs/lavaball.mdl"))	||
		(!strcmp (mod->name, "progs/laser.mdl"))	||
		(!strcmp (mod->name, "progs/boss.mdl"))		|| 
		(!strcmp (mod->name, "progs/oldone.mdl"))	|| 
		(!strcmp (mod->name, "progs/missile.mdl"))	||
		(!strcmp (mod->name, "progs/grenade.mdl"))	||
		(!strcmp (mod->name, "progs/spike.mdl"))	|| 
		(!strcmp (mod->name, "progs/s_spike.mdl"))	||
		(!strcmp (mod->name, "progs/zom_gib.mdl"))	||
		(!strncmp (mod->name, "progs/v_", 8))		||
		(!strncmp (mod->name, "progs/bolt", 10))	||
		(!strncmp (mod->name, "progs/gib", 9))		||
		(!strncmp (mod->name, "progs/h_", 8))		||
		(!strncmp (mod->name, "progs/flame", 11)))
		mod->noshadow = true;

	mod->fullbright = false;

	if (
		(!strcmp (mod->name, "progs/laser")) ||
		(!strcmp (mod->name, "progs/lavaball.mdl")) ||
		(!strncmp (mod->name, "progs/bolt", 10)) ||
		(!strncmp (mod->name, "progs/flame", 11)) 
		)
		mod->fullbright = true;

	//
	// build the draw lists
	//
	GL_MakeAliasModelDisplayLists (mod, pheader);

//
// move the complete, relocatable alias model to the cache
//	
	end = Hunk_LowMark ();
	total = end - start;
	
	Cache_Alloc (&mod->cache, total, loadname);
	if (!mod->cache.data)
		return;
	memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);
}


/*
=============
GL_DrawAliasBlendedFrame
=============
*/
void GL_DrawAliasBlendedFrame (int frame, aliashdr_t *paliashdr, entity_t* e)
{
	float		l, blend;
	trivertx_t	*verts1, *verts2;
	int			count, pose, numposes, *order;
	vec3_t		d;

	if ((frame >= paliashdr->numframes) || (frame < 0))
	{
		Con_DPrintf ("GL_DrawAliasBlendedFrame: no such frame %d\n", frame);
		frame = 0;
	}

	pose		= paliashdr->frames[frame].firstpose;
	numposes	= paliashdr->frames[frame].numposes;

	if (numposes > 1)
	{
		e->frame_interval = paliashdr->frames[frame].interval;
		pose += (int)(cl.time / e->frame_interval) % numposes;
	}
	else 
	{
		e->frame_interval = 0.1f;
	}

	if (e->pose2 != pose)
	{
		e->frame_start_time = realtime;
		e->pose1 = e->pose2;
		e->pose2 = pose;
		blend = 0;
	}
	else
	{
		blend = ((realtime - e->frame_start_time)*slowmo.value) / e->frame_interval;

		if (cl.paused || blend > 1) 
			blend = 1;
	}

	lastposenum0 = e->pose1;
	lastposenum  = e->pose2;

	verts1  = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts2  = verts1;

	verts1 += e->pose1 * paliashdr->poseverts;
	verts2 += e->pose2 * paliashdr->poseverts;

	order = (int *)((byte *)paliashdr + paliashdr->commands);


	for (;;)
	{
		// get the vertex count and primitive type
		count = *order++;
        
		if (!count) 
			break;
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
		{
			glBegin (GL_TRIANGLE_STRIP);
		}
		do
		{
			if (!e->model->fullbright)
			{
				float d2, l1, l2, diff;
				d[0] = shadedots_mdl[verts2->lightnormalindex] - shadedots_mdl[verts1->lightnormalindex];
				d2 = shadedots2_mdl[verts2->lightnormalindex] - shadedots2_mdl[verts1->lightnormalindex];
				l1 = shadedots_mdl[verts1->lightnormalindex] + (blend * d[0]);
				l2 = shadedots2_mdl[verts1->lightnormalindex] + (blend * d2);
				if (l1 != l2)
				{
					if (l1 > l2) {
						diff = l1 - l2;
						diff *= lightlerpoffset;
						l = l1 - diff;
					} else {
						diff = l2 - l1;
						diff *= lightlerpoffset;
						l = l1 + diff;
					}
				}
				else
				{
					l = l1;
				}

				glColor4f (l * lightcolor[0], l * lightcolor[1], l * lightcolor[2], e->alpha);	// Tomaz - QC Alpha
			}
			else {
				glColor4f (1, 1, 1, e->alpha);	// Tomaz - QC Alpha
			}

			// texture coordinates come from the draw list
			glTexCoord2f (((float *)order)[0], ((float *)order)[1]);
			order += 2;

			// normals and vertexes come from the frame list
			VectorSubtract(verts2->v, verts1->v, d);

			// blend the vertex positions from each frame together
			glVertex3f (
						verts1->v[0] + (blend * d[0]),
						verts1->v[1] + (blend * d[1]),
						verts1->v[2] + (blend * d[2]));

			verts1++;
			verts2++;
		} 
		while 
			(--count);
		glEnd ();

	}
	glColor4f (1,1,1,1);
}

/*
=============
GL_DrawAliasBlendedShadow
=============
*/
void GL_DrawAliasBlendedShadow (aliashdr_t *paliashdr, int pose1, int pose2, entity_t* e)
{
	trivertx_t* verts1;
	trivertx_t* verts2;
	int*        order;
	vec3_t      point1;
	vec3_t      point2;
	vec3_t      d;
	float       height;
	float       lheight;
	int         count;
	float       blend;

	// Tomaz - New Shadow Begin
	trace_t		downtrace;
	vec3_t		downmove;
	float		s1,c1;
	// Tomaz - New Shadow End

	blend = (realtime - e->frame_start_time) / e->frame_interval;

	if (blend > 1) blend = 1;

	lheight = e->origin[2] - lightspot[2];
	height  = -lheight; // Tomaz - New Shadow

	// Tomaz - New Shadow Begin
	VectorCopy (e->origin, downmove);
	downmove[2] = downmove[2] - 4096;
	memset (&downtrace, 0, sizeof(downtrace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, e->origin, downmove, &downtrace);

	s1 = sin( e->angles[1]/180*M_PI);
	c1 = cos( e->angles[1]/180*M_PI);
	// Tomaz - New Shadow End

	verts1 = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts2 = verts1;

	verts1 += pose1 * paliashdr->poseverts;
	verts2 += pose2 * paliashdr->poseverts;

	order = (int *)((byte *)paliashdr + paliashdr->commands);

	for (;;)
	{
		// get the vertex count and primitive type
		count = *order++;

		if (!count) break;

		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
		{
			glBegin (GL_TRIANGLE_STRIP);
		}

		do
		{
			order += 2;

			point1[0] = verts1->v[0] * paliashdr->scale[0] + paliashdr->scale_origin[0];
			point1[1] = verts1->v[1] * paliashdr->scale[1] + paliashdr->scale_origin[1];
			point1[2] = verts1->v[2] * paliashdr->scale[2] + paliashdr->scale_origin[2];

			point2[0] = verts2->v[0] * paliashdr->scale[0] + paliashdr->scale_origin[0];
			point2[1] = verts2->v[1] * paliashdr->scale[1] + paliashdr->scale_origin[1];
			point2[2] = verts2->v[2] * paliashdr->scale[2] + paliashdr->scale_origin[2];

			VectorSubtract(point2, point1, d);

			// Tomaz - New shadow Begin
			point1[0] = point1[0] + (blend * d[0]);
			point1[1] = point1[1] + (blend * d[1]);
			point1[2] = point1[2] + (blend * d[2]);

			point1[2] =  - (e->origin[2] - downtrace.endpos[2]);

			point1[2] += ((point1[1] * (s1 * downtrace.plane.normal[0])) -
						  (point1[0] * (c1 * downtrace.plane.normal[0])) -
						  (point1[0] * (s1 * downtrace.plane.normal[1])) - 
						  (point1[1] * (c1 * downtrace.plane.normal[1]))) +  
						  ((1.0 - downtrace.plane.normal[2])*20) + 0.2 ;

			glVertex3fv (point1);
			// Tomaz - New shadow Begin

			verts1++;
			verts2++;
		} 
		while 
			(--count);
		glEnd ();
	}       
}
