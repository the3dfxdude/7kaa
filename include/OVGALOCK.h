// Filename    : OVGALOCK.H
// Description : header file for VgaFrontLock
// Owner       : Gilbert

#ifndef __OVGALOCK_H
#define __OVGALOCK_H

class VgaFrontLock
{
public:
	VgaFrontLock();
	~VgaFrontLock();
	void	re_lock();
	void	re_unlock();
};

class VgaCustomPalette
{
private:
	void *backup_pal;

public:
	VgaCustomPalette( char * );
	~VgaCustomPalette();

	static int set_custom_palette(char *);

private:
	int save_palette();
	int restore_palette();
};


class MouseDispCount
{
private:
	void *cursor_handle;
public:
	MouseDispCount();
	~MouseDispCount();
};

#endif