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
// particle engine
//

#include "quakedef.h"

typedef enum	ptype_s		ptype_t;
typedef struct	particle_s	particle_t;

enum ptype_s
{
	pt_static		= 1 << 0,
	pt_smoke		= 1 << 1,
	pt_bubble		= 1 << 2,
	pt_explode		= 1 << 3,
	pt_blood		= 1 << 4,
	pt_snow			= 1 << 5,
	pt_rain			= 1 << 6,
	pt_bulletpuff	= 1 << 7,
	pt_fade			= 1 << 8,
	pt_grav			= 1 << 9,
	pt_rail			= 1 << 10,
	pt_blob			= 1 << 11,
	pt_blob2		= 1 << 12,
	pt_smokeexp		= 1 << 13,
	pt_fire			= 1 << 14,
	pt_fire2		= 1 << 15
};


struct particle_s
{
	int					texnum;
	int					contents;
	int					fade;
	int					growth;

	float				die;
	float				time;
	float				alpha;
	float				scale;
	float				bounce;
	float				gravity;

	byte				colorred;
	byte				colorgreen;
	byte				colorblue;

	vec3_t				origin;
	vec3_t				velocity;

	ptype_t				type;

	qboolean			glow;

	particle_t			*next;
	particle_t			*prev;
};

int		particle_tex;
int		smoke1_tex;
int		smoke2_tex;
int		smoke3_tex;
int		smoke4_tex;
int		blood_tex;
int		bubble_tex;
int		snow_tex;
int		rain_tex;

particle_t	*active_particles, *free_particles, *particles;

int		r_numparticles;

vec3_t	r_pright, r_pup, r_ppn;

extern byte	particle[32][32];
extern byte	smoke1[32][32];
extern byte	smoke2[32][32];
extern byte	smoke3[32][32];
extern byte	smoke4[32][32];
extern byte	blood[32][32];
extern byte	bubble[32][32];
extern byte	snow[32][32];
extern byte	rain[32][32];

void R_InitParticleTexture (void)
{
	int		x, y;
	byte	data[32][32][4];

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= particle[x][y];
		}
	}
	particle_tex = GL_LoadTexture ("particle", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke1[x][y];
		}
	}
	smoke1_tex = GL_LoadTexture ("smoke1", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke2[x][y];
		}
	}
	smoke2_tex = GL_LoadTexture ("smoke2", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke3[x][y];
		}
	}
	smoke3_tex = GL_LoadTexture ("smoke3", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke4[x][y];
		}
	}
	smoke4_tex = GL_LoadTexture ("smoke4", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= blood[x][y];
		}
	}
	blood_tex = GL_LoadTexture ("blood", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= bubble[x][y];
		}
	}
	bubble_tex = GL_LoadTexture ("bubble", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= snow[x][y];
		}
	}
	snow_tex = GL_LoadTexture ("snow", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= rain[x][y];
		}
	}
	rain_tex = GL_LoadTexture ("rain", 32, 32, &data[0][0][0], true, true, 4);
}

/*
===============
R_InitParticles
===============
*/
void R_InitParticles (void)
{
	int		i = COM_CheckParm ("-particles");

	r_numparticles = 4096;

	if (i)
	{
		r_numparticles = (int)(atoi(com_argv[i+1]));
	}

	particles = (particle_t *) Hunk_AllocName (r_numparticles * sizeof(particle_t), "particles");

	memset(particles, 0, r_numparticles * sizeof(particle_t));

	for( i = 0 ; i < r_numparticles ; ++i )
	{
		particles[i].prev	= &particles[i - 1];
		particles[i].next	= &particles[i + 1];
	}

	particles[0].prev		= NULL;
	particles[r_numparticles - 1].next	= NULL;

	free_particles			= &particles[0];
	active_particles		= NULL;

	R_InitParticleTexture ();
}

/*
===============
R_ClearParticles
===============
*/
void R_ClearParticles (void)
{
	int		i;

	free_particles			= &particles[0];
	active_particles		= NULL;

	memset(particles, 0, r_numparticles * sizeof(particle_t));

	for( i = 0 ; i < r_numparticles ; ++i )
	{
		particles[i].prev	= &particles[i - 1];
		particles[i].next	= &particles[i + 1];
	}

	particles[0].prev		= NULL;
	particles[r_numparticles - 1].next	= NULL;
}

particle_t* addParticle()
{
	particle_t*	p	= free_particles;

	if(!p)
	{
		return NULL;
	}

	free_particles	= p->next;

	if(free_particles)
	{
		free_particles->prev	= NULL;
	}

	p->next			= active_particles;

	if(active_particles)
	{
		active_particles->prev	= p;
	}

	active_particles			= p;

	return p;
}

