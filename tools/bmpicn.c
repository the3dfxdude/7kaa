/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2018-2020 Jesse Allen
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
 * gcc -g bmpicn.c -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf -o bmpicn
 *
 */

#include <stdio.h>
#include <SDL.h>

//-------- Define palette file format -------//

#pragma pack(1)
struct PaletteHeader
{
	char data[8];
};
struct PaletteEntry
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
#pragma pack()

int main(int argc, char **argv)
{
	struct PaletteHeader pal;
	SDL_Surface *s;
	FILE *fh;

	if( argc<2 )
	{
		printf("Usage: bmpicn input.bmp\n");
		return 1;
	}

	if( SDL_Init(SDL_INIT_VIDEO) < 0 )
		return 1;

	s = SDL_LoadBMP(argv[1]);
	if( !s )
	{
		printf("could not read the file\n");
		SDL_Quit();
		return 1;
	}

	if( s->format->format != SDL_PIXELFORMAT_INDEX8 )
	{
		printf("not index 8bit format\n");
		SDL_Quit();
		return 1;
	}

	if( s->w != s->pitch )
	{
		printf("surface has a pitch\n");
		SDL_Quit();
		return 1;
	}

	fh = fopen("OUT.ICN", "w");
	if( !fh )
		return 1;
	short w = s->w, h = s->h;
	fwrite(&w, sizeof(short), 1, fh);
	fwrite(&h, sizeof(short), 1, fh);
	fwrite(s->pixels, 1, s->w*s->h, fh);
	fclose(fh);

	fh = fopen("OUT.COL", "w");
	if( !fh )
		return 1;
	fwrite(&pal, sizeof(struct PaletteHeader), 1, fh);
	for( int i = 0; i < s->format->palette->ncolors; i++ )
	{
		fwrite(&s->format->palette->colors[i], 3, 1, fh); // only rgb
	}
	fclose(fh);

	SDL_FreeSurface(s);
	SDL_Quit();

	return 0;
}
