/*
Copyright (C) Brian Paul

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

#include <malloc.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>

#pragma warning(disable : 4244)     // MIPS

// GLU- Replacement-functions : taken from MESA 

static GLint bytes_per_pixel( GLenum format, GLenum type )
{
	GLint n, m;
	
	switch (format) {
	case GL_COLOR_INDEX:
	case GL_STENCIL_INDEX:
	case GL_DEPTH_COMPONENT:
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_LUMINANCE:
		n = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		n = 2;
		break;
	case GL_RGB:
		n = 3;
		break;
	case GL_RGBA:
#ifdef GL_EXT_abgr
	case GL_ABGR_EXT:
#endif
		n = 4;
		break;
	default:
		n = 0;
	}
	
	switch (type) {
	case GL_UNSIGNED_BYTE:	m = sizeof(GLubyte);	break;
	case GL_BYTE:		m = sizeof(GLbyte);	break;
	case GL_BITMAP:		m = 1;			break;
	case GL_UNSIGNED_SHORT:	m = sizeof(GLushort);	break;
	case GL_SHORT:		m = sizeof(GLshort);	break;
	case GL_UNSIGNED_INT:	m = sizeof(GLuint);	break;
	case GL_INT:		m = sizeof(GLint);	break;
	case GL_FLOAT:		m = sizeof(GLfloat);	break;
	default:			m = 0;
	}
	
	return n * m;
}



#define CEILING( A, B )  ( (A) % (B) == 0 ? (A)/(B) : (A)/(B)+1 )

GLint APIENTRY gluScaleImage( GLenum format,
                              GLsizei widthin, GLsizei heightin,
                              GLenum typein, const void *datain,
                              GLsizei widthout, GLsizei heightout,
                              GLenum typeout, void *dataout )
{
	GLint components, i, j, k;
	GLfloat *tempin, *tempout;
	GLfloat sx, sy;
	GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
	GLint packrowlength, packalignment, packskiprows, packskippixels;
	GLint sizein, sizeout;
	GLint rowstride, rowlen;
	
	
	/* Determine number of components per pixel */
	switch (format) {
	case GL_COLOR_INDEX:
	case GL_STENCIL_INDEX:
	case GL_DEPTH_COMPONENT:
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_LUMINANCE:
		components = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		components = 2;
		break;
	case GL_RGB:
		//case GL_BGR:
		components = 3;
		break;
	case GL_RGBA:
		//   case GL_BGRA:
#ifdef GL_EXT_abgr
	case GL_ABGR_EXT:
#endif
		components = 4;
		break;
	default:
		return GL_INVALID_ENUM;
	}
	
	/* Determine bytes per input datum */
	switch (typein) {
	case GL_UNSIGNED_BYTE:	sizein = sizeof(GLubyte);	break;
	case GL_BYTE:		sizein = sizeof(GLbyte);	break;
	case GL_UNSIGNED_SHORT:	sizein = sizeof(GLushort);	break;
	case GL_SHORT:		sizein = sizeof(GLshort);	break;
	case GL_UNSIGNED_INT:	sizein = sizeof(GLuint);	break;
	case GL_INT:		sizein = sizeof(GLint);		break;
	case GL_FLOAT:		sizein = sizeof(GLfloat);	break;
	case GL_BITMAP:
		/* not implemented yet */
	default:
		return GL_INVALID_ENUM;
	}
	
	/* Determine bytes per output datum */
	switch (typeout) {
	case GL_UNSIGNED_BYTE:	sizeout = sizeof(GLubyte);	break;
	case GL_BYTE:		sizeout = sizeof(GLbyte);	break;
	case GL_UNSIGNED_SHORT:	sizeout = sizeof(GLushort);	break;
	case GL_SHORT:		sizeout = sizeof(GLshort);	break;
	case GL_UNSIGNED_INT:	sizeout = sizeof(GLuint);	break;
	case GL_INT:		sizeout = sizeof(GLint);	break;
	case GL_FLOAT:		sizeout = sizeof(GLfloat);	break;
	case GL_BITMAP:
		/* not implemented yet */
	default:
		return GL_INVALID_ENUM;
	}
	
	/* Get glPixelStore state */
	glGetIntegerv( GL_UNPACK_ROW_LENGTH, &unpackrowlength );
	glGetIntegerv( GL_UNPACK_ALIGNMENT, &unpackalignment );
	glGetIntegerv( GL_UNPACK_SKIP_ROWS, &unpackskiprows );
	glGetIntegerv( GL_UNPACK_SKIP_PIXELS, &unpackskippixels );
	glGetIntegerv( GL_PACK_ROW_LENGTH, &packrowlength );
	glGetIntegerv( GL_PACK_ALIGNMENT, &packalignment );
	glGetIntegerv( GL_PACK_SKIP_ROWS, &packskiprows );
	glGetIntegerv( GL_PACK_SKIP_PIXELS, &packskippixels );
	
	/* Allocate storage for intermediate images */
	tempin = (GLfloat *) malloc( widthin * heightin
		* components * sizeof(GLfloat) );
	if (!tempin) {
		return GL_OUT_OF_MEMORY;
	}
	tempout = (GLfloat *) malloc( widthout * heightout
		* components * sizeof(GLfloat) );
	if (!tempout) {
		free( tempin );
		return GL_OUT_OF_MEMORY;
	}
	
	
	/*
    * Unpack the pixel data and convert to floating point
    */
	
	if (unpackrowlength>0) {
		rowlen = unpackrowlength;
	}
	else {
		rowlen = widthin;
	}
	if (sizein >= unpackalignment) {
		rowstride = components * rowlen;
	}
	else {
		rowstride = unpackalignment/sizein
			* CEILING( components * rowlen * sizein, unpackalignment );
	}
	
	switch (typein) {
	case GL_UNSIGNED_BYTE:
		k = 0;
		for (i=0;i<heightin;i++) {
			GLubyte *ubptr = (GLubyte *) datain
				+ i * rowstride
				+ unpackskiprows * rowstride
				+ unpackskippixels * components;
			for (j=0;j<widthin*components;j++) {
				tempin[k++] = (GLfloat) *ubptr++;
			}
		}
		break;
	case GL_BYTE:
		k = 0;
		for (i=0;i<heightin;i++) {
			GLbyte *bptr = (GLbyte *) datain
				+ i * rowstride
				+ unpackskiprows * rowstride
				+ unpackskippixels * components;
			for (j=0;j<widthin*components;j++) {
				tempin[k++] = (GLfloat) *bptr++;
			}
		}
		break;
	case GL_UNSIGNED_SHORT:
		k = 0;
		for (i=0;i<heightin;i++) {
			GLushort *usptr = (GLushort *) datain
				+ i * rowstride
				+ unpackskiprows * rowstride
				+ unpackskippixels * components;
			for (j=0;j<widthin*components;j++) {
				tempin[k++] = (GLfloat) *usptr++;
			}
		}
		break;
	case GL_SHORT:
		k = 0;
		for (i=0;i<heightin;i++) {
			GLshort *sptr = (GLshort *) datain
				+ i * rowstride
				+ unpackskiprows * rowstride
				+ unpackskippixels * components;
			for (j=0;j<widthin*components;j++) {
				tempin[k++] = (GLfloat) *sptr++;
			}
		}
		break;
	case GL_UNSIGNED_INT:
		k = 0;
		for (i=0;i<heightin;i++) {
			GLuint *uiptr = (GLuint *) datain
				+ i * rowstride
				+ unpackskiprows * rowstride
				+ unpackskippixels * components;
			for (j=0;j<widthin*components;j++) {
				tempin[k++] = (GLfloat) *uiptr++;
			}
		}
		break;
	case GL_INT:
		k = 0;
		for (i=0;i<heightin;i++) {
			GLint *iptr = (GLint *) datain
				+ i * rowstride
				+ unpackskiprows * rowstride
				+ unpackskippixels * components;
			for (j=0;j<widthin*components;j++) {
				tempin[k++] = (GLfloat) *iptr++;
			}
		}
		break;
	case GL_FLOAT:
		k = 0;
		for (i=0;i<heightin;i++) {
			GLfloat *fptr = (GLfloat *) datain
				+ i * rowstride
				+ unpackskiprows * rowstride
				+ unpackskippixels * components;
			for (j=0;j<widthin*components;j++) {
				tempin[k++] = *fptr++;
			}
		}
		break;
	default:
		return GL_INVALID_ENUM;
	}
	
	
	/*
    * Scale the image!
    */
	
	if (widthout > 1)
		sx = (GLfloat) (widthin-1) / (GLfloat) (widthout-1);
	else
		sx = (GLfloat) (widthin-1);
	if (heightout > 1)
		sy = (GLfloat) (heightin-1) / (GLfloat) (heightout-1);
	else
		sy = (GLfloat) (heightin-1);
	
	/*#define POINT_SAMPLE*/
