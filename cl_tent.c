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
// cl_tent.c -- client side temporary entities

#include "quakedef.h"

int			num_temp_entities;
entity_t	cl_temp_entities[MAX_TEMP_ENTITIES];
beam_t		cl_beams[MAX_BEAMS];

sfx_t	*cl_sfx_wizhit;
sfx_t	*cl_sfx_knighthit;
sfx_t	*cl_sfx_tink1;
sfx_t	*cl_sfx_ric1;
sfx_t	*cl_sfx_ric2;
sfx_t	*cl_sfx_ric3;
sfx_t	*cl_sfx_r_exp3;

model_t	*cl_bolt1_mod;
model_t	*cl_bolt2_mod;
model_t	*cl_bolt3_mod;

/*
=================
CL_ParseTEnt
=================
*/
void CL_InitTEnts (void)
{
	cl_sfx_wizhit	= S_PrecacheSound ("wizard/hit.wav");
	cl_sfx_knighthit= S_PrecacheSound ("hknight/hit.wav");
	cl_sfx_tink1	= S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1		= S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2		= S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3		= S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3	= S_PrecacheSound ("weapons/r_exp3.wav");

	cl_bolt1_mod	= Mod_ForName ("progs/bolt.mdl", true);
	cl_bolt2_mod	= Mod_ForName ("progs/bolt2.mdl", true);
	cl_bolt3_mod	= Mod_ForName ("progs/bolt3.mdl", true);
}

/*
=================
CL_ParseBeam
=================
*/
void CL_ParseBeam (model_t *m)
{
	int		ent;
	vec3_t	start, end;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort ();
	
	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();
	
	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

// override any beam with the same entity
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}
	Con_Printf ("beam list overflow!\n");	
}

/*
=================
CL_ParseTEnt
=================
*/
void CL_ParseTEnt (void)
{
	int		amount, type, rnd, colorStart, colorLength;
	vec3_t	pos, pos2, angle;
	dlight_t	*dl;

	type = MSG_ReadByte ();
	switch (type)
	{
	case TE_WIZSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 20, 30);
		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;
		
	case TE_KNIGHTSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 226, 20);
		S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;
		
	case TE_SPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_SparkShower(pos, vec3_origin);	// Tomaz - Particle System

		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_PLASMA:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;

	case TE_SUPERSPIKE:			// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_SparkShower(pos, vec3_origin);	// Tomaz - Particle System

		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

// Tomaz - Particle System Begin
	case TE_SNOW:
		pos[0] = MSG_ReadCoord (); // mins
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		pos2[0] = MSG_ReadCoord (); // maxs
		pos2[1] = MSG_ReadCoord ();
		pos2[2] = MSG_ReadCoord ();
		amount	= MSG_ReadShort ();
		R_Snow(pos, pos2, amount);
		break;
		
	case TE_RAIN:
		pos[0]	= MSG_ReadCoord (); // mins
		pos[1]	= MSG_ReadCoord ();
		pos[2]	= MSG_ReadCoord ();
		pos2[0] = MSG_ReadCoord (); // maxs
		pos2[1] = MSG_ReadCoord ();
		pos2[2] = MSG_ReadCoord ();
		amount	= MSG_ReadShort ();
		R_Rain(pos, pos2, amount);
		break;			
// Tomaz - Particle System End

	case TE_GUNSHOT:			// bullet hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_SparkShower(pos, vec3_origin);	// Tomaz - Particle System
		break;
		
	case TE_EXPLOSION:			// rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosion (pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 300;
		dl->die = cl.time + 0.5;
		dl->decay = 300;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

// Tomaz - Particle System Begin		
	case TE_TAREXPLOSION:			// tarbaby explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosion (pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->color[0]	= 1;
		dl->color[1]	= 0;
		dl->color[2]	= 1;
		dl->radius		= 300;
		dl->die			= cl.time + 0.5;
		dl->decay		= 300;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
// Tomaz - Particle System End

	case TE_LIGHTNING1:				// lightning bolts
		CL_ParseBeam (cl_bolt1_mod);
		break;
	
	case TE_LIGHTNING2:				// lightning bolts
		CL_ParseBeam (cl_bolt2_mod);
		break;
	
	case TE_LIGHTNING3:				// lightning bolts
		CL_ParseBeam (cl_bolt3_mod);
		break;
	
// PGM 01/21/97 
	case TE_BEAM:				// grappling hook beam
		CL_ParseBeam (Mod_ForName("progs/beam.mdl", true));
		break;
// PGM 01/21/97

	case TE_LAVASPLASH:	
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_LavaSplash (pos);
		break;
	
	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_TeleportSplash (pos);
		break;

	case TE_EXPLOSION2:				// color mapped explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		colorStart = MSG_ReadByte ();
		colorLength = MSG_ReadByte ();
		R_ParticleExplosion2 (pos, colorStart, colorLength);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius	= 300;
		dl->die		= cl.time + 0.5;
		dl->decay	= 300;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

// Tomaz - RailTrail Begin
	case TE_RAILTRAIL: 
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		pos2[0] = MSG_ReadCoord();
		pos2[1] = MSG_ReadCoord();
		pos2[2] = MSG_ReadCoord();
		angle[0] = MSG_ReadCoord();
		angle[1] = MSG_ReadCoord();
		angle[2] = MSG_ReadCoord();
		R_RailTrail (pos, pos2, angle);
		break;
// Tomaz - RailTrail End		

	default:
		Sys_Error ("CL_ParseTEnt: bad type");
	}
}


/*
=================
CL_NewTempEntity
=================
*/
entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

	if (cl_numvisedicts == MAX_VISEDICTS)
		return NULL;
	if (num_temp_entities == MAX_TEMP_ENTITIES)
		return NULL;
	ent = &cl_temp_entities[num_temp_entities];
	memset (ent, 0, sizeof(*ent));
	num_temp_entities++;
	cl_visedicts[cl_numvisedicts] = ent;
	cl_numvisedicts++;

	ent->colormap = vid.colormap;
	return ent;
}


/*
=================
CL_UpdateTEnts
=================
*/
void CL_UpdateTEnts (void)
{
	int			i;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	entity_t	*ent;
	float		yaw, pitch;
	float		forward;

	num_temp_entities = 0;

// update lightning
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
			continue;

	// if coming from the player, update the start position
		if (b->entity == cl.viewentity)
		{
			VectorCopy (cl_entities[cl.viewentity].origin, b->start);
		}

	// calculate pitch and yaw
		VectorSubtract (b->end, b->start, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(dist[1], dist[0]) * 57.295779513082320);	// Tomaz Speed
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int) (atan2(dist[2], forward) * 57.295779513082320);	// Tomaz Speed
			if (pitch < 0)
				pitch += 360;
		}

	// add new entities for the lightning
		VectorCopy (b->start, org);
		d = VectorNormalize(dist);
		while (d > 0)
		{
			ent = CL_NewTempEntity ();
			if (!ent)
				return;
			VectorCopy (org, ent->origin);
			ent->model = b->model;
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;
			ent->angles[2] = rand()%360;

			org[0] += dist[0]*30;
			org[1] += dist[1]*30;
			org[2] += dist[2]*30;
			d -= 30;
		}
	}
	
}