particle_t* remParticle(particle_t*	p)
{
	particle_t*	next;

	if(!p)
	{
		return NULL;
	}

	if(p == active_particles)
	{
		active_particles	= p->next;
	}

	if(p->next)
	{
		p->next->prev		= p->prev;
	}

	if(p->prev)
	{
		p->prev->next		= p->next;
	}

	if(free_particles)
	{
		free_particles->prev	= p;
	}

	next					= p->next;

	p->next					= free_particles;

	free_particles			= p;

	p->prev	= NULL;

	return next;
}

/*
===============
R_ReadPointFile
===============
*/
void R_ReadPointFile_f (void)
{
	FILE	*f;
	vec3_t	origin;
	int		r;
	int		c;
	particle_t	*p;
	char	name[MAX_OSPATH];
	byte	*color;

	_snprintf (name,sizeof(name),"maps/%s.pts", sv.name);

	COM_FOpenFile (name, &f);
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}
	
	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for ( ;; )
	{

		r = fscanf (f,"%f %f %f\n", &origin[0], &origin[1], &origin[2]);
		if (r != 3)
			break;
		c++;
		
		p	= addParticle();
		if(!p)
		{
			Con_Printf ("Not enough free particles\n");
			break;
		}

		color = (byte *) &d_8to24table[(int)(-c)&15];		

		p->fade				= 0;
		p->growth			= 0;
		p->texnum			= particle_tex;
		p->contents			= 0;

		p->die				= 99;
		p->time				= 0;
		p->alpha			= 200;
		p->scale			= 2;
		p->bounce			= 0;
		p->gravity			= 0;

		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];

		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];
		p->velocity[0]		= vec3_origin[0];
		p->velocity[1]		= vec3_origin[1];
		p->velocity[2]		= vec3_origin[2];

		p->type				= pt_static;

		p->glow				= true;
	}

	fclose (f);
	Con_Printf ("%i points read\n", c);
}

/*
===============
R_EntityParticles
===============
*/

float	r_avertexnormals[162][3] = 
{
	#include "anorms.h"
};

vec3_t			avelocities[128];

void R_EntityParticles (entity_t *ent)
{
	int			i;
	float		sp, sy, cp, cy, angle;
	vec3_t		forward;
	particle_t	*p;

	if (!avelocities[0][0])
	{
		for (i=0 ; i<384 ; i++)
		{
			avelocities[0][i] = (rand() & 255) * 0.01;
		}
	}

	for (i=0 ; i<128 ; i++)
	{
		angle				= cl.time * avelocities[i][0];
		sy					= sin(angle);
		cy					= cos(angle);

		angle				= cl.time * avelocities[i][1];
		sp					= sin(angle);
		cp					= cos(angle);
	
		forward[0]			= cp*cy;
		forward[1]			= cp*sy;
		forward[2]			= -sp;

		p	= addParticle();
		if(!p)
		{
			return;
		}

		p->fade				= 0;
		p->growth			= 0;
		p->texnum			= particle_tex;
		p->contents			= 0;

		p->die				= cl.time + 0.01;
		p->time				= 0;
		p->alpha			= 200;
		p->scale			= 2;
		p->bounce			= 0;
		p->gravity			= 0;

		p->colorred			= 255;
		p->colorgreen		= 243;
		p->colorblue		= 27;

		p->origin[0]		= ent->origin[0] + r_avertexnormals[i][0]*64 + forward[0]*16;			
		p->origin[1]		= ent->origin[1] + r_avertexnormals[i][1]*64 + forward[1]*16;			
		p->origin[2]		= ent->origin[2] + r_avertexnormals[i][2]*64 + forward[2]*16;			
		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= 0;

		p->type				= pt_static;

		p->glow				= true;
	}
}

/*
===============
R_ParseParticleEffect

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect (void)
{
	vec3_t		origin, direction;
	int			count, color;

	origin[0]		= MSG_ReadCoord ();
	origin[1]		= MSG_ReadCoord ();
	origin[2]		= MSG_ReadCoord ();

	direction[0]	= MSG_ReadChar () * 0.0625;
	direction[1]	= MSG_ReadChar () * 0.0625;
	direction[2]	= MSG_ReadChar () * 0.0625;

	count			= MSG_ReadByte ();
	color			= MSG_ReadByte ();

	R_RunParticleEffect (origin, direction, color, count);
}
	
/*
===================
R_ParticleExplosion
===================
*/

extern	cvar_t		sv_gravity;