#ifdef POINT_SAMPLE
	for (i=0;i<heightout;i++) {
		GLint ii = i * sy;
		for (j=0;j<widthout;j++) {
			GLint jj = j * sx;
			
			GLfloat *src = tempin + (ii * widthin + jj) * components;
			GLfloat *dst = tempout + (i * widthout + j) * components;
			
			for (k=0;k<components;k++) {
				*dst++ = *src++;
			}
		}
	}
#else
	if (sx<1.0 && sy<1.0) {
		/* magnify both width and height:  use weighted sample of 4 pixels */
		GLint i0, i1, j0, j1;
		GLfloat alpha, beta;
		GLfloat *src00, *src01, *src10, *src11;
		GLfloat s1, s2;
		GLfloat *dst;
		
		for (i=0;i<heightout;i++) {
			i0 = i * sy;
			i1 = i0 + 1;
			if (i1 >= heightin) i1 = heightin-1;
			/*	 i1 = (i+1) * sy - EPSILON;*/
			alpha = i*sy - i0;
			for (j=0;j<widthout;j++) {
				j0 = j * sx;
				j1 = j0 + 1;
				if (j1 >= widthin) j1 = widthin-1;
				/*	    j1 = (j+1) * sx - EPSILON; */
				beta = j*sx - j0;
				
				/* compute weighted average of pixels in rect (i0,j0)-(i1,j1) */
				src00 = tempin + (i0 * widthin + j0) * components;
				src01 = tempin + (i0 * widthin + j1) * components;
				src10 = tempin + (i1 * widthin + j0) * components;
				src11 = tempin + (i1 * widthin + j1) * components;
				
				dst = tempout + (i * widthout + j) * components;
				
				for (k=0;k<components;k++) {
					s1 = *src00++ * (1.0-beta) + *src01++ * beta;
					s2 = *src10++ * (1.0-beta) + *src11++ * beta;
					*dst++ = s1 * (1.0-alpha) + s2 * alpha;
				}
			}
		}
	}
	else {
		/* shrink width and/or height:  use an unweighted box filter */
		GLint i0, i1;
		GLint j0, j1;
		GLint ii, jj;
		GLfloat sum, *dst;
		
		for (i=0;i<heightout;i++) {
			i0 = i * sy;
			i1 = i0 + 1;
			if (i1 >= heightin) i1 = heightin-1; 
			/*	 i1 = (i+1) * sy - EPSILON; */
			for (j=0;j<widthout;j++) {
				j0 = j * sx;
				j1 = j0 + 1;
				if (j1 >= widthin) j1 = widthin-1;
				/*	    j1 = (j+1) * sx - EPSILON; */
				
				dst = tempout + (i * widthout + j) * components;
				
				/* compute average of pixels in the rectangle (i0,j0)-(i1,j1) */
				for (k=0;k<components;k++) {
					sum = 0.0;
					for (ii=i0;ii<=i1;ii++) {
						for (jj=j0;jj<=j1;jj++) {
							sum += *(tempin + (ii * widthin + jj) * components + k);
						}
					}
					sum /= (j1-j0+1) * (i1-i0+1);
					*dst++ = sum;
				}
			}
		}
	}
