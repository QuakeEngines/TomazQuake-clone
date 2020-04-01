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


//
// gl_warp.c -- sky and water polygons
//

//
// External
//
extern	model_t	*loadmodel;
extern	cvar_t	gl_subdivide_size;
extern int causticstexture[31];

//
// Integer
//
int		shinytexture;

//
// msurface_t
//
msurface_t	*warpface;

//
// Float
//
float	turbsin[] =
{
	#include "gl_warp_sin.h"
};

//
// Define
// 
#define TURBSCALE (40.7436589469704879)


/*
================
BoundPoly
================
*/
void BoundPoly (int numverts, float *verts, vec3_t mins, vec3_t maxs)
{
	int		i, j;
	float	*v;

	mins[0] = mins[1] = mins[2] = 9999;
	maxs[0] = maxs[1] = maxs[2] = -9999;
	v = verts;
	for (i=0 ; i<numverts ; i++)
		for (j=0 ; j<3 ; j++, v++)
		{
			if (*v < mins[j])
				mins[j] = *v;
			if (*v > maxs[j])
				maxs[j] = *v;
		}
}

/*
================
SubdividePolygon
================
*/
void SubdividePolygon (int numverts, float *verts)
{
	int		i, j, k;
	vec3_t	mins, maxs;
	float	m;
	float	*v;
	vec3_t	front[64], back[64];
	int		f, b;
	float	dist[64];
	float	frac;
	glpoly_t	*poly;
	float	s, t;

	if (numverts > 60)
		Sys_Error ("numverts = %i", numverts);

	BoundPoly (numverts, verts, mins, maxs);

	for (i=0 ; i<3 ; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5;
		m = gl_subdivide_size.value * floor (m/gl_subdivide_size.value + 0.5);
		if (maxs[i] - m < 8)
			continue;
		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;
		for (j=0 ; j<numverts ; j++, v+= 3)
			dist[j] = *v - m;

		// wrap cases
		dist[j] = dist[0];
		v-=i;
		VectorCopy (verts, v);

		f = b = 0;
		v = verts;
		for (j=0 ; j<numverts ; j++, v+= 3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy (v, front[f]);
				f++;
			}
			if (dist[j] <= 0)
			{
				VectorCopy (v, back[b]);
				b++;
			}
			if (dist[j] == 0 || dist[j+1] == 0)
				continue;
			if ( (dist[j] > 0) != (dist[j+1] > 0) )
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);
				for (k=0 ; k<3 ; k++)
					front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);
				f++;
				b++;
			}
		}

		SubdividePolygon (f, front[0]);
		SubdividePolygon (b, back[0]);
		return;
	}

	poly = Hunk_Alloc (sizeof(glpoly_t) + (numverts-4) * VERTEXSIZE*sizeof(float));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts;
	for (i=0 ; i<numverts ; i++, verts+= 3)
	{
		VectorCopy (verts, poly->verts[i]);
		s = DotProduct (verts, warpface->texinfo->vecs[0]);
		t = DotProduct (verts, warpface->texinfo->vecs[1]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;
	}
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface (msurface_t *s)
{
	vec3_t		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	float		*vec;

	warpface = s;

	//
	// convert edges back to a normal polygon
	//
	numverts = 0;
	for (i=0 ; i<s->numedges ; i++)
	{
		lindex = loadmodel->surfedges[s->firstedge + i];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
		VectorCopy (vec, verts[numverts]);
		numverts++;
	}

	SubdividePolygon (numverts, verts[0]);
}

//=========================================================

/*			
Emit*****Polys - emits polygons with special effects
*/

/*
=============
EmitWaterPolys

Does a water warp on the pre-fragmented glpoly_t chain
=============
*/
void EmitWaterPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	float		r, t, os, ot;
	vec3_t		nv;

	for (p=s->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			r = os + turbsin[(int)((ot*0.125+realtime) * TURBSCALE) & 255];
			r *= (0.015f);

			t = ot + turbsin[(int)((os*0.125+realtime) * TURBSCALE) & 255];
			t *= (0.015f);
			
			nv[0] = v[0];
			nv[1] = v[1];
			nv[2] = v[2];

			if (r_wave.value)
				nv[2] = v[2] + r_wave.value *sin(v[0]*0.02+realtime)*sin(v[1]*0.02+realtime)*sin(v[2]*0.02+realtime);

			glTexCoord2f (r, t);
			glVertex3fv (nv);
		}
		glEnd ();
	}
}

/*
=============
EmitEnvMapPolys

Enviroment Mapping
=============
*/

void EmitEnvMapPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int		i;
	vec3_t		vert;

	if (!gl_envmap.value)
		return;

	p = s->polys;


	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture (GL_TEXTURE_2D, shinytexture);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	for (p=s->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			vert[0] = (v[0] * r_world_matrix[0]) + (v[1] * r_world_matrix[4]) + (v[2] * r_world_matrix[8] ) + r_world_matrix[12];
			vert[1] = (v[0] * r_world_matrix[1]) + (v[1] * r_world_matrix[5]) + (v[2] * r_world_matrix[9] ) + r_world_matrix[13];
			vert[2] = (v[0] * r_world_matrix[2]) + (v[1] * r_world_matrix[6]) + (v[2] * r_world_matrix[10]) + r_world_matrix[14];

			VectorNormalize (vert);
		
			glTexCoord2f (vert[0], vert[1]);
			glVertex3fv (v);
		}

		glEnd ();
	}

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4f(1,1,1,1);
}

/*
=============
EmitCausticsPolys
=============
*/
void EmitCausticsPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i, tn;

	tn = (int)(host_time * 20)%31;
	glBindTexture (GL_TEXTURE_2D, causticstexture[tn]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f (1,1,1,0.3f);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	for (p=s->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			glTexCoord2f (v[3], v[4]);
			glVertex3fv (v);
		}
		glEnd ();
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1,1,1,1);
}