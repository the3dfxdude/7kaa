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
 * gcc -g genfont.c -I/usr/include/SDL2 -lSDL2 -lSDL2_ttf -o genfont
 *
 * Briefly read through the code below on the font styles, and the fonts
 * that appear to produce the best results. The program works by walking
 * through a 8-bit word range and converting the word from an assumed encoding
 * to utf-8.
 *
 * It would be nice if SDL_ttf was a bit more flexible on using utf-8 for
 * accessing the glyph information, but it seems to be mainly written around
 * ucs-2. Otherwise, it still meets the fundamental use requirements.
 *
 * The exact process of doing this was conceived by examining the original
 * game fonts and information provided by the original game authors. This
 * allows us to recreate a similar looking style, but with freely available
 * type faces.
 *
 * This program was necessary when contributors provided translations that
 * required different characters beyond the original LATIN-1 codeset. If you
 * are looking to generating new fonts for another codeset, you must change
 * the codeset for iconv and experiment with fonts. Previous experience showed
 * that font quality was an issue.
 *
 * Finally note that testing fonts in the game will require providing the
 * necessary gettext modules and locale.res information to operate correctly.
 *
 */

#include <stdio.h>
#include <iconv.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define MAX_GLYPHS 256
#define MAX_UTF8_CHAR 5

#define TRANSPARENT_CODE 255

//-------- Define struct FontHeader -------//

#pragma pack(1)
struct FontHeader
{
	unsigned short max_width;
	unsigned short max_height;
	unsigned short std_height;
	unsigned short first_char;              // ascii code of the first character
	unsigned short last_char;               // ascii code of the last character
};

//-------- Define struct FontInfo -------//

struct FontInfo // info for each character
{
	char          offset_y;
	unsigned char width;
	unsigned char height;
	uint32_t      bitmap_offset;  // file offset relative to bitmap data
};
#pragma pack()

//-------- Define Font Styles -------//

#define COLOR_BLACK {0, 0, 0}
#define COLOR_OFF_BLACK {14, 0, 0}
#define COLOR_OFF_GREY_48 {47, 47, 35}
#define COLOR_GREY_48 {48, 48, 48}
#define COLOR_GREY_80 {80, 80, 80}
#define COLOR_GREY_143 {143, 143, 143}
#define COLOR_GREY_175 {175, 175, 175}
#define COLOR_GREY_207 {207, 207, 207}
#define COLOR_WHITE {255, 251, 247}

SDL_Color white = {255, 255, 255};

#define SHIFT_NONE { 0, 0, 0, 0}
//#define SHIFT_CASA {-1,-1, 2, 2} // if using outline
#define SHIFT_CASA {-1,-1, 0, 0}
#define SHIFT_HITP { 0, 0, 2, 2}
#define SHIFT_NEWS { 1, 1, 1, 1}

SDL_Surface *render_glyph(SDL_Palette *pal, char *c, struct FontInfo *info);
SDL_Surface *render_glyph_blend(SDL_Palette *pal, char *c, struct FontInfo *info);
SDL_Surface *render_glyph_blend_shadow(SDL_Palette *pal, char *c, struct FontInfo *info);
SDL_Surface *render_glyph_outline(SDL_Palette *pal, char *c, struct FontInfo *info);
SDL_Surface *render_glyph_shadow(SDL_Palette *pal, char *c, struct FontInfo *info);

typedef struct _FontStyle
{
	int ptsize;
	int std_height;
	int style;
	int hinting;
	SDL_Color color;
	SDL_Color shadow;
	SDL_Rect shift;
	SDL_Surface *(*render)(SDL_Palette*,char*,struct FontInfo*);
} FontStyle;

/* LiberationSerif-Regular.ttf */
FontStyle casa =
{
	18,
	21,
	TTF_STYLE_NORMAL,
	TTF_HINTING_NORMAL,
	COLOR_BLACK,
	COLOR_GREY_175,
	SHIFT_CASA,
	&render_glyph_blend_shadow
};
	//COLOR_OFF_BLACK,

