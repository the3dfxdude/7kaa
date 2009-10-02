//Filename    : OF_FACT.H
//Description : Header of FirmFactory

#ifndef __OF_FACT_H
#define __OF_FACT_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

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
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

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
