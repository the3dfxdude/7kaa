// Filename    : OVOLUME.CPP
// Description : volume unit type

#include <OAUDIO.h>
#include <OCONFIG.h>
#include <OVOLUME.h>

const DEFAULT_VOL_DROP = 100;	// distance 100, no sound
const	DEFAULT_DIST_LIMIT = 40;
const DEFAULT_PAN_DROP = 100;	// distance 100, extreme left or extreme right
RelVolume DEF_REL_VOLUME(100,0);


DsVolume::DsVolume(long dsVol, long dsPan) : ds_vol(dsVol), ds_pan(dsPan)
{
}

DsVolume::DsVolume(AbsVolume &absVolume) : ds_vol(absVolume.abs_vol*100-10000), ds_pan(absVolume.ds_pan)
{
}

DsVolume::DsVolume(RelVolume &relVolume)
	: ds_vol(audio.vol_multiply(relVolume.rel_vol)), ds_pan(relVolume.ds_pan)
{
}


AbsVolume::AbsVolume(long absVol, long dsPan)
	: abs_vol(absVol), ds_pan(dsPan)
{
}

AbsVolume::AbsVolume(DsVolume &dsVolume)
	: abs_vol((dsVolume.ds_vol+10000)/100), ds_pan(dsVolume.ds_pan)
{
}

RelVolume::RelVolume(long relVol, long dsPan)
	: rel_vol(relVol), ds_pan(dsPan)
{
}

RelVolume::RelVolume(PosVolume &posVolume)
{
	long absX = posVolume.x >= 0 ? posVolume.x : -posVolume.x;
	long absY = posVolume.y >= 0 ? posVolume.y : -posVolume.y;
	long dist = absX >= absY ? absX :absY;
	if( dist <= DEFAULT_DIST_LIMIT )
		rel_vol = rel_vol = 100 - dist * 100 / DEFAULT_VOL_DROP;
	else
		rel_vol = 0;

	if( posVolume.x >= DEFAULT_PAN_DROP )
		ds_pan = 10000;
	else if( posVolume.x <= -DEFAULT_PAN_DROP )
		ds_pan = -10000;
	else
		ds_pan = 10000 / DEFAULT_PAN_DROP * posVolume.x;
}


RelVolume::RelVolume(PosVolume &posVolume, int drop, int limit)
{
	long absX = posVolume.x >= 0 ? posVolume.x : -posVolume.x;
	long absY = posVolume.y >= 0 ? posVolume.y : -posVolume.y;
	long dist = absX >= absY ? absX :absY;
	if( dist <= limit )
		rel_vol = 100 - dist * 100 / drop;
	else
		rel_vol = 0;
	
	if( posVolume.x >= DEFAULT_PAN_DROP )
		ds_pan = 10000;
	else if( posVolume.x <= -DEFAULT_PAN_DROP )
		ds_pan = -10000;
	else
		ds_pan = 10000 / DEFAULT_PAN_DROP * posVolume.x;
}


PosVolume::PosVolume(long relLocX, long relLocY) : x(relLocX), y(relLocY)
{
}