/* Sans or sans bold (does not need localization) */
/*FontStyle hitp =
{
	12,
	12,
	TTF_STYLE_BOLD,
	TTF_HINTING_NORMAL,
	COLOR_GREY_207,
	COLOR_BLACK,
	SHIFT_HITP,
	&render_glyph_outline
};*/

/* LiberationSerif-Regular.ttf */
FontStyle mid =
{
	14,
	14,
	TTF_STYLE_BOLD,
	TTF_HINTING_NORMAL,
	COLOR_BLACK,
	COLOR_WHITE,
	SHIFT_NEWS,
	&render_glyph_shadow
};

/* LiberationSans-Bold.ttf */
FontStyle news =
{
	12,
	14,
	TTF_STYLE_BOLD,
	TTF_HINTING_NORMAL,
	COLOR_WHITE,
	COLOR_BLACK,
	SHIFT_NEWS,
	&render_glyph_shadow
};

/* LiberationSans-Bold.ttf */
FontStyle san =
{
	12,
	14,
	TTF_STYLE_BOLD,
	TTF_HINTING_NORMAL,
	COLOR_GREY_48,
	COLOR_WHITE,
	SHIFT_NEWS,
	&render_glyph_shadow
};
	//COLOR_OFF_GREY_48,

/* LiberationSerif-Regular.ttf */
FontStyle smal =
{
	13,
	14,
	TTF_STYLE_NORMAL,
	TTF_HINTING_NORMAL,
	COLOR_BLACK,
	COLOR_WHITE,
	SHIFT_NONE,
	&render_glyph
};

/* LiberationSerif-Regular.ttf */
FontStyle std =
{
	16,
	15,
	TTF_STYLE_NORMAL,
	TTF_HINTING_NORMAL,
	COLOR_BLACK,
	COLOR_BLACK,
	SHIFT_NONE,
	&render_glyph
};

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

//--------- Global data ---------//

TTF_Font *font;
SDL_Surface *glyphs[MAX_GLYPHS];
struct FontInfo info[MAX_GLYPHS];
FontStyle *style;

//--------- Begin of function clip_area --------//
//
// Scan frame using rect dimensions. Clip to the smallest required area.
//
// This could be done by accessing glyph dimensions from freetype directly,
// but this code was already available and the iconv code assumed utf-8
// instead of SDL_ttf's TTF_GlyphMetrics "Uint16" (probably ucs-2).
//
static void clip_area(unsigned char *frame, int pitch, SDL_Rect *rect, unsigned char transCode)
{
	int x, y;
	SDL_Point xy1;
	SDL_Point xy2;

	xy1.x = rect->x;
	xy1.y = rect->y;
	xy2.x = rect->x+rect->w-1;
	xy2.y = rect->y+rect->h-1;

	//-------- clip the top border ---------//

	for( y= xy1.y ; y <= xy2.y ; y++ )
	{
		for( x= xy1.x ; x <= xy2.x ; x++ )
		{
			if( frame[y*pitch+x] != transCode )
				break;
		}

		if( x <= xy2.x ) // if non-transparent color encountered in this line
		{
			xy1.y = y;
			break;
		}
	}
	if( y > xy2.y )
	{
		xy1.y = xy2.y;
	}

	//-------- clip the bottom border ---------//

	for( y= xy2.y ; y >= xy1.y ; y-- )
	{
		for( x= xy1.x ; x <= xy2.x ; x++ )
		{
			if( frame[y*pitch+x] != transCode )
				break;
		}

		if( x <= xy2.x ) // if non-transparent color encountered in this line
		{
			xy2.y = y;
			break;
		}
	}
	if( y < xy1.y )
	{
		xy2.y = xy1.y;
	}

	//-------- clip the left border ---------//

	for( x= xy1.x ; x <= xy2.x ; x++ )
	{
		for( y= xy1.y ; y <= xy2.y ; y++ )
		{
			if( frame[y*pitch+x] != transCode )
				break;
		}

		if( y <= xy2.y ) // if non-transparent color encountered in this line
		{
			xy1.x = x;
			break;
		}
	}
	if( x > xy2.x )
	{
		xy1.x = xy2.x;
	}

	//-------- clip the right border ---------//

	for( x= xy2.x ; x >= xy1.x ; x-- )
	{
		for( y= xy1.y ; y <= xy2.y ; y++ )
		{
			if( frame[y*pitch+x] != transCode )
				break;
		}

		if( y <= xy2.y ) // if non-transparent color encountered in this line
		{
			xy2.x = x;
			break;
		}
	}
	if( x < xy1.x )
	{
		xy2.x = xy1.x;
	}

	rect->x = xy1.x;
	rect->y = xy1.y;
	rect->w = xy2.x-xy1.x+1;
	rect->h = xy2.y-xy1.y+1;
}

