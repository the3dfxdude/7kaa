// Filename    : OCOLTBL.H
// Description : Header file for color remap table ColorTable

#ifndef __OCOLTBL_H
#define __OCOLTBL_H

typedef unsigned char BYTE;

#define MAX_COLOUR_TABLE_SIZE 0x100

// ---------- define RGBColor --------//
struct RGBColor
{
	BYTE	red;
	BYTE	green;
	BYTE	blue;

	RGBColor() {}
	RGBColor(BYTE r, BYTE g, BYTE b) : red(r), green(g), blue(b) {}
};

// ---------- define HSVColor --------//
struct HSVColor
{
	double hue;
	double saturation;
	double brightness;

	HSVColor() {}
	HSVColor(double h, double s, double v) : hue(h), saturation(s), brightness(v) {}
};

// ---------- define struct PalDesc ---------//
struct PalDesc
{
	BYTE *pal;
	int	pal_pitch;		// 3 for RGB, or sizeof(PALETTEENTRY)
	int	pal_size;		// number of color
	int	bit_width;		// 8 for 8-bit, 6 for 6-bit
	BYTE *reserved_color;
	int	reserved_count;
	
	PalDesc(BYTE *palPtr, int pitch, int size, int bitWidth, BYTE *rPtr =(unsigned char*)0, int rCount=0):
		pal(palPtr), pal_pitch(pitch), pal_size(size), bit_width(bitWidth),
		reserved_color(rPtr), reserved_count(rCount) {}

	BYTE	normalize(BYTE c)       { return bit_width==8 ? c : (c<<(8-bit_width))+ (1<<(7-bit_width)); }
	BYTE	red(int n)              { return normalize(pal[pal_pitch*n]); }
	BYTE	green(int n)            { return normalize(pal[pal_pitch*n+1]); }
	BYTE	blue(int n)             { return normalize(pal[pal_pitch*n+2]); }
	RGBColor get_rgb(int n)			{ return RGBColor(red(n), green(n), blue(n)); }
	int	is_reserved(BYTE c, int &reservedIndex)
	{
		if( reserved_color && reservedIndex < reserved_count &&
			c == reserved_color[reservedIndex])
		{
			reservedIndex ++;
			return 1;
		}
		return 0;
	}
};


// ----------- define class ColorTable ---------//

class File;

class ColorTable
{
private:
	BYTE *remap_table;
	BYTE **remap_table_array;

public:
	int	abs_scale;
	int	table_size;
	static BYTE identity_table[MAX_COLOUR_TABLE_SIZE];

public:
	ColorTable();
	ColorTable(int absScale, int tableSize, BYTE *customTable);
	ColorTable(const ColorTable &);
	~ColorTable();
	void	init();
	void	init(int absScale, int tableSize, BYTE *customTable);
	void	deinit();
	ColorTable& operator=(const ColorTable &);

	static RGBColor bright_func( RGBColor, int, int);

	// pass ColorTable::bright_func as the argument fp for brightness table
	void	generate_table(int absScale, PalDesc &palD, RGBColor (*fp)(RGBColor, int, int));
	void	generate_table_fast(int absScale, PalDesc &palD, RGBColor (*fp)(RGBColor, int, int));
	void	generate_table(PalDesc &sPalD, PalDesc &palD);
	BYTE *get_table(int scale=0);
	BYTE **get_table_array(int scale=0)	{ return remap_table_array + scale + abs_scale; }
	void	patch_table(BYTE from, BYTE to);

	int	write_file(File *);
	int	read_file(File *);

private:
	void	create_table_array();

	static int	color_dist(RGBColor, RGBColor);
	static int	color_dist_hsv(RGBColor, RGBColor);
	static HSVColor rgb2hsv(RGBColor &);
	static RGBColor hsv2rgb(HSVColor &);
};

#endif