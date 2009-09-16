// Filename    : OSNOW2.CPP
// Description : Class Snow
// Ownership   : Gilbert

#include <OSNOW.H>
#include <OVGABUF.H>
#include <ALL.H>

//------------ Begin of function Snow::set_bound ---------//
void Snow::set_bound(int x1, int y1, int x2, int y2)
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		layer[i].set_bound(x1, y1, x2, y2);
	}
}
//------------ End of function Snow::set_bound ---------//

//------------ Begin of function Snow::init ---------//
void Snow::init(double s, char animSpeed)
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		// slower the snow, denser
		layer[i].init( 15 + 10*i + animSpeed, 20 + 10*i + animSpeed, 3+i*2, i+2, i/2, s, animSpeed);
	}
}
//------------ End of function Snow::init ---------//

//------------ Begin of function Snow::fall ---------//
void Snow::fall()
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		layer[i].fall();
	}
}
//------------ End of function Snow::fall ---------//

//------------ Begin of function Snow::draw_step ---------//
void Snow::draw_step(VgaBuf *vgabuf)
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		layer[i].draw_step(vgabuf);
	}
}
//------------ End of function Snow::draw_step ---------//
