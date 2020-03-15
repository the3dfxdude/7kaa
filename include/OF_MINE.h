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

//Filename    : OF_MINE.H
//Description : Header of FirmMine

#ifndef __OF_MINE_H
#define __OF_MINE_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

#ifndef __ORAWRES_H
#include <ORAWRES.h>
#endif

//-------- define constant ---------//

#define DEFAULT_MINE_MAX_STOCK_QTY 	500

struct FirmMineCrc;

//------- Define class FirmMine --------//

#pragma pack(1)
class FirmMine : public Firm
{
public:
	short		raw_id;
	short		site_recno;
	float		reserve_qty;		// non-mined raw materials reserve
	float		stock_qty;			// mined raw materials stock
	float		max_stock_qty;

	short		next_output_link_id;
	short		next_output_firm_recno;

	float 	cur_month_production;
	float		last_month_production;
	float		production_30days()		{ return last_month_production*(30-info.game_day)/30 +
															cur_month_production; }

	int		is_operating()		{ return productivity > 0 && reserve_qty > 0; }

	int	   ai_has_excess_worker();

public:
	FirmMine();
	~FirmMine();

	void 		init_derived();

	void 		change_nation(int newNationRecno);

	void		draw(int displayLayer=1);

	void 		put_info(int refreshFlag);
	int		detect_info();

	void 		disp_mine_info(int dispY1, int refreshFlag);

	void		next_day();
	void		next_month();

	virtual	FirmMine* cast_to_FirmMine() { return this; };

	void		process_ai();

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(FirmMineCrc *c);

private:
	void 		 produce_raw();
	Location* scan_raw_site();
	void 		 set_next_output_firm();

	//------------ AI actions ---------------//

	int		think_build_market();
	int		think_inc_productivity();
};
#pragma pack()

//--------------------------------------//

#endif
