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

//Filename    : OF_WAR.H
//Description : Header of FirmWar war factory.

#ifndef __OF_WAR_H
#define __OF_WAR_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//-------- Define constant ---------//

#define MAX_BUILD_QUEUE				20

//------- Define class FirmWar --------//

#pragma pack(1)
class FirmWar : public Firm
{
public:
	short build_unit_id;
	uint32_t last_process_build_frame_no;
	float build_progress_days;

	char  build_queue_array[MAX_BUILD_QUEUE];		// it stores the unit id.
	char  build_queue_count;

	int	is_operating()		{ return productivity > 0 && build_unit_id; }

public:
	FirmWar();
	~FirmWar();

	void 	put_info(int refreshFlag);
	void 	detect_info();

	void 	disp_main_menu(int refreshFlag);
	void 	detect_main_menu();

	void 	disp_build_menu(int refreshFlag);
	void 	detect_build_menu();

	void    change_nation(int newNationRecno);

	void	next_day();
	void	process_ai();

	virtual	FirmWar* cast_to_FirmWar() { return this; };
	void	add_queue(int unitId, int amount = 1);
	void	remove_queue(int unitId, int amount = 1);
	void	cancel_build_unit();

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t	crc8();
	virtual	void	clear_ptr();

	enum {FIRMWAR_BUILD_BATCH_COUNT = 10}; // Number of units enqueued when holding shift - ensure this is less than MAX_BUILD_QUEUE

private:
	void 	disp_war_info(int dispY1, int refreshFlag);
	void  disp_build_button(int y, int unitId, int buttonUp);
	void  disp_queue_button(int y, int unitId, int buttonUp);

	void 	process_build();
	void	process_queue();

	//-------- AI actions ---------//

	void	think_new_production();
	int 	should_build_new_weapon();
	int	think_del();
};
#pragma pack()

//--------------------------------------//

#endif