// Uses iconv to convert char to UTF-8. Returns a complete string in buf that
// is terminated if successful. The conversion descriptor should be a byte
// encoding to UTF-8 method.
int conv_char(iconv_t cd, int cin, char *buf, size_t buf_size)
{
	char c[2];
	size_t buf_left = 2;
	char *p = c;

	c[0] = (unsigned char)cin;
	c[1] = 0;

	return iconv(cd, &p, &buf_left, &buf, &buf_size);
}

// Initializes single FontInfo using clipping dimensions, since this how we
// clipped determines the final icn dimensions and vertical offsetting.
void init_info(struct FontInfo *info, SDL_Rect *clip)
{
	info->offset_y = clip->y;  // offset by how much clipping from top
	info->width = clip->w;
	info->height = clip->h;
}

// Sets bmp offset to *offset, and increments offset by size.
void init_info_bmp_offset(struct FontInfo *info, int *offset)
{
	info->bitmap_offset = *offset;
	*offset += 4;
	*offset += info->width * info->height;
}

int init_font_style(char *filename, char *style_name)
{
	if( strcmp(style_name, "casa") == 0 )
		style = &casa;
//	else if( strcmp(style_name, "hitp") == 0 )
//		style = &hitp;
	else if( strcmp(style_name, "mid") == 0 )
		style = &mid;
	else if( strcmp(style_name, "news") == 0 )
		style = &news;
	else if( strcmp(style_name, "san") == 0 )
		style = &san;
	else if( strcmp(style_name, "smal") == 0 )
		style = &smal;
	else if( strcmp(style_name, "std") == 0 )
		style = &std;
	if( !style )
	{
		printf("can't find style\n");
		return 0;
	}
	font = TTF_OpenFont(filename, style->ptsize);
	if( !font )
	{
		printf("%s\n", TTF_GetError());
		return 0;
	}
	TTF_SetFontStyle(font, style->style);
	TTF_SetFontHinting(font, style->hinting);
	return 1;
}

SDL_Palette *init_pal(char *filename)
{
	struct PaletteHeader h;
	struct PaletteEntry e;
	SDL_Color color;
	SDL_Palette *pal;
	FILE *fh;

	fh = fopen(filename, "r");
	if( !fh )
		return NULL;

	fread(&h, sizeof(struct PaletteHeader), 1, fh);

	pal = SDL_AllocPalette(256);
	for( int i = 0; i < 256; i++ )
	{
		if( !fread(&e, sizeof(struct PaletteEntry), 1, fh) )
		{
			SDL_FreePalette(pal);
			return NULL;
		}
		color.r = e.r;
		color.g = e.g;
		color.b = e.b;
		SDL_SetPaletteColors(pal, &color, i, 1);
	}

	fclose(fh);

	return pal;
}

