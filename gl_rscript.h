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

#define MAX_ANIM_FRAMES 20

typedef struct
{
	float	power;
	float	movediv;
} rturb_t;

typedef struct
{
	float	xspeed;
	float	yspeed;
} rscroll_t;

typedef struct
{
	float	alpha;
	BOOL	blendfunc;
	BOOL	envmap;
	float	animtime;
} rflags_t;

typedef struct
{
	char	name[56];
} rfname_t;

typedef struct
{
	int			num;
	rfname_t	name[MAX_ANIM_FRAMES];
	int			texnum[MAX_ANIM_FRAMES];
	int			current;
	double		lasttime;
} ranim_t;

typedef struct
{
	char				scriptname[56];
	char				texname[56];
	int					texnum;
	float				texscale;
	BOOL				texexist;
	BOOL				useturb;
	BOOL				usescroll;
	BOOL				usevturb;
	BOOL				useanim;
	rturb_t				turb;
	rturb_t				vturb;
	rscroll_t			scroll;
	rflags_t			flags;
	ranim_t				anim;
	int					nextstage;
	char				nextname[56];
} rscript_t;

#define MAX_RS	512

rscript_t	rscripts[MAX_RS];
int GetRSForName(char name[56]);