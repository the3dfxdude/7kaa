/*
 * Seven Kingdoms: Ancient Adversaries
 *
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

// Filename    : OCOLTBL.CPP
// Description : generated color remap table

#include <OCOLTBL.h>
#include <ALL.h>
#include <math.h>

// ---------- define const -----------//

// value of full intensity, 255 for 24-bit color, 64 for 18-bit color
#define MAX_COLOUR 255
#define PI 3.14159265359L
#define NEAREST_COLOR 8

BYTE ColorTable::identity_table[MAX_COLOUR_TABLE_SIZE] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

// --------- define inline function ---------//

inline int sq(int a)
{
	return a*a;
}


// ---------- begin of function ColorTable::ColorTable ----------//
ColorTable::ColorTable()
{
	remap_table = NULL;
	remap_table_array = NULL;
}


ColorTable::ColorTable(int absScale, int tableSize, BYTE *customTable)
{
	remap_table = NULL;
	remap_table_array = NULL;
	init(absScale, tableSize, customTable);
}
// ---------- end of function ColorTable::ColorTable ----------//


// ---------- begin of function ColorTable::ColorTable ----------//
ColorTable::ColorTable(const ColorTable& ct) : abs_scale(ct.abs_scale),
	table_size(ct.table_size)
{
	if( ct.remap_table )
	{
		remap_table = (BYTE *)mem_add(table_size * (2*abs_scale+1) );
		memcpy(remap_table, ct.remap_table, table_size * (2*abs_scale+1) );
		remap_table_array = (BYTE **)mem_add(sizeof(BYTE *) * (2*abs_scale+1) );
		create_table_array();
	}
	else
	{
		remap_table = NULL;
		remap_table_array = NULL;
	}
}
// ---------- end of function ColorTable::ColorTable ----------//


// ---------- begin of function ColorTable::~ColorTable ----------//
ColorTable::~ColorTable()
{
	deinit();
}
// ---------- end of function ColorTable::~ColorTable ----------//


// ---------- begin of function ColorTable::init ----------//
void ColorTable::init()
{
	deinit();
	abs_scale = 0;
}


// initialize a custom table, given the no. of absolute scale and table size
// the customTable array is (2*absScale+1) groups 
// and each group has (tableSize) bytes of remapping entries
void ColorTable::init(int absScale, int tableSize, BYTE *customTable)
{
	deinit();

	abs_scale = absScale;
	table_size = table_size;
	remap_table = (BYTE *)mem_add(table_size * (2*absScale+1) );
	memcpy(remap_table, customTable, tableSize * (2*absScale+1) );
	remap_table_array = (BYTE **)mem_add(sizeof(BYTE *) * (2*absScale+1) );
	create_table_array();
}
// ---------- end of function ColorTable::init ----------//


// ---------- begin of function ColorTable::deinit ----------//
void ColorTable::deinit()
{
	if( remap_table )
	{
		mem_del( remap_table );
		remap_table = NULL;
	}
	if( remap_table_array)
	{
		mem_del( remap_table_array);
		remap_table_array = NULL;
	}
}
// ---------- end of function ColorTable::deinit ----------//


// ---------- begin of function ColorTable::operator= ----------//
ColorTable& ColorTable::operator=(const ColorTable& ct)
{
	deinit();

	abs_scale = ct.abs_scale;
	table_size= ct.table_size;

	if( ct.remap_table )
	{
		remap_table = (BYTE *)mem_add(table_size * (2*abs_scale+1) );
		memcpy(remap_table, ct.remap_table, table_size * (2*abs_scale+1) );
		remap_table_array = (BYTE **)mem_add(sizeof(BYTE *) * (2*abs_scale+1) );
		create_table_array();
	}
	else
	{
		remap_table = NULL;
		remap_table_array = NULL;
	}

	return *this;
}
// ---------- begin of function ColorTable::operator= ----------//


// ---------- begin of function ColorTable::generate_table ----------//
//
// generate +absScale to -absScale (total 2*absScale +1 remap table )
// from palette pal (768 byte)
// any color in the reservedColor is unmodified
// note reservedColor array must be in accending order
//
// <int> absScale              number of scale to full white/full black
// <BYTE *>pal                 input palette, size must be 3*table_size
// <int>palSize                size of palette entry
// <BYTE *>reservedColor      array of reserved color, 
// <int> reservedCount         size of reservedColor
// reserved color will map to itself and will not be mapped except by itself
//
void ColorTable::generate_table(int absScale, PalDesc & palD, RGBColor (*fp)(RGBColor, int, int))
{
	int palSize = palD.pal_size;

	err_when(absScale < 0 || palD.reserved_count < 0 || palD.pal == NULL);
	err_when(palSize > MAX_COLOUR_TABLE_SIZE);
	deinit();

	abs_scale = absScale;
	table_size = palSize;
	BYTE *remapEntry = remap_table = (BYTE *)mem_add(table_size * (2*absScale+1) );
	remap_table_array = (BYTE **)mem_add(sizeof(BYTE *) * (2*absScale+1) );

	int scale;

	// ------- generate negative scale ----------//
	for( scale = -absScale; scale < 0; ++scale)
	{
		int reservedIndex = 0;
		for( int c=0; c < palSize; ++c, ++remapEntry)
		{
			*remapEntry = c;
			// ------ see if it is a reserved color --------//
			if( palD.is_reserved(c, reservedIndex) )
				continue;

			RGBColor rgb = (*fp)(palD.get_rgb(c), scale, absScale);

			// ------- scan the closet color, except the reserved color
			int cc, dist[NEAREST_COLOR], thisDiff;
			BYTE closeColor[NEAREST_COLOR]; // [0] is the closest

			for( cc = 0; cc < NEAREST_COLOR; ++cc )
			{
				closeColor[cc] = c;
				dist[cc] = 3*0xff*0xff+1;
			}
			int dReservedIndex = 0;
			int d;
			for( d=0; d < palSize; ++d)
			{
				// ------- skip scanning reserved color ------//
				if( palD.is_reserved(d, dReservedIndex) )
					continue;

				// ------- compare the sqaure distance ----------//
				thisDiff = color_dist(rgb, palD.get_rgb(d));
				if( thisDiff < dist[NEAREST_COLOR-1])
				{
					BYTE d1 = (BYTE) d; 
					for( cc = 0; cc < NEAREST_COLOR; ++cc )
					{
						if( thisDiff < dist[cc] )
						{
							// swap thisDiff and dist[cc]
							// so that the replaced result will be shifted to next
							int tempd;
							BYTE tempc;
							tempd = dist[cc];
							dist[cc] = thisDiff;
							thisDiff = tempd;

							tempc = closeColor[cc];
							closeColor[cc] = d1;
							d1 = tempc;
						}
					}
				}
			}

			// closeColor[] are the closest 8 colours, use hsv comparison to find the nearest
			d = closeColor[0];
			*remapEntry = d;
			int minDiff = color_dist_hsv(rgb, palD.get_rgb(d));
			for( cc = 1; cc < NEAREST_COLOR; ++cc)
			{
				d = closeColor[cc];
				thisDiff = color_dist_hsv(rgb, palD.get_rgb(d));
				if( thisDiff < minDiff )
				{
					minDiff = thisDiff;
					*remapEntry = d;
				}
			}
		}
	}

	err_when( remapEntry - remap_table != table_size * abs_scale);

	// scale == 0
	memcpy( remapEntry, identity_table, palSize);
	remapEntry += table_size;

	// ------- generate positive scale ----------//
	for( scale = 1; scale <= absScale; ++scale)
	{
		int reservedIndex = 0;
		for( int c=0; c < palSize; ++c, ++remapEntry)
		{
			*remapEntry = c;
			// ------ see if it is a reserved color --------//
			if( palD.is_reserved(c, reservedIndex) )
				continue;

			RGBColor rgb = (*fp)(palD.get_rgb(c), scale, absScale);
			
			// ------- scan the closet color, except the reserved color
			int cc, dist[NEAREST_COLOR], thisDiff;
			BYTE closeColor[NEAREST_COLOR]; // [0] is the closest

			for( cc = 0; cc < NEAREST_COLOR; ++cc )
			{
				closeColor[cc] = c;
				dist[cc] = 3*0xff*0xff+1;
			}
			int dReservedIndex = 0;
			int d;
			for( d=0; d < palSize; ++d)
			{
				// ------- skip scanning reserved color ------//
				if( palD.is_reserved(d, dReservedIndex) )
					continue;

				// ------- compare the sqaure distance ----------//
				thisDiff = color_dist(rgb, palD.get_rgb(d));

				if( thisDiff < dist[NEAREST_COLOR-1])
				{
					BYTE d1 = (BYTE) d; 
					for( cc = 0; cc < NEAREST_COLOR; ++cc )
					{
						if( thisDiff < dist[cc] )
						{
							// swap thisDiff and dist[cc]
							// so that the replaced result will be shifted to next
							int tempd;
							BYTE tempc;
							tempd = dist[cc];
							dist[cc] = thisDiff;
							thisDiff = tempd;

							tempc = closeColor[cc];
							closeColor[cc] = d1;
							d1 = tempc;
						}
					}
				}
			}

			// closeColor[] are the closest 8 colours, use hsv comparison to find the nearest
			d = closeColor[0];
			*remapEntry = d;
			int minDiff = color_dist_hsv(rgb, palD.get_rgb(d));
			for( cc = 1; cc < NEAREST_COLOR; ++cc)
			{
				d = closeColor[cc];
				thisDiff = color_dist_hsv(rgb, palD.get_rgb(d));
				if( thisDiff < minDiff )
				{
					minDiff = thisDiff;
					*remapEntry = d;
				}
			}
		}
	}

	create_table_array();
}
// ---------- end of function ColorTable::generate_table ----------//


// ---------- begin of function ColorTable::generate_table_fast ----------//
//  simplified version, it ignores reserved colors
void ColorTable::generate_table_fast (int absScale, PalDesc &palD, RGBColor (*fp)(RGBColor, int, int))
{
	int palSize = palD.pal_size;

	err_when(absScale < 0 || palD.pal == NULL);
	err_when(palSize > MAX_COLOUR_TABLE_SIZE);
	deinit();

	abs_scale = absScale;
	table_size = palSize;
	BYTE *remapEntry = remap_table = (BYTE *)mem_add(table_size * (2*absScale+1) );
	remap_table_array = (BYTE **)mem_add(sizeof(BYTE *) * (2*absScale+1) );

	int scale;

	// ------- generate negative scale ----------//
	for( scale = -absScale; scale < 0; ++scale)
	{
		for( int c=0; c < palSize; ++c, ++remapEntry)
		{
			*remapEntry = c;

			RGBColor rgb = (*fp)(palD.get_rgb(c), scale, absScale);

			// ------- scan the closet color, except the reserved color
			int cc, dist[NEAREST_COLOR], thisDiff;
			BYTE closeColor[NEAREST_COLOR]; // [0] is the closest

			for( cc = 0; cc < NEAREST_COLOR; ++cc )
			{
				closeColor[cc] = c;
				dist[cc] = 3*0xff*0xff+1;
			}

			int d;
			for( d=0; d < palSize; ++d)
			{
				// ------- compare the sqaure distance ----------//
				thisDiff = color_dist(rgb, palD.get_rgb(d));

				if( thisDiff < dist[NEAREST_COLOR-1])
				{
					BYTE d1 = (BYTE) d; 
					for( cc = 0; cc < NEAREST_COLOR; ++cc )
					{
						if( thisDiff < dist[cc] )
						{
							// swap thisDiff and dist[cc]
							// so that the replaced result will be shifted to next
							int tempd;
							BYTE tempc;
							tempd = dist[cc];
							dist[cc] = thisDiff;
							thisDiff = tempd;

							tempc = closeColor[cc];
							closeColor[cc] = d1;
							d1 = tempc;
						}
					}
				}
			}

			// closeColor[] are the closest 8 colours, use hsv comparison to find the nearest
			d = closeColor[0];
			*remapEntry = d;
			int minDiff = color_dist_hsv(rgb, palD.get_rgb(d));
			for( cc = 1; cc < NEAREST_COLOR; ++cc)
			{
				d = closeColor[cc];
				thisDiff = color_dist_hsv(rgb, palD.get_rgb(d));
				if( thisDiff < minDiff )
				{
					minDiff = thisDiff;
					*remapEntry = d;
				}
			}
		}
	}

	err_when( remapEntry - remap_table != table_size * abs_scale);

	// scale == 0
	memcpy( remapEntry, identity_table, palSize);
	remapEntry += table_size;

	// ------- generate positive scale ----------//
	for( scale = 1; scale <= absScale; ++scale)
	{
		for( int c=0; c < palSize; ++c, ++remapEntry)
		{
			*remapEntry = c;

			RGBColor rgb = (*fp)(palD.get_rgb(c), scale, absScale);

			// ------- scan the closet color, except the reserved color
			int cc, dist[NEAREST_COLOR], thisDiff;
			BYTE closeColor[NEAREST_COLOR]; // [0] is the closest

			for( cc = 0; cc < NEAREST_COLOR; ++cc )
			{
				closeColor[cc] = c;
				dist[cc] = 3*0xff*0xff+1;
			}

			int d;
			for( d=0; d < palSize; ++d)
			{
				// ------- compare the sqaure distance ----------//
				thisDiff = color_dist(rgb, palD.get_rgb(d));
				if( thisDiff < dist[NEAREST_COLOR-1])
				{
					BYTE d1 = (BYTE) d; 
					for( cc = 0; cc < NEAREST_COLOR; ++cc )
					{
						if( thisDiff < dist[cc] )
						{
							// swap thisDiff and dist[cc]
							// so that the replaced result will be shifted to next
							int tempd;
							BYTE tempc;
							tempd = dist[cc];
							dist[cc] = thisDiff;
							thisDiff = tempd;

							tempc = closeColor[cc];
							closeColor[cc] = d1;
							d1 = tempc;
						}
					}
				}
			}

			// closeColor[] are the closest 8 colours, use hsv comparison to find the nearest
			d = closeColor[0];
			*remapEntry = d;
			int minDiff = color_dist_hsv(rgb, palD.get_rgb(d));
			for( cc = 1; cc < NEAREST_COLOR; ++cc)
			{
				d = closeColor[cc];
				thisDiff = color_dist_hsv(rgb, palD.get_rgb(d));
				if( thisDiff < minDiff )
				{
					minDiff = thisDiff;
					*remapEntry = d;
				}
			}
		}
	}

	create_table_array();
}
// ---------- end of function ColorTable::generate_table_fast ----------//


// ---------- begin of function ColorTable::generate_table ----------//
//
// match one set of palette with a universal palette
// the set of palette is pointed by sPal, size is sPalSize,
// with some reserved color pointed by sReservedColor and size is sReservedCount
// the univeral palette is pointed by Pal, size is PalSize,
// with some reserved color pointed by reservedColor and size is reservedCount
// note : numbers in reservedColor must be in ascending order
//
// generated map size must be palSize and it has only scale
//
void ColorTable::generate_table(PalDesc &sPalD, PalDesc &palD)
{
	int sPalSize = sPalD.pal_size, palSize = palD.pal_size;

	err_when(sPalD.pal == NULL || sPalSize <= 0 || sPalD.reserved_count < 0);
	err_when(palD.pal == NULL || palSize <= 0 || palD.reserved_count < 0);
	err_when(palSize > MAX_COLOUR_TABLE_SIZE || sPalSize > MAX_COLOUR_TABLE_SIZE);
	deinit();

	abs_scale = 0;
	table_size = sPalSize;
	BYTE *remapEntry = remap_table = (BYTE *)mem_add(sPalSize);
	remap_table_array = (BYTE **)mem_add(sizeof(BYTE *));

	int sReservedIndex = 0;
	for(int c=0; c < sPalSize; ++c, ++remapEntry)
	{
		*remapEntry = c;				// put a default value (as if it is a reserved color)
		
		// ------ see if it is a reserved color --------//
		if( sPalD.is_reserved(c, sReservedIndex))
			continue;

		RGBColor rgb = sPalD.get_rgb(c);

		// ------- scan the closet color, except the reserved color
		int cc, dist[NEAREST_COLOR], thisDiff;
		BYTE closeColor[NEAREST_COLOR]; // [0] is the closest

		for( cc = 0; cc < NEAREST_COLOR; ++cc )
		{
			closeColor[cc] = c;
			dist[cc] = 3*0xff*0xff+1;
		}
		int dReservedIndex = 0;
		int d;
		for( d=0; d < palSize; ++d)
		{
			// ------- skip scanning reserved color ------//
			if( palD.is_reserved(d, dReservedIndex) )
				continue;

			// ------- compare the sqaure distance ----------//
			thisDiff = color_dist(rgb, palD.get_rgb(d));

			if( thisDiff < dist[NEAREST_COLOR-1])
			{
				BYTE d1 = (BYTE) d; 
				for( cc = 0; cc < NEAREST_COLOR; ++cc )
				{
					if( thisDiff < dist[cc] )
					{
						// swap thisDiff and dist[cc]
						// so that the replaced result will be shifted to next
						int tempd;
						BYTE tempc;
						tempd = dist[cc];
						dist[cc] = thisDiff;
						thisDiff = tempd;

						tempc = closeColor[cc];
						closeColor[cc] = d1;
						d1 = tempc;
					}
				}
			}
		}

		// closeColor[] are the closest 8 colours, use hsv comparison to find the nearest
		d = closeColor[0];
		*remapEntry = d;
		int minDiff = color_dist_hsv(rgb, palD.get_rgb(d));
		for( cc = 1; cc < NEAREST_COLOR; ++cc)
		{
			d = closeColor[cc];
			thisDiff = color_dist_hsv(rgb, palD.get_rgb(d));
			if( thisDiff < minDiff )
			{
				minDiff = thisDiff;
				*remapEntry = d;
			}
		}
	}

	create_table_array();
}
// ---------- end of function ColorTable::generate_table ----------//


// ---------- begin of function ColorTable::get_table ----------//
BYTE *ColorTable::get_table(int scale)
{
	err_when( !remap_table );
	err_when( scale < -abs_scale || scale > abs_scale);
	return remap_table + table_size * (scale + abs_scale);
}
// ---------- end of function ColorTable::get_table ----------//


// ---------- begin of function ColorTable::create_table_array ----------//
void ColorTable::create_table_array()
{
	err_when( !remap_table );
	for( int j = 0; j < 2*abs_scale+1; ++j)
	{
		remap_table_array[j] = remap_table + table_size * j;
	}	
}
// ---------- end of function ColorTable::create_table_array ----------//


// ---------- begin of function ColorTable::bright_func ---------//
RGBColor ColorTable::bright_func(RGBColor c, int scale, int absScale)
{
	RGBColor ans;
	if( scale < 0)
	{
		double factor = sqrt(double(absScale + scale) / absScale);
		ans.red = BYTE(c.red * factor);
		ans.green = BYTE(c.green * factor);
		ans.blue = BYTE(c.blue * factor);
	}
	else
	{
		ans.red = c.red + (MAX_COLOUR - c.red) * scale / absScale;
		ans.green = c.green + (MAX_COLOUR - c.green) * scale / absScale;
		ans.blue = c.blue + (MAX_COLOUR - c.blue) * scale / absScale;
	}
	return ans;
}
// ---------- end of function ColorTable::bright_func ---------//


// ---------- begin of function ColorTable::patch_table --------//
void ColorTable::patch_table(BYTE from, BYTE to)
{
	err_when(from >= table_size);
	for(int s = -abs_scale; s <= abs_scale; ++s)
	{
		get_table(s)[from] = to;
	}
}
// ---------- end of function ColorTable::patch_table --------//


// ---------- begin of function ColorTable::color_dist --------//
int ColorTable::color_dist(RGBColor c1, RGBColor c2)
{
	return sq((int)c2.red-c1.red) + sq((int)c2.green-c1.green) + sq((int)c2.blue-c1.blue);
}
// ---------- end of function ColorTable::color_dist --------//


// ---------- begin of function ColorTable::color_dist_hsv --------//
int ColorTable::color_dist_hsv(RGBColor c1, RGBColor c2)
{
	// calculate a distance for the colour
	// h betweeh 0 and 6
	// s between 0 and 1
	// v between 0 and 1
	HSVColor hsv1(rgb2hsv(c1));
	HSVColor hsv2(rgb2hsv(c2));;

	double dx = hsv2.saturation * cos(hsv2.hue * PI / 3.0) - hsv1.saturation * cos(hsv1.hue * PI / 3.0);
	double dy = hsv2.saturation * sin(hsv2.hue * PI / 3.0) - hsv1.saturation * sin(hsv1.hue * PI / 3.0);
	double dv = hsv2.brightness - hsv1.brightness;

	return int(10000 * ( dx*dx + dy*dy + dv*dv ));
}
// ---------- end of function ColorTable::color_dist_hsv --------//


// -------- begin of function ColorTable::rgb2hsv ---------//
HSVColor ColorTable::rgb2hsv(RGBColor &rgb)
{
	if( rgb.red == rgb.green && rgb.red == rgb.blue)
	{
		return HSVColor(1.0, 0.0, rgb.red / 255.0);
	}

	// find the smallest colour
	if( rgb.red <= rgb.green && rgb.red <= rgb.blue)
	{
		if( rgb.green >= rgb.blue )
		{
			// g is the primary, b is secondary
			return HSVColor( 2.0 + (double) rgb.blue/ rgb.green, 
				rgb.blue != 0 ? 1.0 - (double) rgb.red / rgb.blue : 1.0,
				rgb.green / 255.0);
		}
		else
		{
			// b is the primary, g is secondary
			return HSVColor( 4.0 - (double) rgb.green / rgb.blue,
				rgb.green != 0 ? 1.0 - (double) rgb.red/ rgb.green : 1.0,
				rgb.blue / 255.0);
		}
	}
	else if( rgb.green <= rgb.red && rgb.green <= rgb.blue)
	{
		if( rgb.red >= rgb.blue)
		{
			// r is the primary, b is secondary
			return HSVColor( 6.0 - (double)rgb.blue/rgb.red,
				rgb.blue!=0 ? 1.0 - (double)rgb.green/rgb.blue: 1.0,
				rgb.red / 255.0);
		}
		else
		{
			// b is the primary, r is secondary
			return HSVColor( 4.0 + (double)rgb.red/rgb.blue,
				rgb.red!=0 ? 1.0 - (double)rgb.green/rgb.red: 1.0,
				rgb.blue / 255.0);
		}
	}
	else if( rgb.blue <= rgb.red && rgb.blue <= rgb.green)
	{
		if( rgb.red >= rgb.green)
		{
			// r is the primary, g is secondary
			return HSVColor( (double)rgb.green/rgb.red,
				rgb.green!=0 ? 1.0 - (double)rgb.blue/rgb.green: 1.0,
				rgb.red / 255.0);
		}
		else
		{
			// g is the primary, r is secondary
			return HSVColor( 2.0 - (double)rgb.red/rgb.green,
				rgb.red!=0 ? 1.0 - (double)rgb.blue/rgb.red: 1.0,
				rgb.green / 255.0);
		}
	}
	else
	{
		err_here();
		return HSVColor( 1.0, 0.0, rgb.red / 255.0);
	}
}
// -------- end of function ColorTable::rgb2hsv ---------//


// -------- begin of function ColorTable::hsv2rgb ---------//
RGBColor ColorTable::hsv2rgb(HSVColor &hsv)
{
	while( hsv.hue < 0.0)
		hsv.hue += 6.0;
	while(hsv.hue >= 6.0)
		hsv.hue -= 6.0;
	double p = hsv.brightness * 255.0;
	err_when( p >= 256.0);

	RGBColor ans;

	if( hsv.hue < 1.0)
	{
		// r is primary, g is secondary
		ans.red = BYTE(p);
		p *= hsv.hue;
		ans.green = BYTE(p);                // *r * h;
		p *= 1.0 - hsv.saturation;
		ans.blue = BYTE(p);
	}
	else if( hsv.hue < 2.0)
	{
		// g is primary, r is secondary
		ans.green = BYTE(p);
		p *= 2.0 - hsv.hue;
		ans.red = BYTE(p);
		p *= 1.0 - hsv.saturation;
		ans.blue = BYTE(p);
	}
	else if( hsv.hue < 3.0)
	{
		// g is primary, b is secondary
		ans.green = BYTE(p);
		p *= hsv.hue - 2.0;
		ans.blue = BYTE(p);
		p *= 1.0 - hsv.saturation;
		ans.red = BYTE(p);
	}
	else if( hsv.hue < 4.0)
	{
		// b is primary g is secondary
		ans.blue = BYTE(p);
		p *= 4.0 - hsv.hue;
		ans.green = BYTE(p);
		p *= 1.0 - hsv.saturation;
		ans.red = BYTE(p);
	}
	else if( hsv.hue < 5.0)
	{
		// b is primary, r is secondary
		ans.blue = BYTE(p);
		p *= hsv.hue - 4.0;
		ans.red = BYTE(p);
		p *= 1.0 - hsv.saturation;
		ans.green = BYTE(p);
	}
	else if( hsv.hue < 6.0)
	{
		// r is primary, b is secondary
		ans.red = BYTE(p);
		p *= 6.0 - hsv.hue;
		ans.blue = BYTE(p);
		p *= 1.0 - hsv.saturation;
		ans.green = BYTE(p);
	}

	return ans;
}
// -------- end of function ColorTable::hsv2rgb ---------//


// -------- begin of function ColorTable::write_file ---------//
int ColorTable::write_file(File *f)
{
	return( f->file_put_long(abs_scale) && !f->file_put_long(table_size)
		&& f->file_write(remap_table, table_size * (2*abs_scale+1)) );
}
// -------- end of function ColorTable::write_file ---------//


// -------- begin of function ColorTable::read_file ---------//
int ColorTable::read_file(File *f)
{
	deinit();
	abs_scale = f->file_get_long();
	table_size = f->file_get_long();

	remap_table = (BYTE *)mem_add(table_size * (2*abs_scale+1) );
	if(! f->file_read(remap_table, table_size * (2*abs_scale+1)) )
	{
		mem_del(remap_table);
		remap_table = 0;
		return 0;
	}
	remap_table_array = (BYTE **)mem_add(sizeof(BYTE *) * (2*abs_scale+1) );
	create_table_array();
	return 1;
}
// -------- end of function ColorTable::read_file ---------//
