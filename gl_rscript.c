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
/*
RScript loading, parsing, and rendering.

Code syntax is similar to that of shaders
No reference to any shader material was used whilst
making this code, hence the weak and buggy state of it :)
*/

#include "quakedef.h"
#include "gl_rscript.h"
#include "bsp_render.h"

typedef struct
{
	int		texnum;
	float	sl, tl, sh, th;
} glpic_t;

extern	int			lightmap_textures;
extern	glpoly_t	*lightmap_polys[MAX_LIGHTMAPS];
extern	qboolean	lightmap_modified[MAX_LIGHTMAPS];
extern	glRect_t	lightmap_rectchange[MAX_LIGHTMAPS];
extern	byte		lightmaps[4*MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];

int RS_AnimTexture(int rs)
{
	double rt;

	if (host_time < rscripts[rs].anim.lasttime)
		rscripts[rs].anim.lasttime = 0;

	rt = host_time - rscripts[rs].anim.lasttime;

	if (rt < rscripts[rs].flags.animtime)
		return rscripts[rs].anim.texnum[rscripts[rs].anim.current];

	if (rt > rscripts[rs].flags.animtime)
	{
		rscripts[rs].anim.current	+= (rt / rscripts[rs].flags.animtime);

		while (rscripts[rs].anim.current >= rscripts[rs].anim.num)
			rscripts[rs].anim.current = rscripts[rs].anim.current - rscripts[rs].anim.num;

		rscripts[rs].anim.lasttime	+= rscripts[rs].flags.animtime;
	}
	return rscripts[rs].anim.texnum[rscripts[rs].anim.current];
}

float MakeMapXCoord(float x, int rs)
{
	float txm = 0;

	if (rs > MAX_RS) 
		rs = 0;

	if (!rs) 
		return x;

	if (rscripts[rs].usescroll) 
	{
		txm=realtime*rscripts[rs].scroll.xspeed;
		while (txm > 1 && (1-txm) > 0) txm=1-txm;
		while (txm < 0 && (1+txm) > 1) txm=1+txm;
	}

	if (rscripts[rs].useturb) 
	{
		float power, movediv;

		power		= rscripts[rs].turb.power * 0.05;
		movediv		= rscripts[rs].turb.movediv;
		x			+= sin((x*0.1+realtime) * power) * sin((x*0.1+realtime))/movediv;
	}
	x	+= txm; 

	return x*rscripts[rs].texscale;
}

float MakeMapYCoord(float y, int rs)
{
	float tym = 0;

	if (rs > MAX_RS) 
		rs=0;

	if (!rs) 
		return y;

	if (rscripts[rs].usescroll) 
	{
		tym=realtime*rscripts[rs].scroll.yspeed;
		while (tym > 1 && (1-tym) > 0) tym=1-tym;
		while (tym < 0 && (1+tym) > 1) tym=1+tym;
	}

	if (rscripts[rs].useturb) 
	{
		float power, movediv;

		movediv		= rscripts[rs].turb.movediv;
		power		= rscripts[rs].turb.power *0.05;
		y			+= sin((y*0.1+realtime) * power) * sin((y*0.1+realtime))/movediv;
	}
	y	+= tym; 
	
	return y*rscripts[rs].texscale;
}