void R_ParticleExplosion (vec3_t origin)
{
	int			i, contents;
	particle_t	*p;
	byte		*color;

	contents = Mod_PointInLeaf(origin, cl.worldmodel)->contents;

	for (i=0 ; i<64 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];

		p->fade				= -255;
		p->growth			= 16;
		p->texnum			= smoke1_tex + (rand() & 3);
		p->contents			= contents;

		p->die				= cl.time + 5;
		p->time				= 0;
		p->alpha			= 200;
		p->scale			= 2;
		p->bounce			= 0;
		p->gravity			= 0;

		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];

		p->origin[0]		= origin[0] + (rand() &31) - 16;
		p->origin[1]		= origin[1] + (rand() &31) - 16;
		p->origin[2]		= origin[2] + (rand() &31) - 16;
		p->velocity[0]		= (rand() & 3) - 2;
		p->velocity[1]		= (rand() & 3) - 2;
		p->velocity[2]		= (rand() & 3) - 2;

		p->type				= pt_smokeexp;

		p->glow				= true;
	}

	for (i=0 ; i<256 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		if ((contents == CONTENTS_EMPTY) || (contents == CONTENTS_SOLID))
		{
			p->fade			= -128;
			p->growth		= -2;
			p->texnum		= particle_tex;
			p->contents		= contents;
			
			p->die			= cl.time + 5;
			p->time			= 0;
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->bounce		= 1.5;
			p->gravity		= -0.5f * sv_gravity.value;

			p->colorred		= 255;
			p->colorgreen	= 243;
			p->colorblue	= 27;

			p->origin[0]	= origin[0] + ((rand() & 31) - 16);
			p->origin[1]	= origin[1] + ((rand() & 31) - 16);
			p->origin[2]	= origin[2] + ((rand() & 31) - 16);
			p->velocity[0]	= (rand() & 511) - 256;
			p->velocity[1]	= (rand() & 511) - 256;
			p->velocity[2]	= (rand() & 511) - 256;

			p->type			= pt_explode;

			p->glow			= true;
		}
		else
		{
			p->fade			= 0;
			p->growth		= 0;
			p->texnum		= bubble_tex;
			p->contents		= contents;

			p->die			= cl.time + 5;
			p->time			= 0;
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->bounce		= 0;
			p->gravity		= 0;

			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;

			p->origin[0]	= origin[0] + ((rand() & 31) - 16);
			p->origin[1]	= origin[1] + ((rand() & 31) - 16);
			p->origin[2]	= origin[2] + ((rand() & 31) - 16);
			p->velocity[0]	= (rand() & 511) - 256;
			p->velocity[1]	= (rand() & 511) - 256;
			p->velocity[2]	= (rand() & 511) - 256;

			p->type			= pt_bubble;

			p->glow			= false;
		}
	}
}

/*
====================
R_ParticleExplosion2
====================
*/
void R_ParticleExplosion2 (vec3_t origin, int colorStart, int colorLength)
{
	int			i,contents;
	particle_t	*p;
	int			colorMod = 0;
	byte		*color;

	contents = Mod_PointInLeaf(origin, cl.worldmodel)->contents;

	for (i=0; i<384; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			color			= (byte *) &d_8to24table[(int)colorStart + (colorMod % colorLength)];
			colorMod++;
			
			p->fade			= -128;
			p->growth		= -2;
			p->texnum		= particle_tex;
			p->contents		= contents;

			p->die			= cl.time + 5;
			p->time			= 0;
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->bounce		= 0;
			p->gravity		= -0.5f * sv_gravity.value;

			p->colorred		= color[0];
			p->colorgreen	= color[1];
			p->colorblue	= color[2];

			p->origin[0]	= origin[0] + ((rand() & 31) - 16);
			p->origin[1]	= origin[1] + ((rand() & 31) - 16);
			p->origin[2]	= origin[2] + ((rand() & 31) - 16);
			p->velocity[0]	= (rand() & 511) - 256;
			p->velocity[1]	= (rand() & 511) - 256;
			p->velocity[2]	= (rand() & 511) - 256;

			p->type			= pt_explode;

			p->glow			= true;
		}
		else
		{
			p->fade			= 0;
			p->growth		= 0;
			p->texnum		= bubble_tex;
			p->contents		= contents;

			p->die			= cl.time + 5;
			p->time			= 0;
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->bounce		= 0;
			p->gravity		= 0;

			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;

			p->origin[0]	= origin[0] + ((rand() & 31) - 16);
			p->origin[1]	= origin[1] + ((rand() & 31) - 16);
			p->origin[2]	= origin[2] + ((rand() & 31) - 16);
			p->velocity[0]	= (rand() & 511) - 256;
			p->velocity[1]	= (rand() & 511) - 256;
			p->velocity[2]	= (rand() & 511) - 256;

			p->type			= pt_bubble;

			p->glow			= false;
		}
	}
}

