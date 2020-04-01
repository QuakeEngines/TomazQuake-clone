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
// gl_texman - this file holds all texture management related functions.
//
#include "quakedef.h"

typedef struct
{
	int				width;
	int				height;
	int				texnum;
	int				bytesperpixel;
	char			identifier[64];
	qboolean		mipmap;
	unsigned short	crc;
} gltexture_t;

typedef struct
{
	char	*name;
	int		minimize;
	int		maximize;
} glmode_t;

typedef struct showlmp_s
{
	qboolean	isactive;
	int			x;
	int			y;
	char		label[32];
	char		pic[128];
} showlmp_t;


#define SHOWLMP_MAXLABELS 256
#define	MAX_GLTEXTURES	2048

showlmp_t showlmp[SHOWLMP_MAXLABELS];
gltexture_t	gltextures[MAX_GLTEXTURES];

int		numgltextures;
int		texels;
int		gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
int		gl_filter_max = GL_LINEAR;
int		lhcsumtable[256];
int		image_width;
int		image_height;

glmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

/*
===============
Draw_TextureMode_f
Change texture mode, see glmode_t above
===============
*/
void Draw_TextureMode_f (void)
{
	int		i;
	gltexture_t	*glt;

	if (Cmd_Argc() == 1)
	{
		for (i=0 ; i< 6 ; i++)
			if (gl_filter_min == modes[i].minimize)
			{
				Con_Printf ("%s\n", modes[i].name);
				return;
			}
		Con_Printf ("current filter is unknown???\n");
		return;
	}

	for (i=0 ; i< 6 ; i++)
	{
		if (!Q_strcasecmp (modes[i].name, Cmd_Argv(1) ) )
			break;
	}
	if (i == 6)
	{
		Con_Printf ("bad filter name\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->mipmap)
		{
			glBindTexture (GL_TEXTURE_2D, glt->texnum);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		}
	}
}

void GL_ResetTextures_f( void )
{
	int				i;
	gltexture_t*	t;
	
	for( i = 0, t = gltextures ; i < numgltextures ; i++, t++ )
	{
		glBindTexture( GL_TEXTURE_2D, t->texnum );
		
		if( t->mipmap )
		{
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
		}
	}
}

/*
===========
GL_Mipmap
===========
*/
void GL_Mipmap (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

void R_ResampleTextureLerpLine (byte *in, byte *out, int inwidth, int outwidth)
{
	int		j, xi, oldx, f, fstep, endx;

	oldx	= 0;
	fstep	= (int) (inwidth * 65536.0f / outwidth);
	endx	= (inwidth - 1);

	for (j = 0,f = 0;j < outwidth;j++, f += fstep)
	{
		xi = (int) f >> 16;
		if (xi != oldx)
		{
			in += (xi - oldx) * 4;
			oldx = xi;
		}
		if (xi < endx)
		{
			int lerp = f & 0xFFFF;
			*out++ = (byte) ((((in[4] - in[0]) * lerp) >> 16) + in[0]);
			*out++ = (byte) ((((in[5] - in[1]) * lerp) >> 16) + in[1]);
			*out++ = (byte) ((((in[6] - in[2]) * lerp) >> 16) + in[2]);
			*out++ = (byte) ((((in[7] - in[3]) * lerp) >> 16) + in[3]);
		}
		else
		{
			*out++ = in[0];
			*out++ = in[1];
			*out++ = in[2];
			*out++ = in[3];
		}
	}
}

/*
================
R_ResampleTexture
================
*/
void R_ResampleTexture (void *indata, int inwidth, int inheight, void *outdata,  int outwidth, int outheight)
{
	int		i, j, yi, oldy, f, fstep, endy = (inheight-1);
	byte	*inrow, *out, *row1, *row2;

	out		= outdata;
	fstep	= (int)(inheight * 65536.0f / outheight);

	row1	= malloc(outwidth * 4);
	row2	= malloc(outwidth * 4);

	inrow	= indata;
	oldy	= 0;

	R_ResampleTextureLerpLine (inrow,				row1, inwidth, outwidth);
	R_ResampleTextureLerpLine (inrow + inwidth * 4, row2, inwidth, outwidth);

	for (i=0, f=0; i<outheight; i++, f+=fstep)
	{
		yi = f >> 16;
		if (yi < endy)
		{
			int lerp = f & 0xFFFF;

			if (yi != oldy)
			{
				inrow = (byte *)indata + inwidth * 4 * yi;
				if (yi == oldy+1)
				{
					memcpy(row1, row2, outwidth * 4);
				}
				else
				{
					R_ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
				}

				R_ResampleTextureLerpLine (inrow + inwidth*4, row2, inwidth, outwidth);
				oldy = yi;
			}

			j = outwidth - 4;

			while(j >= 0)
			{
				out[ 0] = (byte) ((((row2[ 0] - row1[ 0]) * lerp) >> 16) + row1[ 0]);
				out[ 1] = (byte) ((((row2[ 1] - row1[ 1]) * lerp) >> 16) + row1[ 1]);
				out[ 2] = (byte) ((((row2[ 2] - row1[ 2]) * lerp) >> 16) + row1[ 2]);
				out[ 3] = (byte) ((((row2[ 3] - row1[ 3]) * lerp) >> 16) + row1[ 3]);
				out[ 4] = (byte) ((((row2[ 4] - row1[ 4]) * lerp) >> 16) + row1[ 4]);
				out[ 5] = (byte) ((((row2[ 5] - row1[ 5]) * lerp) >> 16) + row1[ 5]);
				out[ 6] = (byte) ((((row2[ 6] - row1[ 6]) * lerp) >> 16) + row1[ 6]);
				out[ 7] = (byte) ((((row2[ 7] - row1[ 7]) * lerp) >> 16) + row1[ 7]);
				out[ 8] = (byte) ((((row2[ 8] - row1[ 8]) * lerp) >> 16) + row1[ 8]);
				out[ 9] = (byte) ((((row2[ 9] - row1[ 9]) * lerp) >> 16) + row1[ 9]);
				out[10] = (byte) ((((row2[10] - row1[10]) * lerp) >> 16) + row1[10]);
				out[11] = (byte) ((((row2[11] - row1[11]) * lerp) >> 16) + row1[11]);
				out[12] = (byte) ((((row2[12] - row1[12]) * lerp) >> 16) + row1[12]);
				out[13] = (byte) ((((row2[13] - row1[13]) * lerp) >> 16) + row1[13]);
				out[14] = (byte) ((((row2[14] - row1[14]) * lerp) >> 16) + row1[14]);
				out[15] = (byte) ((((row2[15] - row1[15]) * lerp) >> 16) + row1[15]);
				out		+= 16;
				row1	+= 16;
				row2	+= 16;
				j		-= 4;
			}
			if (j & 2)
			{
				out[ 0] = (byte) ((((row2[ 0] - row1[ 0]) * lerp) >> 16) + row1[ 0]);
				out[ 1] = (byte) ((((row2[ 1] - row1[ 1]) * lerp) >> 16) + row1[ 1]);
				out[ 2] = (byte) ((((row2[ 2] - row1[ 2]) * lerp) >> 16) + row1[ 2]);
				out[ 3] = (byte) ((((row2[ 3] - row1[ 3]) * lerp) >> 16) + row1[ 3]);
				out[ 4] = (byte) ((((row2[ 4] - row1[ 4]) * lerp) >> 16) + row1[ 4]);
				out[ 5] = (byte) ((((row2[ 5] - row1[ 5]) * lerp) >> 16) + row1[ 5]);
				out[ 6] = (byte) ((((row2[ 6] - row1[ 6]) * lerp) >> 16) + row1[ 6]);
				out[ 7] = (byte) ((((row2[ 7] - row1[ 7]) * lerp) >> 16) + row1[ 7]);
				out		+= 8;
				row1	+= 8;
				row2	+= 8;
			}
			if (j & 1)
			{
				out[ 0] = (byte) ((((row2[ 0] - row1[ 0]) * lerp) >> 16) + row1[ 0]);
				out[ 1] = (byte) ((((row2[ 1] - row1[ 1]) * lerp) >> 16) + row1[ 1]);
				out[ 2] = (byte) ((((row2[ 2] - row1[ 2]) * lerp) >> 16) + row1[ 2]);
				out[ 3] = (byte) ((((row2[ 3] - row1[ 3]) * lerp) >> 16) + row1[ 3]);
				out		+= 4;
				row1	+= 4;
				row2	+= 4;
			}
			row1 -= outwidth * 4;
			row2 -= outwidth * 4;
		}
		else
		{
			if (yi != oldy)
			{
				inrow = (byte *)indata + inwidth * 4 * yi;
				
				if (yi == oldy+1)
				{
					memcpy(row1, row2, outwidth * 4);
				}
				else
				{
					R_ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
				}
				oldy = yi;
			}
			memcpy(out, row1, outwidth * 4);
		}
	}
	free(row1);
	free(row2);
}

/*
================
GL_UploadMipmaps
================
*/
void GL_UploadMipmaps (unsigned *data, int width, int height, qboolean alpha)
{
	static unsigned	scaled[1024 * 1024 * 4];
	int				scaled_width, scaled_height, type, miplevel;

	type		= alpha ? gl_alpha_format : gl_solid_format;
	miplevel	= 0;

	for (scaled_width  = 2; scaled_width  < width;  scaled_width  <<= 1);
	for (scaled_height = 2; scaled_height < height; scaled_height <<= 1);

	if (scaled_width  > gl_max_size.value)
		scaled_width  = gl_max_size.value;
	if (scaled_height > gl_max_size.value)
		scaled_height = gl_max_size.value;

	if (scaled_width  > 1024)
		scaled_width  = 1024;
	if (scaled_height > 1024)
		scaled_height = 1024;

	if (scaled_width != width || scaled_height != height)
	{
		R_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);
	}
	else
	{
		memcpy (scaled, data, width * height * 4);
	}

	glTexImage2D (GL_TEXTURE_2D, 0, type, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);

	while (scaled_width > 1 || scaled_height > 1)
	{
		GL_Mipmap ((byte *)scaled, scaled_width, scaled_height);

		scaled_width  >>= 1;
		scaled_height >>= 1;

		if (scaled_width  < 1)
			scaled_width  = 1;
		if (scaled_height < 1)
			scaled_height = 1;

		miplevel++;

		glTexImage2D (GL_TEXTURE_2D, miplevel, type, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}
	
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
}

/*
=========
GL_Upload
=========
*/
void GL_Upload (unsigned *data, int width, int height, qboolean alpha)
{
	static unsigned	scaled[1024 * 1024 * 4];
	int				scaled_width, scaled_height, type;

	type = alpha ? gl_alpha_format : gl_solid_format;

	for (scaled_width  = 2; scaled_width  < width;  scaled_width  <<= 1);
	for (scaled_height = 2; scaled_height < height; scaled_height <<= 1);

	if (scaled_width  > gl_max_size.value)
		scaled_width  = gl_max_size.value;
	if (scaled_height > gl_max_size.value)
		scaled_height = gl_max_size.value;

	if (scaled_width  > 1024)
		scaled_width  = 1024;
	if (scaled_height > 1024)
		scaled_height = 1024;

	if (scaled_width != width || scaled_height != height)
	{
		R_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);

		glTexImage2D	(GL_TEXTURE_2D, 0, type, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		
		glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

		return;
	}
	
	glTexImage2D	(GL_TEXTURE_2D, 0, type, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
}

/*
==========
GL_Upload8
==========
*/
void GL_Upload8 (byte *data, int width, int height, qboolean mipmap, qboolean alpha)
{
	static unsigned int	trans[0x40000];
	int					s = width * height;
	int					i;
	qboolean			noalpha = true;

	for( i = 0 ; i < s && i < 0x40000 ; ++i )
	{
		trans[i]	= d_8to24table[data[i]];

		if( data[i] == 255 )
		{
			noalpha	= false;
		}
	}

	if(!mipmap)
	{
		GL_Upload (trans, width, height, (alpha && !noalpha));
		return;
	}

	GL_UploadMipmaps (trans, width, height, (alpha && !noalpha));
}

int argh, argh2;

/*
==============
GL_LoadTexture
==============
*/
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int bytesperpixel)
{
	int				i;
	gltexture_t		*glt;
	unsigned short	crc;

	if (!identifier[0])
	{
		Con_Printf("GL_LoadTexture: no identifier\n");
		sprintf (identifier, "%s_%i", "argh", argh);
		argh++;
	}

	crc = CRC_Block(data, width*height*bytesperpixel);

	for (i=0, glt=gltextures ; i < numgltextures ; i++, glt++)
	{
		if (!strcmp (identifier, glt->identifier))
		{
			if (crc != glt->crc || width != glt->width || height != glt->height)
			{
				Con_DPrintf("GL_LoadTexture: cache mismatch\n");
				sprintf (identifier, "%s_%i", identifier, argh2);
				argh2++;
				goto GL_LoadTexture_setup;
			}
			return glt->texnum;
		}
	}

GL_LoadTexture_setup:

	glt = &gltextures[numgltextures];
	numgltextures++;

	strcpy (glt->identifier, identifier);
	glt->texnum = texture_extension_number;
	texture_extension_number++;

	glt->crc			= crc;
	glt->width			= width;
	glt->height			= height;
	glt->mipmap			= mipmap;
	glt->bytesperpixel	= bytesperpixel;

	if (!isDedicated)
	{
		glBindTexture (GL_TEXTURE_2D, glt->texnum);
		
		if (bytesperpixel == 1)
		{
			GL_Upload8 (data, width, height, mipmap, alpha);
		}
		else if (bytesperpixel == 4)
		{
			if (mipmap)
			{
				GL_UploadMipmaps ((void *)data, width, height, alpha);
			}
			else
			{
				GL_Upload ((void *)data, width, height, alpha);
			}
		}
		else
		{
			Sys_Error("GL_LoadTexture: unknown bytesperpixel\n");
		}
	}

	return glt->texnum;
}

/****************************************/

// Tomaz - TGA Begin

/*
===========
PCX Loading
===========
*/

typedef struct
{
	char		manufacturer;
	char		version;
	char		encoding;
	char		bits_per_pixel;
	unsigned short	xmin,ymin,xmax,ymax;
	unsigned short	hres,vres;
	unsigned char	palette[48];
	char		reserved;
	char		color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;
	char		filler[58];
	unsigned 	data;			// unbounded
} pcx_t;

/*
=======
LoadPCX
=======
*/
byte* LoadPCX (FILE *f, char *name)
{
	pcx_t	*pcx, pcxbuf;
	byte	palette[768];
	byte	*pix, *image_rgba;
	int		x, y;
	int		dataByte, runLength;
	int		count;

//
// parse the PCX file
//
	fread (&pcxbuf, 1, sizeof(pcxbuf), f);

	pcx = &pcxbuf;

	if (pcx->manufacturer	!= 0x0a	|| 
		pcx->version		!= 5	|| 
		pcx->encoding		!= 1	|| 
		pcx->bits_per_pixel != 8	||
		// Allow up to 1024X1024 pcx textures, i don't know what negative side effects
		// this will, or can have, but it works. 3dfx users will have to go without it
		// though.
		pcx->xmax			> 1024	||	// was 320 
		pcx->ymax			> 1024)	// was 256
	{
		Con_Printf ("%s Bad pcx file\n", name);
		return NULL;
	}

	// seek to palette
	fseek (f, -768, SEEK_END);
	fread (palette, 1, 768, f);

	fseek (f, sizeof(pcxbuf) - 4, SEEK_SET);

	count = (pcx->xmax+1) * (pcx->ymax+1);
	image_rgba = malloc( count * 4);

	for (y=0 ; y<=pcx->ymax ; y++)
	{
		pix = image_rgba + 4*y*(pcx->xmax+1);
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = fgetc(f);

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = fgetc(f);
			}
			else
				runLength = 1;

			while(runLength-- > 0)
			{
				pix[0] = palette[dataByte*3];
				pix[1] = palette[dataByte*3+1];
				pix[2] = palette[dataByte*3+2];
				pix[3] = 255;
				pix += 4;
				x++;
			}
		}
	}
	fclose(f);
	image_width = pcx->xmax+1;
	image_height = pcx->ymax+1;
	return image_rgba;
}

