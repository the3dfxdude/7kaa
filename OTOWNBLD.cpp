//Filename    : OTOWNSE2.CPP
//Description : Object TownZone

#include <OVGA.H>
#include <OGAME.H>
#include <OINFO.H>
#include <OTOWN.H>
#include <OWORLD.H>
#include <OTOWNRES.H>


//------- Begin of function TownBuild::draw -----------//
//
// Draw the town building on the zoom map
//
// <int> absBaseX, absBaseY - the absolute base (center-bottom) coordination
//										of the building.
//
// <int> selectedFlag - whether the town of this building is currently selected or not.
//
void TownBuild::draw(int townRecno, int absBaseX, int absBaseY)
{
	int absX1 = absBaseX - bitmap_width/2;
	int absY1 = absBaseY - bitmap_height;
	int absX2 = absX1 + bitmap_width  - 1;
	int absY2 = absY1 + bitmap_height - 1;

	//-------- check if the firm is within the view area --------//

	int x1 = absX1 - World::view_top_x;

	if( x1 <= -bitmap_width || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = absY1 - World::view_top_y;

	if( y1 <= -bitmap_height || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//

	int x2 = absX2 - World::view_top_x;
	int y2 = absY2 - World::view_top_y;

	//------- get the color remap table for this sprite ------//

	Town* townPtr = town_array[townRecno];

	char* colorRemapTable = game.get_color_remap_table(townPtr->nation_recno, town_array.selected_recno==townRecno);

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		vga_back.put_bitmap_area_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
			max(0,x1)-x1, max(0,y1)-y1, min(ZOOM_WIDTH-1,x2)-x1, min(ZOOM_HEIGHT-1,y2)-y1, colorRemapTable );
	}

	//---- the whole bitmap is inside the view area ------//

	else
	{
		vga_back.put_bitmap_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr, colorRemapTable );
	}
}
//--------- End of function TownBuild::draw -----------//

