/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2018-2022 Jesse Allen
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * To compile:
 * gcc -g -I../include icnpack.c -o icnpack
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <COLCODE.h>

//-------------------------------------------------
//
// Format of the compressed data
// compressed	decompressed
// FF		FF
// FE (-2)	FF FF
// FD (-3)	FF FF FF
// FC (-4)	FF FF FF FF
// FB (-5)	FF FF FF FF FF
// FA (-6)	FF FF FF FF FF FF
// F9 (-7)	FF FF FF FF FF FF FF
// F8 B		FF ... <B times>
//
// Header COLCODE.H provides some helpers macros.
//
//-------------------------------------------------
//
// Format of the bitmap data :
//
// <short>  width
// <short>  height
// <char..> bitmap image
//
//-------------------------------------------------


int main(int argc, char **argv)
{
	short w, h;
	unsigned char *pixels;
	FILE *fh;

	if( argc<2 )
	{
		printf("Usage: icnbmp input.icn\n");
		return 1;
	}

	fh = fopen(argv[1], "r");
	if( !fh )
		return 1;
	fread(&w, sizeof(short), 1, fh);
	fread(&h, sizeof(short), 1, fh);

	pixels = malloc(w*h);
	fread(pixels, 1, w*h, fh);
	fclose(fh);

	fh = fopen("OUT.ICN", "w");
	if( !fh )
		return 1;
	fwrite(&w, sizeof(short), 1, fh);
	fwrite(&h, sizeof(short), 1, fh);

	unsigned char run = 0;
	for( int i = 0; i < w*h; i++ )
	{
		if( run && pixels[i] != TRANSPARENT_CODE )
		{
			unsigned char code;
			if( run == 1 )
			{
				code = TRANSPARENT_CODE;
			}
			if( run <= UNIQUE_REPEAT_CODE_NUM )
			{
				code = FEW_TRANSPARENT_CODE(run);
			}
			else
			{
				code = MANY_TRANSPARENT_CODE;
				fwrite(&code, sizeof(unsigned char), 1, fh);
				code = run;
			}
			fwrite(&code, sizeof(unsigned char), 1, fh);
			run = 0;
		}
		if( pixels[i] == TRANSPARENT_CODE )
		{
			unsigned char code;
			if( ++run < TRANSPARENT_CODE )
				continue;
			code = MANY_TRANSPARENT_CODE;
			fwrite(&code, sizeof(unsigned char), 1, fh);
			code = 0xff;
			fwrite(&code, sizeof(unsigned char), 1, fh);
			run = 0;
			continue;
		}
		fwrite(&pixels[i], sizeof(unsigned char), 1, fh);
	}

	fclose(fh);
	return 0;
}
