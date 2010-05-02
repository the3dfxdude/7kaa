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

// Filename    : OVGALOCK.CPP
// Description : object to vga_front.temp_unlock and temp_restore_lock
// Onwer       : Gilbert Luis
// Mainly used by ODPLAY.CPP to separate OVGA.H
// some GUID initialization problems exist if OVGA.H is included in ODPLAY.CPP

#include <OSYS.h>
#include <OVGALOCK.h>
#include <OVGABUF.h>
#include <OVGA.h>
#include <OFILE.h>
#include <OMOUSE.h>


VgaFrontLock::VgaFrontLock()
{
	vga_front.temp_unlock();
}

VgaFrontLock::~VgaFrontLock()
{
	vga_front.temp_restore_lock();
}

void VgaFrontLock::re_lock()
{
	vga_front.temp_restore_lock();
}

void VgaFrontLock::re_unlock()
{
	vga_front.temp_unlock();
}


#ifdef USE_DPLAY
VgaCustomPalette::VgaCustomPalette(char *fileName)
{
	vga.set_custom_palette(fileName);
}

VgaCustomPalette::~VgaCustomPalette()
{
	vga.free_custom_palette();
}


MouseDispCount::MouseDispCount()
{
	// set cursor position
	SetCursorPos( mouse.cur_x, mouse.cur_y);

	// show cursor
	mouse.hide();
	// #### patch begin Gilbert 9/1 #######//
	vga_front.temp_unlock();
	// #### patch end Gilbert 9/1 #######//
	ShowCursor(TRUE);
}

MouseDispCount::~MouseDispCount()
{
	// set cursor position
	POINT winMousePos;
	GetCursorPos(&winMousePos);
	mouse.cur_x = winMousePos.x;
	mouse.cur_y = winMousePos.y;

	// hide cursor
	ShowCursor(FALSE);
	// #### patch begin Gilbert 9/1 #######//
	vga_front.temp_restore_lock();
	// #### patch end Gilbert 9/1 #######//
	mouse.show();
	int ev = mouse.get_event();
	ev = mouse.get_event();

}
#endif //USE_DPLAY