SDL_Surface *render_glyph(SDL_Palette *pal, char *c, struct FontInfo *info)
{
	SDL_Surface *g_font;
	SDL_Surface *g;
	SDL_Surface *g8;
	SDL_Rect clip;

	// render font pass
	g_font = TTF_RenderUTF8_Solid(font, c, style->color);
	if( !g_font )
		return NULL;

	// down convert
	g = SDL_CreateRGBSurface(0, g_font->w, g_font->h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g, pal);
	memset(g->pixels, TRANSPARENT_CODE, g->pitch*g->h);

	SDL_BlitSurface(g_font, NULL, g, NULL);
	SDL_FreeSurface(g_font);

	// clip down to minimum size
	clip.x = 0;
	clip.y = 0;
	clip.w = g->w;
	clip.h = g->h;
	clip_area((unsigned char *)g->pixels, g->pitch, &clip, TRANSPARENT_CODE);

	g8 = SDL_CreateRGBSurface(0, clip.w, clip.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g8, pal);
	memset(g8->pixels, TRANSPARENT_CODE, g8->pitch*g8->h);
	SDL_BlitSurface(g, &clip, g8, NULL);

	SDL_FreeSurface(g);

	// return final 8-bit glyph
	init_info(info, &clip);
	return g8;
}

// This function assumes to blend black glyphs onto a white background. The
// blending will be in shades of grey.
SDL_Surface *render_glyph_blend(SDL_Palette *pal, char *c, struct FontInfo *info)
{
	SDL_Surface *g_font;
	SDL_Surface *g;
	SDL_Surface *g8;
	SDL_Rect clip;

	// Set the background color as white for blending, which will be transparent
	SDL_SetPaletteColors(pal, &white, TRANSPARENT_CODE, 1);

	// render font pass
	g_font = TTF_RenderUTF8_Blended(font, c, style->color);
	if( !g_font )
		return NULL;

	// down convert
	g = SDL_CreateRGBSurface(0, g_font->w, g_font->h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g, pal);
	memset(g->pixels, TRANSPARENT_CODE, g->pitch*g->h);

	SDL_BlitSurface(g_font, NULL, g, NULL);
	SDL_FreeSurface(g_font);

	// clip down to minimum size
	clip.x = 0;
	clip.y = 0;
	clip.w = g->w;
	clip.h = g->h;
	clip_area((unsigned char *)g->pixels, g->pitch, &clip, TRANSPARENT_CODE);

	g8 = SDL_CreateRGBSurface(0, clip.w, clip.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g8, pal);
	memset(g8->pixels, TRANSPARENT_CODE, g8->pitch*g8->h);
	SDL_BlitSurface(g, &clip, g8, NULL);

	SDL_FreeSurface(g);

	// return final 8-bit glyph
	init_info(info, &clip);
	return g8;
}

// This function assumes to blend black glyphs onto a white background. The
// blending will be in shades of grey. The shadow is also blended, and must be
// based on a grey color, not white.
SDL_Surface *render_glyph_blend_shadow(SDL_Palette *pal, char *c, struct FontInfo *info)
{
	SDL_Surface *g_font;
	SDL_Surface *g_shadow;
	SDL_Surface *g;
	SDL_Surface *g8;
	SDL_Rect clip;

	// Set the background color as white for blending, which will be transparent
	SDL_SetPaletteColors(pal, &white, TRANSPARENT_CODE, 1);

	// render font pass
	g_font = TTF_RenderUTF8_Blended(font, c, style->color);
	if( !g_font )
		return NULL;

	// down convert
	g = SDL_CreateRGBSurface(0, g_font->w+style->shift.w, g_font->h+style->shift.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g, pal);
	memset(g->pixels, TRANSPARENT_CODE, g->pitch*g->h);

	// render shadow pass
	g_shadow = TTF_RenderUTF8_Blended(font, c, style->shadow);

	clip.x = style->shift.x;
	clip.y = style->shift.y;
	clip.w = g->w;
	clip.h = g->h;

	SDL_BlitSurface(g_shadow, NULL, g, &clip);
	SDL_FreeSurface(g_shadow);

	SDL_BlitSurface(g_font, NULL, g, NULL);
	SDL_FreeSurface(g_font);

	// clip down to minimum size
	clip.x = 0;
	clip.y = 0;
	clip.w = g->w;
	clip.h = g->h;
	clip_area((unsigned char *)g->pixels, g->pitch, &clip, TRANSPARENT_CODE);

	g8 = SDL_CreateRGBSurface(0, clip.w, clip.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g8, pal);
	memset(g8->pixels, TRANSPARENT_CODE, g8->pitch*g8->h);
	SDL_BlitSurface(g, &clip, g8, NULL);

	SDL_FreeSurface(g);

	// return final 8-bit glyph
	init_info(info, &clip);
	info->offset_y += abs(style->shift.y); // account shift in render
	return g8;
}

