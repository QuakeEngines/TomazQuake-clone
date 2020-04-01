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
// this file is specific to the crosshair, since we use several crosshairs
//
#include "quakedef.h"


float glowcos[17] = 
{
	 1.000000f,
	 0.923880f,
	 0.707105f,
	 0.382680f,
	 0.000000f,
	-0.382680f,
	-0.707105f,
	-0.923880f,
	-1.000000f,
	-0.923880f,
	-0.707105f,
	-0.382680f,
	 0.000000f,
	 0.382680f,
	 0.707105f,
	 0.923880f,
	 1.000000f
};


float glowsin[17] = 
{
	 0.000000f,
	 0.382680f,
	 0.707105f,
	 0.923880f,
	 1.000000f,
	 0.923880f,
	 0.707105f,
	 0.382680f,
	-0.000000f,
	-0.382680f,
	-0.707105f,
	-0.923880f,
	-1.000000f,
	-0.923880f,
	-0.707105f,
	-0.382680f,
	 0.000000f
};

void R_DrawGlows (entity_t *e)
{
	vec3_t	lightorigin;	// Origin of torch.
	vec3_t	v;				// Vector to torch.
	float	radius;			// Radius of torch flare.
	float	distance;		// Vector distance to torch.
	float	intensity;		// Intensity of torch flare.
	int			i;
	model_t		*clmodel;

	if (gl_fogenable.value)
	{
		glDisable(GL_FOG);
	}	

	clmodel = e->model;

	// NOTE: this isn't centered on the model
	VectorCopy(e->origin, lightorigin);	

	// set radius based on what model we are doing here
	radius = clmodel->glow_radius;

	VectorSubtract(lightorigin, r_origin, v);

	// See if view is outside the light.
	distance = Length(v);
	if (distance > radius)
	{
		glDepthMask (false);
		glDisable (GL_TEXTURE_2D);
		glBlendFunc (GL_ONE, GL_ONE);

		glBegin(GL_TRIANGLE_FAN);

		if (!strncmp (clmodel->name, "progs/glow_",11))
		{	
			lightorigin[2] += 4.0f;
			intensity = (1 - ((1024.0f - distance) * 0.0009765625)) * 0.75;
			glColor3f(1.0f*intensity, 0.7f*intensity, 0.4f*intensity);		
		}
		else
		{
			glColor3fv (clmodel->glow_color);				
		}

		VectorScale (vpn, -radius, v);
		VectorAdd (v, lightorigin, v);

		glVertex3fv(v);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i=16; i>=0; i--) 
		{
			v[0] = lightorigin[0] + radius * (vright[0] * glowcos[i] + vup[0] * glowsin[i]);
			v[1] = lightorigin[1] + radius * (vright[1] * glowcos[i] + vup[1] * glowsin[i]);
			v[2] = lightorigin[2] + radius * (vright[2] * glowcos[i] + vup[2] * glowsin[i]);
			glVertex3fv(v);
		}
		glEnd();
		glColor3f (1,1,1);
		glEnable (GL_TEXTURE_2D);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask (true);
	}
	if (gl_fogenable.value)
	{
		glEnable(GL_FOG);
	}
}