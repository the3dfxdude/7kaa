// Filename    : OEXPMASK.CPP
// Description : explored area mask
// Owner       : Gilbert


#include <OEXPMASK.h>
#include <OFILE.h>
#include <OWORLDMT.h>
#include <OVGABUF.h>
#include <OSTR.h>
#include <OCOLTBL.h>
#include <OCONFIG.h>

// --------- Define Constant ------//
#define MASK_FILENAME "EXPLMASK.BIN"
#define REMAP_FILENAME "EXPREMAP.BIN"

// no. of darkness scale defined as MAX_BRIGHTNESS_ADJUST_DEGREE in OVGA.H

// ------- Begin of function ExploredMask::init ---------//
void ExploredMask::init(ColorTable *colorTable)
{
	brightness_table = colorTable;

	// ------- read into exploration mask bitmap --------//
	String str;
	str = DIR_RES;
	str += MASK_FILENAME;
	File maskFile;
	maskFile.file_open(str);
	int fileSize = maskFile.file_size();
	mask_bitmap = (char *) mem_add( fileSize );
	maskFile.file_read(mask_bitmap, fileSize);
	maskFile.file_close();

	// ------- read into exploration remap bitmap ------//
	str = DIR_RES;
	str += REMAP_FILENAME;
	File remapFile;
	remapFile.file_open(str);
	fileSize = remapFile.file_size();
	remap_bitmap = (char *) mem_add(fileSize);
	remapFile.file_read(remap_bitmap, fileSize);
	remapFile.file_close();
}
// ------- End of function ExploredMask::init ---------//


// ------- Begin of function ExploredMask::deinit ---------//
void ExploredMask::deinit()
{
	mem_del(mask_bitmap);
	mem_del(remap_bitmap);
}
// ------- End of function ExploredMask::deinit ---------//


// ------- Begin of function ExploredMask::draw ---------//
//
// Draw a mask for explored area
// short xLoc, yLoc            location of the mask
// int northRow                explored flags, bit 0 for northeast square
//                             bit 1 for north square, bit 2 for northwest square
// int thisRow                 explored flags, bit 0 for east square
//                             bit 1 for this square, bit 2 for west square
// int southRow                explored flags, bit 0 for southeast square
//                             bit 1 for south square, bit 2 for southwest square
//
void ExploredMask::draw(short x, short y, int northRow, int thisRow, int southRow)
{
	switch(config.explore_mask_method)
	{
	case 0:
		break;
	case 1:		// use bit masking
		vga_back.explore_mask(x, y, mask_bitmap, northRow, thisRow, southRow);
		break;
	case 2:		// use remapping
		vga_back.explore_remap(x, y, remap_bitmap, (char **)brightness_table->get_table_array(),
			northRow, thisRow, southRow);
		break;
	default:
		err_here();
	}
}
// ------- End of function ExploredMask::draw ---------//