#endif
	
	
	/*
    * Return output image
    */
	
	if (packrowlength>0) {
		rowlen = packrowlength;
	}
	else {
		rowlen = widthout;
	}
	if (sizeout >= packalignment) {
		rowstride = components * rowlen;
	}
	else {
		rowstride = packalignment/sizeout
			* CEILING( components * rowlen * sizeout, packalignment );
	}
	
	switch (typeout) {
	case GL_UNSIGNED_BYTE:
		k = 0;
		for (i=0;i<heightout;i++) {
			GLubyte *ubptr = (GLubyte *) dataout
				+ i * rowstride
				+ packskiprows * rowstride
				+ packskippixels * components;
			for (j=0;j<widthout*components;j++) {
				*ubptr++ = (GLubyte) tempout[k++];
			}
		}
		break;
	case GL_BYTE:
		k = 0;
		for (i=0;i<heightout;i++) {
			GLbyte *bptr = (GLbyte *) dataout
				+ i * rowstride
				+ packskiprows * rowstride
				+ packskippixels * components;
			for (j=0;j<widthout*components;j++) {
				*bptr++ = (GLbyte) tempout[k++];
			}
		}
		break;
	case GL_UNSIGNED_SHORT:
		k = 0;
		for (i=0;i<heightout;i++) {
			GLushort *usptr = (GLushort *) dataout
				+ i * rowstride
				+ packskiprows * rowstride
				+ packskippixels * components;
			for (j=0;j<widthout*components;j++) {
				*usptr++ = (GLushort) tempout[k++];
			}
		}
		break;
	case GL_SHORT:
		k = 0;
		for (i=0;i<heightout;i++) {
			GLshort *sptr = (GLshort *) dataout
				+ i * rowstride
				+ packskiprows * rowstride
				+ packskippixels * components;
			for (j=0;j<widthout*components;j++) {
				*sptr++ = (GLshort) tempout[k++];
			}
		}
		break;
	case GL_UNSIGNED_INT:
		k = 0;
		for (i=0;i<heightout;i++) {
			GLuint *uiptr = (GLuint *) dataout
				+ i * rowstride
				+ packskiprows * rowstride
				+ packskippixels * components;
			for (j=0;j<widthout*components;j++) {
				*uiptr++ = (GLuint) tempout[k++];
			}
		}
		break;
	case GL_INT:
		k = 0;
		for (i=0;i<heightout;i++) {
			GLint *iptr = (GLint *) dataout
				+ i * rowstride
				+ packskiprows * rowstride
				+ packskippixels * components;
			for (j=0;j<widthout*components;j++) {
				*iptr++ = (GLint) tempout[k++];
			}
		}
		break;
	case GL_FLOAT:
		k = 0;
		for (i=0;i<heightout;i++) {
			GLfloat *fptr = (GLfloat *) dataout
				+ i * rowstride
				+ packskiprows * rowstride
				+ packskippixels * components;
			for (j=0;j<widthout*components;j++) {
				*fptr++ = tempout[k++];
			}
		}
		break;
	default:
		return GL_INVALID_ENUM;
	}
	
	
	/* free temporary image storage */
	free( tempin );
	free( tempout );
	
	return 0;
}

