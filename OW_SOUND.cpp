// Filename   : OW_SOUND.CPP
// Description: Ambient Sound Functions

#include <OSYS.h>
#include <OAUDIO.h>
#include <OSE.h>
#include <OVOLUME.h>
#include <OWEATHER.h>
#include <OWORLD.h>

// ------- define constant -------//
#define MAX_BIRD 17

//------- Begin of function World::process_ambient_sound -------//
//
void World::process_ambient_sound()
{
	int temp = weather.temp_c();
	if( weather.rain_scale() == 0 && temp >= 15 && m.random(temp) >= 12)
	{
		int bird = m.random(MAX_BIRD) + 1;
		char sndFile[] = "BIRDS00";
		err_when( bird > 99 );
		sndFile[5] = (bird / 10) + '0';
		sndFile[6] = (bird % 10) + '0';

		int xLoc = m.random(max_x_loc) - (zoom_matrix->top_x_loc + zoom_matrix->disp_x_loc/2);
		int yLoc = m.random(max_y_loc) - (zoom_matrix->top_y_loc + zoom_matrix->disp_y_loc/2);
		RelVolume relVolume(PosVolume(xLoc, yLoc), 200, MAX_MAP_WIDTH);
		if( relVolume.rel_vol < 80)
			relVolume.rel_vol = 80;

		se_ctrl.request(sndFile, relVolume);
	}
}
//-------- End of function World::process_ambient_sound --------//
