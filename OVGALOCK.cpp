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


VgaCustomPalette::VgaCustomPalette(char *fileName)
{
	backup_pal = NULL;
	if( save_palette() && fileName)
		set_custom_palette(fileName);
}

VgaCustomPalette::~VgaCustomPalette()
{
	restore_palette();
	if( backup_pal)
		mem_del(backup_pal);
}


int VgaCustomPalette::save_palette()
{
	// ------ allocate space --------//
	if( !backup_pal )
		backup_pal = mem_add( sizeof(PALETTEENTRY) * 256);

	// ------- get current palette --------//
	if( vga.dd_pal->GetEntries(0, 0, 256, (PALETTEENTRY *)backup_pal) )
	{
		// get palette fail, free backup_pal to indicate save_palette failed
		mem_del(backup_pal);
		backup_pal = NULL;
		return 0;
	}
	else
		return 1;
}



int VgaCustomPalette::set_custom_palette(char *fileName)
{
	PALETTEENTRY palEntry[256];
	char palBuf[256][3];
	File palFile;

	palFile.file_open(fileName);
	palFile.file_seek(8);     				// bypass the header info
	palFile.file_read(palBuf, 256*3);
	palFile.file_close();

	for(int i=0; i<256; i++)
	{
		palEntry[i].peRed   = palBuf[i][0];
		palEntry[i].peGreen = palBuf[i][1];
		palEntry[i].peBlue  = palBuf[i][2];
		palEntry[i].peFlags = 0;
	}

	return !vga.dd_pal->SetEntries(0, 0, 256, palEntry);
}


int VgaCustomPalette::restore_palette()
{
	if( backup_pal)
		return !vga.dd_pal->SetEntries(0, 0, 256, (PALETTEENTRY *)backup_pal);
	else
		return 1;
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