/*
===============
R_BlobExplosion
===============
*/
void R_BlobExplosion (vec3_t origin)
{
	int			i;
	particle_t	*p;
	byte		*color;
	
	for (i=0 ; i<384 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		color				= (byte *) &d_8to24table[(int)(rand() & 7) + 66];

		p->fade				= -128;
		p->growth			= -2;
		p->texnum			= particle_tex;
		p->contents			= 0;
			
		p->die				= cl.time + 5;
		p->time				= 0;
		p->alpha			= 200;
		p->scale			= (rand() & 3) +1;
		p->bounce			= 0;
		p->gravity			= -0.5f * sv_gravity.value;

		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];

		p->origin[0]		= origin[0] + (rand() & 31) - 16;
		p->origin[1]		= origin[1] + (rand() & 31) - 16;
		p->origin[2]		= origin[2] + (rand() & 31) - 16;
		p->velocity[0]		= (rand() & 511) - 256;
		p->velocity[1]		= (rand() & 511) - 256;
		p->velocity[2]		= (rand() & 511) - 256;

		p->type				= pt_blob;

		p->glow				= true;
		
		if (i & 1)
		{
			color			= (byte *) &d_8to24table[(int)(rand() & 7) + 150];

			p->colorred		= color[0];
			p->colorgreen	= color[1];
			p->colorblue	= color[2];

			p->type			= pt_blob2;
		}
	}
}

/*
===============
R_RunParticleEffect

===============
*/

void R_RunParticleEffect (vec3_t origin, vec3_t direction, int color, int count)
{
	int			i, contents;
	particle_t	*p;
	byte		*color24;

	if (count == 128)
	{
		for (i=0 ; i<count ; i++)
		{	// rocket explosion
			p	= addParticle();
			if(!p)
			{
				return;
			}

			contents			= Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;

			p->scale			= 2;
			p->alpha			= 200;
			p->die				= cl.time + 5;
			
			if ((contents		== CONTENTS_EMPTY) ||
				(contents		== CONTENTS_SOLID))
			{
				p->texnum		= particle_tex;
				p->bounce		= 0;

				p->colorred		= 255;
				p->colorgreen	= 243;
				p->colorblue	= 147;

				p->fade			= -128;
				p->growth		= -2;
				p->gravity		= -0.5f * sv_gravity.value;

				p->type			= pt_explode;
				p->glow			= true;
			}
			else
			{
				p->texnum		= bubble_tex;
				p->bounce		= 0;

				p->colorred		= 127;
				p->colorgreen	= 127;
				p->colorblue	= 255;

				p->fade			= 0;
				p->growth		= 0;
				p->gravity		= 0;

				p->type			= pt_bubble;
				p->glow			= false;
			}

			p->origin[0]		= origin[0] + ((rand() & 31) - 16);
			p->origin[1]		= origin[1] + ((rand() & 31) - 16);
			p->origin[2]		= origin[2] + ((rand() & 31) - 16);

			p->velocity[0]		= (rand() & 511) - 256;
			p->velocity[1]		= (rand() & 511) - 256;
			p->velocity[2]		= (rand() & 511) - 256;
		}
		return;
	}

	for (i=0 ; i<count ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		p->texnum			= blood_tex;
		p->bounce			= 0;
		p->scale			= 2;
		p->alpha			= 200;
		p->die				= cl.time + 5;

		color24				= (byte *) &d_8to24table[(int)(rand() & 3) + color];

		p->colorred			= color24[0];
		p->colorgreen		= color24[1];
		p->colorblue		= color24[2];

		p->growth			= 0;
		p->fade				= -64;
		p->gravity			= -0.5f * sv_gravity.value;

		p->type				= pt_blood;
		p->glow				= false;

		p->origin[0]		= origin[0] + ((rand() & 15) - 8);
		p->origin[1]		= origin[1] + ((rand() & 15) - 8);
		p->origin[2]		= origin[2] + ((rand() & 15) - 8);

		p->velocity[0]		= direction[0] * 15;
		p->velocity[1]		= direction[1] * 15;
		p->velocity[2]		= direction[2] * 15;
	}
}

/*
===============
R_SparkShower

===============
*/
void R_SparkShower (vec3_t origin, vec3_t direction)
{
	int			i, contents;
	particle_t	*p;

	p	= addParticle();
	if (!free_particles)
	{
		return;
	}

	contents			= Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;

	if ((contents		== CONTENTS_EMPTY) ||
		(contents		== CONTENTS_SOLID))
	{
		p->scale			= 2;
		p->alpha			= 200;
		p->texnum			= smoke1_tex + (rand() & 3);
		p->bounce			= 0;
		p->type				= pt_bulletpuff;
		p->glow				= false;

		p->colorred			= 187;
		p->colorgreen		= 187;
		p->colorblue		= 187;

		p->fade				= -255;
		p->growth			= 16;
		p->gravity			= 0;

		p->die				= cl.time + 5;

		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];

		p->velocity[0]		= (rand() & 7) - 4;
		p->velocity[1]		= (rand() & 7) - 4;
		p->velocity[2]		= (rand() & 7) - 4;
	}

	for (i=0 ; i<10 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		contents			= Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;

		p->scale			= (rand() & 3) +1;

		p->alpha			= 200;
		p->die				= cl.time + 5;

		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			p->texnum		= particle_tex;
			p->bounce		= 1.5;

			p->colorred		= 255;
			p->colorgreen	= 243;
			p->colorblue	= 147;

			p->fade			= -128;
			p->growth		= -2;
			p->gravity		= -0.5f * sv_gravity.value;

			p->type			= pt_explode;
			p->glow			= true;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;

			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;

			p->fade			= 0;
			p->growth		= 0;
			p->gravity		= 0;

			p->type			= pt_bubble;
			p->glow			= false;
		}

		p->origin[0]		= origin[0] + ((rand() & 7) - 4);
		p->origin[1]		= origin[1] + ((rand() & 7) - 4);
		p->origin[2]		= origin[2] + ((rand() & 7) - 4);

		p->velocity[0]		= direction[0] + (rand() & 127) - 64;
		p->velocity[1]		= direction[1] + (rand() & 127) - 64;
		p->velocity[2]		= direction[2] + (rand() & 127) - 64;
	}
}

