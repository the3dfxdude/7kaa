// Filename    : OWARPT.CPP
// Description : War point


#include <COLOR.h>
#include <OVGABUF.h>
#include <OWORLD.h>
#include <OWARPT.h>
#include <ALL.h>

// --------- define constant ---------//

// divide the map into zone, each zone has size WARPOINT_ZONE_SIZE

#define WARPOINT_ZONE_SIZE 8
#define WARPOINT_ZONE_COLUMN ((MAX_MAP_WIDTH + WARPOINT_ZONE_SIZE -1) / WARPOINT_ZONE_SIZE)
#define WARPOINT_ZONE_ROW ((MAX_MAP_HEIGHT + WARPOINT_ZONE_SIZE -1) / WARPOINT_ZONE_SIZE)

#define WARPOINT_STRENGTH 0x100000
#define WARPOINT_STRENGTH_MAX 0x1000000

#define DRAW_PHASE_PERIOD 16


// ---------- begin of function WarPoint::inc ----------//
void WarPoint::inc()
{
	if( (strength += WARPOINT_STRENGTH) > WARPOINT_STRENGTH_MAX )
		strength = WARPOINT_STRENGTH_MAX;
}
// ---------- end of function WarPoint::inc ----------//


// ---------- begin of function WarPoint::decay ----------//
inline void WarPoint::decay()
{
	strength >>= 1;
}
// ---------- end of function WarPoint::decay ----------//


// ------ begin of function WarPointArray::WarPointArray -------//
WarPointArray::WarPointArray()
{
	war_point = NULL;
	init_flag = 0;
}
// ------ end of function WarPointArray::WarPointArray -------//


// ------ begin of function WarPointArray::~WarPointArray -------//
WarPointArray::~WarPointArray()
{
	deinit();
}
// ------ end of function WarPointArray::~WarPointArray -------//


// ------ begin of function WarPointArray::init -------//
void WarPointArray::init()
{
	deinit();

	// allocate memory for war_point
	war_point = (WarPoint *)mem_add( sizeof(WarPoint) * WARPOINT_ZONE_COLUMN * WARPOINT_ZONE_ROW );
	memset(war_point, 0, sizeof(WarPoint) * WARPOINT_ZONE_COLUMN * WARPOINT_ZONE_ROW );

	draw_phase = 0;
	init_flag = 1;
}
// ------ end of function WarPointArray::init -------//


// ------ begin of function WarPointArray::deinit -------//
void WarPointArray::deinit()
{
	if( init_flag )
	{
		mem_del(war_point);
		init_flag = 0;
	}
}
// ------ end of function WarPointArray::deinit -------//


// ------ begin of function WarPointArray::draw_dot -------//
void WarPointArray::draw_dot()
{
	if( ++draw_phase >= DRAW_PHASE_PERIOD )
		draw_phase = 0;

	if( draw_phase & 1)
		return;

	static unsigned char dotColor[DRAW_PHASE_PERIOD/2] = 
		{ 0xa0, 0xa4, 0xa8, 0x00, 0xb0, 0xb4, 0xb8, 0x00 };
	unsigned char color = dotColor[draw_phase / 2];

	int x,y;
	short mapY;
	unsigned char *writePtr;
	unsigned char *vgaBufPtr = (unsigned char *)vga_back.buf_ptr();
	int vgaBufPitch = vga_back.buf_pitch();

 	for( y = 0, mapY=MAP_Y1; y < WARPOINT_ZONE_ROW; ++y, mapY+=WARPOINT_ZONE_SIZE)
	{
		WarPoint *warPt = war_point + y * WARPOINT_ZONE_COLUMN;
		writePtr = vgaBufPtr + vgaBufPitch * mapY + MAP_X1;
		for( x = 0; x < WARPOINT_ZONE_COLUMN; ++x, ++warPt, writePtr+=WARPOINT_ZONE_SIZE)
		{
			if( warPt->strength > 0 )
			{
				// draw a cross, UNEXPLORED_COLOR is not needed to check
				unsigned char *map1Ptr = writePtr;
				unsigned char *map2Ptr = writePtr + WARPOINT_ZONE_SIZE-2;
				for( int i = 1; i < WARPOINT_ZONE_SIZE; ++i )
				{
					*map1Ptr = color;
					*map2Ptr = color;
					map1Ptr += vgaBufPitch + 1;
					map2Ptr += vgaBufPitch - 1;
				}
			}
		}
	}
}
// ------ end of function WarPointArray::draw_dot -------//


// ------ begin of function WarPointArray::process -------//
void WarPointArray::process()
{
	WarPoint *warPt = war_point;
	for( int i = WARPOINT_ZONE_COLUMN * WARPOINT_ZONE_ROW; i > 0; --i, warPt++)
	{
		warPt->decay();
	}
}
// ------ end of function WarPointArray::process -------//


// ------ begin of function WarPointArray::get_ptr -------//
WarPoint *WarPointArray::get_ptr(int xLoc, int yLoc)
{
	err_when(!init_flag);
	int c = xLoc / WARPOINT_ZONE_SIZE;
	int r = yLoc / WARPOINT_ZONE_SIZE;
	err_when( c < 0 || c >= WARPOINT_ZONE_COLUMN);
	err_when( r < 0 || r >= WARPOINT_ZONE_ROW);
	return war_point+ (r * WARPOINT_ZONE_COLUMN + c);
}
// ------ end of function WarPointArray::get_ptr -------//


// ------ begin of function WarPointArray::add_point -------//
void WarPointArray::add_point(int xLoc, int yLoc)
{
	get_ptr(xLoc, yLoc)->inc();
}
// ------ end of function WarPointArray::add_point -------//