SDL_Surface *render_glyph_outline(SDL_Palette *pal, char *c, struct FontInfo *info)
{
	SDL_Surface *g_font;
	SDL_Surface *g_shadow;
	SDL_Surface *g;
	SDL_Surface *g8;
	SDL_Rect clip;

	// render font pass
	g_font = TTF_RenderUTF8_Solid(font, c, style->color);
	if( !g_font )
		return NULL;

	// down convert
	g = SDL_CreateRGBSurface(0, g_font->w+style->shift.w, g_font->h+style->shift.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g, pal);
	memset(g->pixels, TRANSPARENT_CODE, g->pitch*g->h);

	TTF_SetFontOutline(font, 1);

	// render shadow pass
	g_shadow = TTF_RenderUTF8_Solid(font, c, style->shadow);

	clip.x = style->shift.x;
	clip.y = style->shift.y;
	clip.w = g->w;
	clip.h = g->h;

	SDL_BlitSurface(g_shadow, NULL, g, &clip);
	SDL_FreeSurface(g_shadow);

	TTF_SetFontOutline(font, 0);

	SDL_BlitSurface(g_font, NULL, g, NULL);
	SDL_FreeSurface(g_font);

	// clip down to minimum size
	clip.x = 0;
	clip.y = 0;
	clip.w = g->w;
	clip.h = g->h;
	clip_area((unsigned char *)g->pixels, g->pitch, &clip, TRANSPARENT_CODE);

	g8 = SDL_CreateRGBSurface(0, clip.w, clip.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g8, pal);
	memset(g8->pixels, TRANSPARENT_CODE, g8->pitch*g8->h);
	SDL_BlitSurface(g, &clip, g8, NULL);

	SDL_FreeSurface(g);

	// return final 8-bit glyph
	init_info(info, &clip);
	info->offset_y += abs(style->shift.y); // account shift in render
	return g8;
}

SDL_Surface *render_glyph_shadow(SDL_Palette *pal, char *c, struct FontInfo *info)
{
	SDL_Surface *g_font;
	SDL_Surface *g_shadow;
	SDL_Surface *g;
	SDL_Surface *g8;
	SDL_Rect clip;


	// render font pass
	g_font = TTF_RenderUTF8_Solid(font, c, style->color);
	if( !g_font )
		return NULL;

	// down convert
	g = SDL_CreateRGBSurface(0, g_font->w+style->shift.w, g_font->h+style->shift.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g, pal);
	memset(g->pixels, TRANSPARENT_CODE, g->pitch*g->h);

	// render shadow pass
	g_shadow = TTF_RenderUTF8_Solid(font, c, style->shadow);

	clip.x = style->shift.x;
	clip.y = style->shift.y;
	clip.w = g->w;
	clip.h = g->h;

	SDL_BlitSurface(g_shadow, NULL, g, &clip);
	SDL_FreeSurface(g_shadow);

	SDL_BlitSurface(g_font, NULL, g, NULL);
	SDL_FreeSurface(g_font);

	// clip down to minimum size
	clip.x = 0;
	clip.y = 0;
	clip.w = g->w;
	clip.h = g->h;
	clip_area((unsigned char *)g->pixels, g->pitch, &clip, TRANSPARENT_CODE);

	g8 = SDL_CreateRGBSurface(0, clip.w, clip.h, 8, 0, 0, 0, 0);
	SDL_SetSurfacePalette(g8, pal);
	memset(g8->pixels, TRANSPARENT_CODE, g8->pitch*g8->h);
	SDL_BlitSurface(g, &clip, g8, NULL);

	SDL_FreeSurface(g);

	// return final 8-bit glyph
	init_info(info, &clip);
	info->offset_y += abs(style->shift.y); // account shift in render
	return g8;
}