/*
===============
R_Snow
===============
*/
void R_Snow (vec3_t min, vec3_t max, int flakes)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;

	for (i=0 ; i<flakes ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		VectorSubtract(max, min, difference);

		p->die				= cl.time + 10;

		p->scale			= (rand() & 3) +1;
		p->alpha			= 200;
		p->texnum			= snow_tex;
		p->bounce			= 0;
		p->type				= pt_snow;
		p->glow				= false;

		p->colorred			= 255;
		p->colorgreen		= 255;
		p->colorblue		= 255;		

		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;

		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= -50;
	}
}

/*
===============
R_Rain
===============
*/
void R_Rain (vec3_t min, vec3_t max, int drops)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;

	for (i=0 ; i<drops ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		VectorSubtract(max, min, difference);

		p->die				= cl.time + 10;

		p->scale			= (rand() & 3) +1;
		p->alpha			= 200;

		p->texnum			= rain_tex;
		p->bounce			= 0;
		p->type				= pt_rain;
		p->glow				= false;

		p->colorred			= 255;
		p->colorgreen		= 255;
		p->colorblue		= 255;

		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;

		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= -400;
	}
}

/*
===============
R_LavaSplash
===============
*/
void R_LavaSplash (vec3_t origin)
{
	int			i, j;
	particle_t	*p;

	for (i=-16 ; i<16 ; i++)
	{	
		for (j=-16 ; j<16 ; j++)
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}
		
			p->texnum			= particle_tex;
			p->bounce			= 0;
			p->scale			= 5;
			p->alpha			= 255;
			p->die				= cl.time + 10;

			p->fade				= -64;
			p->growth			= 0;
			p->gravity			= -0.1f * sv_gravity.value;

			p->colorred			= 163;
			p->colorgreen		= 39;
			p->colorblue		= 11;			

			p->type				= pt_grav;
			p->glow				= true;
				
			p->velocity[0]		= (rand() & 7) + (j * 8);
			p->velocity[1]		= (rand() & 7) + (i * 8);
			p->velocity[2]		= 256;
	
			p->origin[0]		= origin[0] + (rand() & 7) + (j * 8);
			p->origin[1]		= origin[1] + (rand() & 7) + (i * 8);
			p->origin[2]		= origin[2] + (rand() & 63);
		}
	}
}

/*
===============
R_TeleportSplash

===============
*/
void R_TeleportSplash (vec3_t origin)
{
	int			i, j, k;
	particle_t	*p;

	for (i=-16 ; i<16 ; i+=4)
	{	
		for (j=-16 ; j<16 ; j+=4)
		{
			for (k=-24 ; k<32 ; k+=4)
			{
				p	= addParticle();
				if(!p)
				{
					return;
				}
		
				p->texnum			= particle_tex;
				p->bounce			= 0;
				p->scale			= 2;
				p->alpha			= 200;
				p->die				= cl.time + 1;
				p->gravity			= -0.05f * sv_gravity.value;

				p->colorred			= 255;
				p->colorgreen		= 255;
				p->colorblue		= 255;				

				p->fade				= -255;

				p->type				= pt_fade;
				p->glow				= true;
	
				p->origin[0]		= origin[0] + i + (rand() & 7);
				p->origin[1]		= origin[1] + j + (rand() & 7);
				p->origin[2]		= origin[2] + k + (rand() & 7);
	
				p->velocity[0]		= i*2 + (rand() & 31) - 16;
				p->velocity[1]		= j*2 + (rand() & 31) - 16;
				p->velocity[2]		= k*2 + (rand() & 31) + 24;
			}
		}
	}
}

