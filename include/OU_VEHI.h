//Filename    : OU_VEHI.H
//Description : Header file of Unit Vehicle

#ifndef __OU_VEHI_H
#define __OU_VEHI_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

//----------- Define class UnitVehicle -----------//

class UnitVehicle : public Unit
{
public:
	short solider_hit_points;		// the original hit points of the solider before it gets on the vehicle
	short vehicle_hit_points;		// the original hit points of the vehicle before the soliders gets on it

public:
	void	set_combat_level(int);
	void	dismount();

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();
};

//--------------------------------------------//

#endif