int main(int argc, char **argv)
{
	iconv_t cd;
	struct FontHeader header;
	SDL_Palette *pal;
	FILE *fh;
	int offset;

	if( argc<5 )
	{
		printf("Usage: genfont font.ttf font.res style codeset\n");
		return 1;
	}

	if( SDL_Init(SDL_INIT_VIDEO) < 0 )
		return 1;

	if( TTF_Init() < 0 )
		return 1;

	if( !init_font_style(argv[1], argv[3]) )
	{
		return 1;
	}

	cd = iconv_open("UTF-8//", argv[4]);
	if( cd == (iconv_t)-1 )
	{
		printf("the requested codeset conversion is not supported on this system\n");
		return 1;
	}

//	pal = init_pal("PAL_STD.RES");
	pal = init_pal("PAL_GREY.RES");
	if( !pal )
	{
		printf("unable to load palette\n");
		return 1;
	}

	header.max_width = 0;
	header.max_height = TTF_FontHeight(font);
	//header.std_height = TTF_FontAscent(font);
	//header.max_height = style->max_height;
	header.std_height = style->std_height; // use original font height -- seems to be needed
	header.first_char = 33;
	header.last_char = MAX_GLYPHS-1;
	offset = 0;
	memset(info, 0, sizeof(struct FontInfo)*MAX_GLYPHS);
	for( int i = header.first_char; i <= header.last_char; i++ )
	{
		char mb[MAX_UTF8_CHAR];
		char filename[20];
		int ret;

		glyphs[i] = NULL;

		ret = conv_char(cd, i, mb, MAX_UTF8_CHAR);
		if( ret < 0 )
		{
			printf("Unable to convert char %d\n", i);
			continue;
		}

		glyphs[i] = (*style->render)(pal, mb, &info[i]);
		if( !glyphs[i] )
		{
			printf("Unable to render char %d\n", i);
			// store 1 pixel bitmap
			glyphs[i] = SDL_CreateRGBSurface(0, 1, 1, 8, 0, 0, 0, 0);
			info[i].offset_y = 0;
			info[i].width = 1;
			info[i].height = 1;
		}
		init_info_bmp_offset(&info[i], &offset);

/*
		sprintf(filename, "%d.bmp", i);
		printf("writing %s for %s\n", filename, mb);
		SDL_SaveBMP(glyphs[i], filename);
*/

		if( header.max_width < glyphs[i]->w )
			header.max_width = glyphs[i]->w;

		// we need to update max_height, just in case the shadow makes
		// the font taller than the TTF system reports
		if( header.max_height < info[i].height + info[i].offset_y )
			header.max_height = info[i].height + info[i].offset_y;
	}
	iconv_close(cd);
	TTF_CloseFont(font);
	TTF_Quit();

	fh = fopen(argv[2], "w");
	if( !fh )
	{
		printf("unable to write font.res\n");
		return 1;
	}
	fwrite(&header, sizeof(struct FontHeader), 1, fh);
	fwrite(&info[header.first_char], sizeof(struct FontInfo), header.last_char - header.first_char + 1, fh);
	for( int i = header.first_char; i <= header.last_char; i++ )
	{
		if( !glyphs[i] )
			continue;
		void *p = glyphs[i]->pixels;
		short w = glyphs[i]->w;
		short h = glyphs[i]->h;
		fwrite(&w, sizeof(short), 1, fh);
		fwrite(&h, sizeof(short), 1, fh);
		for( int j = 0; j < glyphs[i]->h; j++, p += glyphs[i]->pitch)
		{
			fwrite(p, sizeof(unsigned char), glyphs[i]->w, fh);
		}
		SDL_FreeSurface(glyphs[i]);
	}
	fclose(fh);

	SDL_FreePalette(pal);
	SDL_Quit();

	return 0;
}