void R_RailTrail (vec3_t start, vec3_t end, vec3_t angle)
{
	vec3_t		vec;
	float		len;
	vec3_t		forward, right, up;
	particle_t	*p;
	byte		*color;

	VectorSubtract (end, start, vec);

	len = VectorNormalize(vec);

	while (len > 0)
	{
		{  
			p	= addParticle();
			if(!p)
			{
				return;
			}

			p->alpha			= 200;
			p->scale			= 2;
			p->die				= cl.time + 1;
			p->glow				= true;

			p->colorred			= 255;
			p->colorgreen		= 255;
			p->colorblue		= 255;

			p->fade				= -255;
			p->growth			= 0;
			p->gravity			= 0;

			p->type				= pt_rail;
			p->texnum			= particle_tex;
			p->bounce			= 0;

			p->origin[0]		= start[0];
			p->origin[1]		= start[1];
			p->origin[2]		= start[2];

			p->velocity[0]		= 0;
			p->velocity[1]		= 0;
			p->velocity[2]		= 0;
		}

		p	= addParticle();
		if(!p)
		{
			return;
		}

		p->alpha			= 200;
		p->scale			= 4;
		p->die				= cl.time + 1;
		p->glow				= true;

		color				= (byte *) &d_8to24table[(int)(rand() & 7) + 208];

		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];

		p->fade				= -255;
		p->growth			= 0;
		p->gravity			= 0;

		p->type				= pt_rail;
		p->texnum			= particle_tex;
		p->bounce			= 0;

		AngleVectors (angle, forward, right, up);

		p->origin[0]		= start[0] + right[0] * 3;
		p->origin[1]		= start[1] + right[1] * 3;
		p->origin[2]		= start[2] + right[2] * 3;
		
		p->velocity[0]		= right[0] * 10;
		p->velocity[1]		= right[1] * 10;
		p->velocity[2]		= right[2] * 10;

		angle[2] += 5;

		len--;
		VectorAdd (start, vec, start);
	}                                     
}

void R_RocketTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	int			contents;
	particle_t	*p;

	if (ent->debris_smoke <= 0)
		return;

	contents = Mod_PointInLeaf(start, cl.worldmodel)->contents;

	if (contents == CONTENTS_SKY || contents == CONTENTS_LAVA)
		return;

	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		VectorCopy(start, p->origin);

		if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
		{
			p->die			= cl.time + 10;

			p->colorred		= 187;
			p->colorgreen	= 187;
			p->colorblue	= 187;			

			p->fade			= -255;
			p->growth		= 16;
			p->gravity		= 0;

			p->alpha		= ent->debris_smoke * 28;
			p->scale		= 2;
			p->texnum		= smoke1_tex + (rand() & 3);
			p->type			= pt_smoke;
			p->bounce		= 0;

			p->velocity[0]	= (rand() & 15) - 8;
			p->velocity[1]	= (rand() & 15) - 8;
			p->velocity[2]	= (rand() & 31) + 15;
		}
		else
		{
			p->die			= cl.time + 10;

			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;

			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			p->type			= pt_bubble;

			p->fade			= 0;
			p->growth		= 0;
			p->gravity		= 0;

			p->velocity[0]	= 0;
			p->velocity[1]	= 0;
			p->velocity[2]	= 20;
		}
		p->glow				= false;
		ent->time_left		= cl.time + 0.05;
		ent->debris_smoke	-= 0.05f;
	}
}

void R_BloodTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	particle_t	*p;
	byte		*color;

	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		p->die				= cl.time + 10;

		color				= (byte *) &d_8to24table[(int)(rand() & 3) + 68];

		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];

		p->fade				= -128;
		p->gravity			= -0.5f * sv_gravity.value;

		p->alpha			= 200;
		p->scale			= 4;
		p->texnum			= blood_tex;
		p->bounce			= 0;
		p->type				= pt_blood;
		p->glow				= false;

		p->velocity[0]		= (rand() & 15) - 8;
		p->velocity[1]		= (rand() & 15) - 8;
		p->velocity[2]		= (rand() & 15) - 8;

		p->origin[0]		= start[0] + ((rand() & 3) - 2);
		p->origin[1]		= start[1] + ((rand() & 3) - 2);
		p->origin[2]		= start[2] + ((rand() & 3) - 2);

		ent->time_left		= cl.time + 0.05;
	}
}

void R_TracerTrail (vec3_t start, vec3_t end, entity_t *ent, byte color)
{
	vec3_t		vec;
	static int	tracercount;
	particle_t	*p;
	byte		*color24;
	
	VectorSubtract (end, start, vec);

	if (cl.time > ent->time_left)
	{
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}

			VectorCopy (start, p->origin);

			p->die				= cl.time + 0.5;

			color24				= (byte *) &d_8to24table[(int)color];

			p->colorred			= color24[0];
			p->colorgreen		= color24[1];
			p->colorblue		= color24[2];

			p->alpha			= 200;
			p->scale			= 5;
			p->texnum			= particle_tex;
			p->bounce			= 0;
			p->type				= pt_static;
			p->glow				= true;

			p->velocity[0]		= 5 * -vec[1];
			p->velocity[1]		= 5 *  vec[0];
			p->velocity[2]		= 0;
		}

		{
			p	= addParticle();
			if(!p)
			{
				return;
			}

			VectorCopy (start, p->origin);

			p->die				= cl.time + 0.5;

			color24				= (byte *) &d_8to24table[(int)color];

			p->colorred			= color24[0];
			p->colorgreen		= color24[1];
			p->colorblue		= color24[2];

			p->alpha			= 200;
			p->scale			= 5;
			p->texnum			= particle_tex;
			p->bounce			= 0;
			p->type				= pt_static;
			p->glow				= true;

			p->velocity[0]		= 5 *  vec[1];
			p->velocity[1]		= 5 * -vec[0];
			p->velocity[2]		= 0;
		}
		ent->time_left			= cl.time + 0.05;
	}
}

