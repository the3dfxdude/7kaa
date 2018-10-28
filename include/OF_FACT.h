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

//Filename    : OF_FACT.H
//Description : Header of FirmFactory

#ifndef __OF_FACT_H
#define __OF_FACT_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

struct FirmFactoryCrc;

//------- Define class FirmFactory --------//

#pragma pack(1)
class FirmFactory : public Firm
{
public:
	int   	product_raw_id;	// the raw id. of the product

	float		stock_qty;			// mined raw materials stock
	float		max_stock_qty;

	float		raw_stock_qty;			// raw materials stock
	float		max_raw_stock_qty;

	float 	cur_month_production;
	float		last_month_production;
	float		production_30days()		{ return last_month_production*(30-info.game_day)/30 +
															cur_month_production; }
	short		next_output_link_id;
	short		next_output_firm_recno;

	int		is_operating()		{ return productivity > 0 && production_30days() > 0; }

	int	   ai_has_excess_worker();

public:
	FirmFactory();
	~FirmFactory();

	void 		init_derived();
	void		draw(int displayLayer=1);

	void 		put_info(int refreshFlag);
	void 		detect_info();

	void		next_day();
	void		next_month();

	void		set_product(int rawId)	{ product_raw_id = rawId; };

	virtual	FirmFactory* cast_to_FirmFactory() { return this; };

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(FirmFactoryCrc *c);

private:
	void		auto_set_product();
	void 		disp_factory_info(int dispY1, int refreshFlag);
	void		change_production();
	void 		set_production(int newProductId);
	void 		set_next_output_firm();
	void 		production();
	void		input_raw();
	void 		manufacture(float maxMftQty);

	//--------------- AI actions ----------------//

	void		process_ai();
	int 		think_build_market();
	int 		think_inc_productivity();
	int 		think_change_production();
};
#pragma pack()

//--------------------------------------//

#endif
