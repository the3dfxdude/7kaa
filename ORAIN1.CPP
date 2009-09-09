// Filename   : ORAIN1.CPP
// Description: Class RainDrop
// Ownership  : Gilbert

#include <ORAIN.H>
#include <OVGABUF.H>
#include <COLOR.H>
#include <OWORLDMT.H>

//--------- Begin of function RainDrop::init ------------//
//
void RainDrop::init(Rain *, short fromX, short fromY, short toX, short toY, short speed )
{
	cur_x = fromX;
	cur_y = fromY;
	dest_x = toX;
	dest_y = toY;
	fall_speed = speed;
}
//--------- End of function RainDrop::init ------------//

//--------- Begin of function RainDrop::fall ------------//
//
void RainDrop::fall()
{
	if( cur_y + fall_speed >= dest_y )
	{
		cur_y = dest_y;
		cur_x = dest_x;
	}
	else
	{
		cur_x += (dest_x - cur_x) * fall_speed / (dest_y - cur_y);
		cur_y += fall_speed;
	}
}
//--------- End of function RainDrop::fall ------------//

//--------- Begin of function RainDrop::is_goal ------------//
//
int RainDrop::is_goal()
{
	return( cur_y + fall_speed >= dest_y);
}
//--------- End of function RainDrop::is_goal ------------//

//--------- Begin of function RainDrop::draw_step ------------//
//
void RainDrop::draw_step(VgaBuf *vgabuf)
{
	short preX = cur_x;
	short preY = cur_y;
	fall();
	preX = (preX + cur_x) /2;
	preY = (preY + cur_y) /2;
	// ####### begin Gilbert 8/5 ######//
	if( preX >= ZOOM_X1 && cur_x >= ZOOM_X1 &&
		preX <= ZOOM_X2 && cur_x <= ZOOM_X2 &&
		preY >= ZOOM_Y1 && cur_y >= ZOOM_Y1 &&
		preY <= ZOOM_Y2 && cur_y <= ZOOM_Y2 )
	{
		short q2X = (preX + cur_x) /2;
		short q2Y = (preY + cur_y) /2;
		short q3X = (preX + 3*cur_x) /4;
		short q3Y = (preY + 3*cur_y) /4;
		vgabuf->line(preX, preY, q2X, q2Y, VGA_GRAY+8);
		vgabuf->line(q2X, q2Y, q3X, q3Y, VGA_GRAY+10);
		vgabuf->line(q3X, q3Y, cur_x, cur_y, VGA_GRAY+12);
	}
	else
	{
		int wrap_size = ZOOM_X2 - ZOOM_X1;
		// if the drop goes outside the screen, come back from the other side
		if( cur_x < ZOOM_X1)
		{
			cur_x += wrap_size;
			dest_x += wrap_size;
		}
		else if( cur_x > ZOOM_X2)
		{
			cur_x -= wrap_size;
			dest_x -= wrap_size;
		}
	}
	// ####### end Gilbert 8/5 ######//
}
//--------- End of function RainDrop::draw_step ------------//