int round_2 (int x )
{
	int v=2;

	while (x>v)
		v*=2;
	
	return v;
}

GLint APIENTRY GL_Build2DMipmaps( GLenum target, GLint components,
                                  GLsizei width, GLsizei height, GLenum format,
                                  GLenum type, const void *data )
{
	GLint w, h;
	void *image, *newimage;
	GLint neww, newh, level, bpp;
	int error;
	GLboolean done;
	GLint retval = 0;
	GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
	GLint packrowlength, packalignment, packskiprows, packskippixels;
	static GLint maxsize = 0;
	
	if (width < 1 || height < 1)
		return GL_INVALID_VALUE;
	
	if (!maxsize)
		glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxsize );
	
	if (!maxsize)
		return GL_INVALID_VALUE;
	
	w = round_2( width );
	if (w>maxsize) {
		w = maxsize;
	}
	h = round_2( height );
	if (h>maxsize) {
		h = maxsize;
	}
	
	bpp = bytes_per_pixel( format, type );
	if (bpp==0) {
		/* probably a bad format or type enum */
		return GL_INVALID_ENUM;
	}
	
	/* Get current glPixelStore values */
	glGetIntegerv( GL_UNPACK_ROW_LENGTH, &unpackrowlength );
	glGetIntegerv( GL_UNPACK_ALIGNMENT, &unpackalignment );
	glGetIntegerv( GL_UNPACK_SKIP_ROWS, &unpackskiprows );
	glGetIntegerv( GL_UNPACK_SKIP_PIXELS, &unpackskippixels );
	glGetIntegerv( GL_PACK_ROW_LENGTH, &packrowlength );
	glGetIntegerv( GL_PACK_ALIGNMENT, &packalignment );
	glGetIntegerv( GL_PACK_SKIP_ROWS, &packskiprows );
	glGetIntegerv( GL_PACK_SKIP_PIXELS, &packskippixels );
	
	/* set pixel packing */
	glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glPixelStorei( GL_PACK_SKIP_ROWS, 0 );
	glPixelStorei( GL_PACK_SKIP_PIXELS, 0 );
	
	done = GL_FALSE;
	
	if (w!=width || h!=height) {
		/* must rescale image to get "top" mipmap texture image */
		image = malloc( (w+4) * h * bpp );
		if (!image) {
			return GL_OUT_OF_MEMORY;
		}
		error = gluScaleImage( format, width, height, type, data,
			w, h, type, image );
		if (error) {
			retval = error;
			done = GL_TRUE;
		}
	}
	else {
		image = (void *) data;
	}
	
	level = 0;
	while (!done) {
		if (image != data) {
			/* set pixel unpacking */
			glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
			glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
			glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
			glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
		}
		
		glTexImage2D( target, level, components, w, h, 0, format, type, image );
		
		if (w==1 && h==1)  break;
		
		neww = (w<2) ? 1 : w/2;
		newh = (h<2) ? 1 : h/2;
		newimage = malloc( (neww+4) * newh * bpp );
		if (!newimage) {
			return GL_OUT_OF_MEMORY;
		}
		
		error =  gluScaleImage( format, w, h, type, image,
			neww, newh, type, newimage );
		if (error) {
			retval = error;
			done = GL_TRUE;
		}
		
		if (image!=data) {
			free( image );
		}
		image = newimage;
		
		w = neww;
		h = newh;
		level++;
	}
	
	if (image!=data) {
		free( image );
	}
	
	/* Restore original glPixelStore state */
	glPixelStorei( GL_UNPACK_ROW_LENGTH, unpackrowlength );
	glPixelStorei( GL_UNPACK_ALIGNMENT, unpackalignment );
	glPixelStorei( GL_UNPACK_SKIP_ROWS, unpackskiprows );
	glPixelStorei( GL_UNPACK_SKIP_PIXELS, unpackskippixels );
	glPixelStorei( GL_PACK_ROW_LENGTH, packrowlength );
	glPixelStorei( GL_PACK_ALIGNMENT, packalignment );
	glPixelStorei( GL_PACK_SKIP_ROWS, packskiprows );
	glPixelStorei( GL_PACK_SKIP_PIXELS, packskippixels );
	
	return retval;
}

int gluBuild2DMipmaps (GLenum target, GLint components, 
						GLint width, GLint height, 
						GLenum format, GLenum type, const void *data)
{
	return GL_Build2DMipmaps (target, components, width, height, format, type, (unsigned char *)data);
}
