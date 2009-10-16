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

//Filename    : OF_BASE.H
//Description : Header of FirmBase

#ifndef __OF_BASE_H
#define __OF_BASE_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

#ifndef __OSKILL_H
#include <OSKILL.h>
#endif

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//------- Define constant -----------//

#define  	MAX_BASE_PRAYER		30
#define		MAX_PRAY_POINTS		400

//------- Define class FirmBase --------//

#pragma pack(1)
class FirmBase : public Firm
{
public:
	short		god_id;
	short  	god_unit_recno;		// unit recno of the summoned god

	float		pray_points;

public:
	FirmBase();
	~FirmBase();

	void		init_derived();

	void 		assign_unit(int unitRecno);
	void 		assign_overseer(int overseerRecno);

	void 		change_nation(int newNationRecno);

	void 		put_info(int refreshFlag);
	void 		detect_info();

	void		next_day();
	void		process_ai();

	int		can_invoke();
	void		invoke_god();

	virtual	FirmBase* cast_to_FirmBase() { return this; };

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

private:
	void 		disp_base_info(int dispY1, int refreshFlag);
	void 		disp_god_info(int dispY1, int refreshFlag);

	void		train_unit();
	void 		recover_hit_point();

	//------------- AI actions --------------//

	void 		think_assign_unit();
	void 		think_invoke_god();
};
#pragma pack()

//--------------------------------------//

#endif