void R_VoorTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	particle_t	*p;

	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		p->die				= cl.time + 5;

		p->colorred			= 187;
		p->colorgreen		= 115;
		p->colorblue		= 159;		

		p->fade				= -128;
		p->gravity			= -0.05f * sv_gravity.value;

		p->alpha			= 200;
		p->scale			= 5;
		p->texnum			= particle_tex;
		p->bounce			= 0;
		p->type				= pt_fade;
		p->glow				= true;

		p->velocity[0]		= (rand() & 15) - 8;
		p->velocity[1]		= (rand() & 15) - 8;
		p->velocity[2]		= (rand() & 15) - 8;

		p->origin[0]		= start[0] + ((rand() & 3) - 2);
		p->origin[1]		= start[1] + ((rand() & 3) - 2);
		p->origin[2]		= start[2] + ((rand() & 3) - 2);

		ent->time_left		= cl.time + 0.05;
	}
}

/*
==========
R_Fire
==========
*/
void R_Fire (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	
	if( cl.time + 2 < ent->time_left )
	{
		ent->time_left = 0;
	}
	
	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}

		p->die			= cl.time + 5;

		p->colorred		= 227;
		p->colorgreen	= 151;
		p->colorblue	= 79;

		p->fade			= -128;
		p->growth		= -2;
		p->gravity		= 0.05f * sv_gravity.value;

		p->alpha		= 128;
		p->scale		= 10;
		p->texnum		= particle_tex;
		p->bounce		= 0;
		p->type			= pt_fire;
		p->glow			= true;

		p->velocity[0]	= (rand() & 3) - 2;
		p->velocity[1]	= (rand() & 3) - 2;
		p->velocity[2]	= 0;

		p->origin[0]	= ent->origin[0];
		p->origin[1]	= ent->origin[1];
		p->origin[2]	= ent->origin[2] + 4;

		if (fire2)
		{
			p->origin[2]	= ent->origin[2] - 2;
			if (ent->frame)
			{
				p->scale		= 30;
				p->velocity[0]	= (rand() & 7) - 4;
				p->velocity[1]	= (rand() & 7) - 4;
				p->velocity[2]	= 0;
				p->fade			= -128;
				p->growth		= -4;
				p->gravity		= 0.1f * sv_gravity.value;
				p->type			= pt_fire2;
			}
		}
		ent->time_left	= cl.time + 0.05;
	}
}

#define	DIST_EPSILON	(0.03125)

int SV_HullPointContents (hull_t *hull, int num, vec3_t p);

qboolean detectCollision( int num, vec3_t start, vec3_t end, vec3_t impact, vec3_t normal )
{
	dclipnode_t	*node, *nodes = cl.worldmodel->hulls->clipnodes;
	mplane_t	*plane, *planes = cl.worldmodel->hulls->planes;
	float		t1, t2;
	float		frac;
	vec3_t		mid;
	int			side;

	qboolean	t1neg, t2neg;

	while( num >= 0 )
	{
		t1neg = false;
		t2neg = false;

		node	= nodes + num;
		plane	= planes + node->planenum;

		t1		= PlaneDiff(start, plane);
		t2		= PlaneDiff(end, plane);

		if( t1 < 0 )
		{
			t1neg	= true;
		}
		if( t2 < 0 )
		{
			t2neg	= true;
		}
		
		if( !t1neg && !t2neg )
		{
			num	= node->children[0];
			continue;
		}

		if( t1neg && t2neg )
		{
			num	= node->children[1];
			continue;
		}

		if( t1neg )
		{
			frac	= (t1 + DIST_EPSILON)/(t1-t2);
		}
		else
		{
			frac	= (t1 - DIST_EPSILON)/(t1-t2);
		}

		if(frac < 0)
		{
			frac	= 0;
		}

		if(frac > 1)
		{
			frac	= 1;
		}
			
		mid[0]	= start[0] + frac*(end[0] - start[0]);
		mid[1]	= start[1] + frac*(end[1] - start[1]);
		mid[2]	= start[2] + frac*(end[2] - start[2]);

		side = ( t1 < 0 );

		if( detectCollision( node->children[side], start, mid, impact, normal ) )
		{
			return true;
		}

		if( SV_HullPointContents( cl.worldmodel->hulls, node->children[side^1], mid ) != CONTENTS_SOLID )
		{
			num		= node->children[ side^1 ];
			VectorCopy( mid, start );

			continue;
		}
		
		if( !side )
		{
			VectorCopy( plane->normal, normal );
		}
		else
		{
			VectorNegate( plane->normal, normal );
		}

		VectorCopy( mid, impact );
		
		return true;
	}

	return false;
}