void RS_DrawPic (int x, int y, qpic_t *pic)
{
	int				rs;
	glpic_t			*gl;
	qboolean		stage;
	float			tx,ty;

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	gl = (glpic_t *)pic->data;

	stage = true;
	
	rs = pic->rs;

	while (stage)
	{
		if (rscripts[rs].flags.blendfunc)
			glColor4f(1,1,1,rscripts[rs].flags.alpha);
		if (rscripts[rs].useanim)
			glBindTexture (GL_TEXTURE_2D, RS_AnimTexture(rs));

		else if (rscripts[rs].texnum)
			glBindTexture (GL_TEXTURE_2D, rscripts[rs].texnum);
		else
			glBindTexture (GL_TEXTURE_2D, gl->texnum);
			
		glBegin (GL_QUADS);

		tx	= MakeMapXCoord(gl->sl,rs);
		ty	= MakeMapYCoord(gl->tl,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x, y);

		tx	= MakeMapXCoord(gl->sh,rs);
		ty	= MakeMapYCoord(gl->tl,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x+pic->width, y);

		tx	= MakeMapXCoord(gl->sh,rs);
		ty	= MakeMapYCoord(gl->th,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x+pic->width, y+pic->height);

		tx	= MakeMapXCoord(gl->sl,rs);
		ty	= MakeMapYCoord(gl->th,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x, y+pic->height);

		glEnd ();

		if (rscripts[rs].flags.blendfunc)
			glColor4f(1,1,1,1); 

		if (rscripts[rs].nextstage)
			rs = rscripts[rs].nextstage;
		else
			stage = false;
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void FinishScripts(int i)
{
	int c;

	if (!strcmp(rscripts[i].scriptname,""))
		return;

	if (rscripts[i].nextname)
		rscripts[i].nextstage = GetRSForName(rscripts[i].nextname);

	if (!strcmp(rscripts[i].texname,""))
		strcpy(rscripts[i].texname,rscripts[i].scriptname);

	if (rscripts[i].anim.num) 
	{
		for (c=0;c<rscripts[i].anim.num;c++)
			rscripts[i].anim.texnum[c] = loadtextureimage(rscripts[i].anim.name[c].name, false, true);
	}

	rscripts[i].texnum = loadtextureimage(rscripts[i].texname, false, false);
}

int GetRSForName(char *name)
{
	int i;

	for (i=0;i<MAX_RS;i++)
	{
		if (!_stricmp(name,rscripts[i].scriptname))
		{
			FinishScripts(i);
			return i;
		}
	}
	return 0;
}

void InitRenderScripts()
{
	FILE		*f;
	char		ch[1024], sp1[1024], sp2[1024];
	float		fp1, fp2, fp3, fp4;
	int			inscript = 0, num = 1, i;

	COM_FOpenFile("scripts/textures.txt", &f);

	if (!f || feof(f)) 
	{
		Con_Printf(" &f9000 *Failed to load Rscript &r\n");
		return;
	}
	
	ch[0] = 0;
	sp1[0] = 0;
	sp2[0] = 0;

	Con_Printf(" *Loaded Rscript\n");

	do 
	{
		fscanf(f,"%s",ch);
		if (inscript) 
		{
			if (!_stricmp(ch,"turb")) 
			{ // turb effect
				fscanf(f,"%f",&fp1); fscanf(f,"%f",&fp2);
				rscripts[num].turb.power	= fp1;
				rscripts[num].turb.movediv	= fp2;
				rscripts[num].useturb		= true;
			}
			else if (!_stricmp(ch,"turbvert")) 
			{ // vertex turb effect
				fscanf(f,"%f",&fp1); fscanf(f,"%f",&fp2); fscanf(f,"%f",&fp3); fscanf(f,"%f",&fp4);
				rscripts[num].vturb.power	= fp1;
				rscripts[num].usevturb		= true;
			} 
			else if (!_stricmp(ch,"scroll")) 
			{ // scrolling texture
				fscanf(f,"%f",&fp1); fscanf(f,"%f",&fp2);
				rscripts[num].scroll.xspeed = fp1;
				rscripts[num].scroll.yspeed = fp2;
				rscripts[num].usescroll		= true;
			} 
			else if (!_stricmp(ch,"stage")) 
			{ // next stage
				fscanf(f,"%s",sp1);
				strcpy(rscripts[num].nextname, sp1);
			} 
			else if (!_stricmp(ch,"map")) 
			{ // texture
				fscanf(f,"%s",sp1);
				strcpy(rscripts[num].texname, sp1);
				rscripts[num].texexist = true;
			} 
			else if (!_stricmp(ch,"anim")) 
			{ // anim map
				fscanf(f,"%f",&fp1);
				rscripts[num].anim.num = fp1;
				for (i=0; i<fp1; i++)
				{
					if (i > MAX_ANIM_FRAMES-1)
						continue;
					fscanf(f,"%s",sp1);
					strcpy(rscripts[num].anim.name[i].name,sp1);
				}
				rscripts[num].useanim=true;
			} 
			else if (!_stricmp(ch,"set")) 
			{ // set texture flags
				fscanf(f,"%s",sp1); fscanf(f,"%f",&fp1);
				if (!_stricmp(sp1, "alpha")) // alpha amount
					rscripts[num].flags.alpha		= fp1;

				else if (!_stricmp(sp1, "blendfunc")) // use blendfunc?
					rscripts[num].flags.blendfunc	= fp1;

				else if (!_stricmp(sp1, "texscale")) // texture scaling
					rscripts[num].texscale			= fp1;

				else if (!_stricmp(sp1, "envmap")) // environmental mapping?
					rscripts[num].flags.envmap		= fp1;
				else if (!_stricmp(sp1, "animtime")) // animation timing (ms)
					rscripts[num].flags.animtime	= fp1;
			} 
			else if (!_stricmp(ch,"}")) 
			{
				inscript=0;
				num++;
			}	
		} 
		else 
		{
			if (_stricmp(ch,"{"))
			{
				strcpy(rscripts[num].scriptname,ch);
			} 
			else 
			{
				rscripts[num].flags.alpha		= 1;
				rscripts[num].texscale			= 1;
				rscripts[num].flags.animtime	= 1;
				inscript						= 1;
			}
		}
	} while (!feof(f));

	fclose(f);
}