/*
=============
TARGA LOADING
=============
*/

typedef struct _TargaHeader 
{
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


TargaHeader		targa_header;

int fgetLittleShort (FILE *f)
{
	byte	b1, b2;

	b1 = fgetc(f);
	b2 = fgetc(f);

	return (short)(b1 + b2*256);
}

int fgetLittleLong (FILE *f)
{
	byte	b1, b2, b3, b4;

	b1 = fgetc(f);
	b2 = fgetc(f);
	b3 = fgetc(f);
	b4 = fgetc(f);

	return b1 + (b2<<8) + (b3<<16) + (b4<<24);
}

/*
=======
LoadTGA
=======
*/
byte* LoadTGA (FILE *fin, char *name)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*image_rgba;

	targa_header.id_length			= fgetc(fin);
	targa_header.colormap_type		= fgetc(fin);
	targa_header.image_type			= fgetc(fin);
	
	targa_header.colormap_index		= fgetLittleShort(fin);
	targa_header.colormap_length	= fgetLittleShort(fin);
	targa_header.colormap_size		= fgetc(fin);
	targa_header.x_origin			= fgetLittleShort(fin);
	targa_header.y_origin			= fgetLittleShort(fin);
	targa_header.width				= fgetLittleShort(fin);
	targa_header.height				= fgetLittleShort(fin);

	targa_header.pixel_size			= fgetc(fin);
	targa_header.attributes			= fgetc(fin);

	if (targa_header.image_type!=2 && targa_header.image_type!=10) 
		Host_Error ("LoadTGA: %s Only type 2 and 10 targa RGB images supported\n", name);

	if (targa_header.colormap_type !=0 || (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		Host_Error ("LoadTGA: %s Only 32 or 24 bit images supported (no colormaps)\n", name);

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	image_rgba = malloc (numPixels*4);
	
	if (targa_header.id_length != 0)
		fseek(fin, targa_header.id_length, SEEK_CUR);  // skip TARGA image comment
	
	if (targa_header.image_type==2) 
	{  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = image_rgba + row*columns*4;
			for(column=0; column<columns; column++) 
			{
				unsigned char red = 0,green = 0,blue = 0,alphabyte = 0;
				switch (targa_header.pixel_size) 
				{
					case 24:
							
							blue		= getc(fin);
							green		= getc(fin);
							red			= getc(fin);
							pixbuf[0]	= red;
							pixbuf[1]	= green;
							pixbuf[2]	= blue;
							pixbuf[3]	= 255;
							pixbuf		+= 4;
							break;
					case 32:
							blue		= getc(fin);
							green		= getc(fin);
							red			= getc(fin);
							alphabyte	= getc(fin);
							pixbuf[0]	= red;
							pixbuf[1]	= green;
							pixbuf[2]	= blue;
							pixbuf[3]	= alphabyte;
							pixbuf		+= 4;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) 
	{   // Runlength encoded RGB images
		unsigned char red = 0,green = 0,blue = 0,alphabyte = 0,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = image_rgba + row*columns*4;
			for(column=0; column<columns; ) 
			{
				packetHeader=getc(fin);
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) 
				{        // run-length packet
					switch (targa_header.pixel_size) 
					{
						case 24:
								blue		= getc(fin);
								green		= getc(fin);
								red			= getc(fin);
								alphabyte	= 255;
								break;
						case 32:
								blue		= getc(fin);
								green		= getc(fin);
								red			= getc(fin);
								alphabyte	= getc(fin);
								break;
					}
					for(j=0;j<packetSize;j++) 
					{
						pixbuf[0]	= red;
						pixbuf[1]	= green;
						pixbuf[2]	= blue;
						pixbuf[3]	= alphabyte;
						pixbuf		+= 4;
						column++;
						if (column==columns) 
						{ // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = image_rgba + row*columns*4;
						}
					}
				}
				else 
				{	// non run-length packet
					for(j=0;j<packetSize;j++) 
					{
						switch (targa_header.pixel_size)
						{
							case 24:
									blue		= getc(fin);
									green		= getc(fin);
									red			= getc(fin);
									pixbuf[0]	= red;
									pixbuf[1]	= green;
									pixbuf[2]	= blue;
									pixbuf[3]	= 255;
									pixbuf		+= 4;
									break;
							case 32:
									blue		= getc(fin);
									green		= getc(fin);
									red			= getc(fin);
									alphabyte	= getc(fin);
									pixbuf[0]	= red;
									pixbuf[1]	= green;
									pixbuf[2]	= blue;
									pixbuf[3]	= alphabyte;
									pixbuf		+= 4;
									break;
						}
						column++;
						if (column==columns) 
						{ // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = image_rgba + row*columns*4;
						}
					}
				}
			}
			breakOut:;
		}
	}
	
	fclose(fin);
	image_width = columns;
	image_height = rows;
	return image_rgba;
}

byte* loadimagepixels (char* filename, qboolean complain)
{
	FILE	*f;
	char	basename[128], name[128];
	byte	*c;
	COM_StripExtension(filename, basename); // strip the extension to allow TGA and PCX

	for (c = basename;*c;c++)
		if (*c == '*')
			*c = '#';

	sprintf (name, "textures/%s.tga", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadTGA (f, name);

	sprintf (name, "textures/%s.pcx", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadPCX (f, name);

	sprintf (name, "%s.tga", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadTGA (f, name);

	sprintf (name, "%s.pcx", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadPCX (f, name);

	if (complain)
		Con_Printf ("Couldn't load %s.tga or .pcx\n", filename);
	return NULL;
}

int loadtextureimage (char* filename, qboolean complain, qboolean mipmap)
{
	int			j, texnum;
	byte		*data;
	qboolean	transparent;

	if (isDedicated)
		return 0;

	transparent	= false;

	data = loadimagepixels (filename, complain);

	if(!data)
		return 0;

	for (j = 0;j < image_width*image_height;j++)
	{
		if (data[j*4+3] < 255)
		{
			transparent = true;
			break;
		}
	}

	texnum = GL_LoadTexture (filename, image_width, image_height, data, mipmap, transparent, 4);

	free(data);

	return texnum;
}

int loadtextureimage2 (char* filename, qboolean complain, qboolean mipmap)
{
	int			j, texnum, i;
	byte		*data;
	qboolean	transparent;

	transparent	= false;

	data = loadimagepixels (filename, complain);

	if(!data)
		return 0;

	for (j = 0;j < image_width*image_height;j++)
	{
		if (data[j*4+3] < 255)
		{
			transparent = true;
			break;
		}
	}

	for (j = 0;j < 32;j++)
	{
		for (i = 0;i < 32;i++)
		{
			if (i == 0 && j == 0)
				Con_Printf("%s\n", filename);

			if (i == 0)
				Con_Printf("{");

			if (i == 31)
				Con_Printf("%3i},\n", data[(j*32+i)*4+3]);
			else
				Con_Printf("%3i,", data[(j*32+i)*4+3]);
		}
	}

	texnum = GL_LoadTexture (filename, image_width, image_height, data, mipmap, transparent, 4);

	free(data);

	return texnum;
}

int loadtextureimage3 (char* filename, qboolean complain, qboolean mipmap, byte *data)
{
	int			j, texnum;
	qboolean	transparent;

	if (isDedicated)
		return 0;

	transparent	= false;

	data = loadimagepixels (filename, complain);

	if(!data)
		return 0;

	for (j = 0;j < image_width*image_height;j++)
	{
		if (data[j*4+3] < 255)
		{
			transparent = true;
			break;
		}
	}

	texnum = GL_LoadTexture (filename, image_width, image_height, data, mipmap, transparent, 4);

	return texnum;
}

// Tomaz - TGA End

// Tomaz - ShowLMP Begin
void HIDELMP()
{
	int		i;
	byte	*lmplabel;

	lmplabel = MSG_ReadString();
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
	{
		if (showlmp[i].isactive && strcmp(showlmp[i].label, lmplabel) == 0)
		{
			showlmp[i].isactive = false;
			return;
		}
	}
}

void SHOWLMP()
{
	int i, k;
	byte lmplabel[256], picname[256];
	float x, y;
	strcpy(lmplabel,MSG_ReadString());
	strcpy(picname, MSG_ReadString());
	x = MSG_ReadShort();
	y = MSG_ReadShort();
	k = -1;
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
	{
		if (showlmp[i].isactive)
		{
			if (strcmp(showlmp[i].label, lmplabel) == 0)
			{
				k = i;
				break; // drop out to replace it
			}
		}
		else if (k < 0) // find first empty one to replace
			k = i;
	}

	if (k < 0)
		return;

	// change existing one
	showlmp[k].isactive = true;
	strcpy(showlmp[k].label, lmplabel);
	strcpy(showlmp[k].pic, picname);
	showlmp[k].x = x;
	showlmp[k].y = y;
}

void SHOWLMP_drawall()
{
	int i;
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
		if (showlmp[i].isactive)
			Draw_Pic(showlmp[i].x, showlmp[i].y, Draw_CachePic(showlmp[i].pic));
}

void SHOWLMP_clear()
{
	int i;
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
		showlmp[i].isactive = false;
}
// Tomaz - ShowLMP End