void R_MoveParticles( void )
{
	particle_t	*p				= active_particles;

	float		frametime		= cl.time - cl.oldtime;

	vec3_t		impact, normal, oldorigin;
	float		dist;

	while( p )
	{
		if ((p->die < cl.time) || (!gl_particles.value))
		{
			p	= remParticle( p );
			continue;
		}

		VectorCopy(p->origin, oldorigin);

		p->origin[0] += p->velocity[0] * frametime;
		p->origin[1] += p->velocity[1] * frametime;
		p->origin[2] += p->velocity[2] * frametime;

		p->velocity[2]		+= frametime * p->gravity;
		p->alpha			+= frametime * p->fade;
		p->scale			+= frametime * p->growth;

		switch( p->type )
		{
			case pt_bubble:
			{
				p->velocity[0] = (rand() & 15) - 8;
				p->velocity[1] = (rand() & 15) - 8;
				p->velocity[2] = (rand() & 31) + 64;
				
				break;
			}

			case pt_snow:
			{
				if (cl.time > p->time)
				{
					p->time = cl.time + (rand() & 3) * 0.1;
					p->velocity[0] = (rand() & 31) - 16;
					p->velocity[1] = (rand() & 31) - 16;
				}
				
				break;
			}
		}

		if (p->alpha <= 0.0f || p->scale <= 0.0f)
		{
			p	= remParticle( p );
			continue;
		}		

		p->contents = Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;

		if  ((p->contents == CONTENTS_SKY) ||
			((p->contents == CONTENTS_SOLID && !p->bounce)) ||
			((p->contents == CONTENTS_EMPTY) && (p->type & pt_bubble)) ||
			((p->contents != CONTENTS_EMPTY) && (p->contents != CONTENTS_SOLID) && (p->type & (pt_explode | pt_bulletpuff | pt_smokeexp))))
		{
			p	= remParticle (p);
			continue;
		}

		if (p->bounce)
		{
			if (detectCollision (0, oldorigin, p->origin, impact, normal))
			{
				VectorCopy (impact, p->origin);
				
				dist = DotProduct (p->velocity, normal) * -p->bounce;
				
				VectorMA (p->velocity, dist, normal, p->velocity);
				
				if( DotProduct (p->velocity, p->velocity) < 0.03 )
				{
					VectorClear (p->velocity);
					p->bounce = 0;
				}
			}
		}

		p	= p->next;
	}
}

void R_DrawParticles (qboolean inwater)
{
	int			texnum;
	
	byte		alpha;
	
	vec3_t		up		= {vup[0]    * 0.75f, vup[1]    * 0.75f, vup[2]    * 0.75f};
	vec3_t		right	= {vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f};
	vec3_t		coord[4];

	particle_t	*p		= active_particles;	

	if( !p )
	{
		return;
	}
		
	VectorAdd      (up, right, coord[0]);
	VectorSubtract (right, up, coord[1]);
	VectorNegate   (coord[0], coord[2]);
	VectorNegate   (coord[1], coord[3]);

	texnum = p->texnum;
	glBindTexture (GL_TEXTURE_2D, texnum);
	glTexEnvf     (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDepthMask   (false);
	glBlendFunc   (GL_SRC_ALPHA, GL_ONE);	
	
	while (p)
	{
		if (p->texnum != texnum)
		{
			texnum = p->texnum;
			glBindTexture (GL_TEXTURE_2D, texnum);
		}
		
		if ((!inwater && p->contents != CONTENTS_EMPTY) || (inwater && p->contents == CONTENTS_EMPTY))
		{
			p = p->next;
			continue;
		}
				
		alpha	= p->alpha;
		
		glColor4ub(p->colorred, p->colorgreen, p->colorblue, alpha);
				
		glPushMatrix();
		{	
			glTranslatef( p->origin[0], p->origin[1], p->origin[2] );
			glScalef( p->scale, p->scale, p->scale );
			
			glBegin (GL_QUADS);
			{
				glTexCoord2f (0, 1);
				glVertex3fv  (coord[0]);
				glTexCoord2f (0, 0);
				glVertex3fv  (coord[1]);
				glTexCoord2f (1, 0); 
				glVertex3fv  (coord[2]);
				glTexCoord2f (1, 1);
				glVertex3fv  (coord[3]);
				
			}
			glEnd ();
			
		}
		glPopMatrix ();
				
		p = p->next;
	}

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf   (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDepthMask (true);
	glColor4f   (1,1,1,1);
}