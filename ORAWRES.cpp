//Filename    : ORAWRES.CPP
//Description : Raw material resource object

#include <OINFO.h>
#include <OVGA.h>
#include <OHELP.h>
#include <OFONT.h>
#include <OSYS.h>
#include <OGAMESET.h>
#include <OF_FACT.h>
#include <OF_MINE.h>
#include <OF_MARK.h>
#include <ORAWRES.h>


//---------- #define constant ------------//

#define RAW_DB   "RAW"

//------- Begin of function RawRes::RawRes -----------//

RawRes::RawRes()
{
	init_flag=0;
}

//--------- End of function RawRes::RawRes -----------//


//---------- Begin of function RawRes::init -----------//
//
// This function must be called after a map is generated.
//
void RawRes::init()
{
	deinit();

	//----- open unit bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_RAW.RES";

	res_icon.init(str, 1);  // 1-read all into buffer

	//------- load database information --------//

	load_all_info();

	init_flag=1;
}
//---------- End of function RawRes::init -----------//


//---------- Begin of function RawRes::deinit -----------//

void RawRes::deinit()
{
	if( init_flag )
	{
		delete [] raw_info_array;
		init_flag=0;
   }
}
//---------- End of function RawRes::deinit -----------//


//------- Begin of function RawRes::load_all_info -------//
//
// Read in information of RAW.DBF into memory array
//
void RawRes::load_all_info()
{
   RawRec  *rawRec;
   RawInfo *rawInfo;
	int      i;
	Database *dbRaw = game_set.open_db(RAW_DB);

	raw_count = (short) dbRaw->rec_count();
	raw_info_array = new RawInfo[raw_count];

	//------ read in raw information array -------//

	for( i=0 ; i<raw_count ; i++ )
	{
		rawRec  = (RawRec*) dbRaw->read(i+1);
		rawInfo = raw_info_array+i;

		m.rtrim_fld( rawInfo->name, rawRec->name, rawRec->NAME_LEN );
#if(defined(GERMAN) || defined(FRENCH) || defined(SPANISH))
		translate.multi_to_win(rawInfo->name, rawInfo->NAME_LEN);
#endif
		rawInfo->raw_id    = i+1;
		rawInfo->tera_type = m.atoi( rawRec->tera_type, rawRec->TERA_TYPE_LEN );
	}
}
//--------- End of function RawRes::load_all_info ---------//


//---------- Begin of function RawRes::next_day -----------//

void RawRes::next_day()
{
	if( info.game_date%15==0 )
		update_supply_firm();
}
//---------- End of function RawRes::next_day -----------//


//------- Begin of function RawRes::update_supply_firm -------//

void RawRes::update_supply_firm()
{
	//----- reset the supply array of each raw and product ----//

	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		raw_info_array[i].raw_supply_firm_array.zap();
		raw_info_array[i].product_supply_firm_array.zap();
	}

	//---- locate for suppliers that supply the products needed ----//

	Firm* 		 firmPtr;
	FirmMine* 	 firmMine;
	FirmFactory* firmFactory;
	FirmMarket*  firmMarket;

	for( short firmRecno=firm_array.size() ; firmRecno>0 ; firmRecno-- )
	{
		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = (Firm*) firm_array[firmRecno];

		//-------- factory as a potential supplier ------//

		if( firmPtr->firm_id == FIRM_FACTORY )
		{
			firmFactory = (FirmFactory*) firmPtr;

			if( firmFactory->product_raw_id &&
				 firmFactory->stock_qty > firmFactory->max_stock_qty / 5 )
			{
				raw_res[firmFactory->product_raw_id]->add_product_supply_firm(firmRecno);
			}
		}

		//-------- mine as a potential supplier ------//

		if( firmPtr->firm_id == FIRM_MINE )
		{
			firmMine = (FirmMine*) firmPtr;

			if( firmMine->raw_id &&
				 firmMine->stock_qty > firmMine->max_stock_qty / 5 )
			{
				raw_res[firmMine->raw_id]->add_raw_supply_firm(firmRecno);
			}
		}

		//-------- market place as a potential supplier ------//

		else if( firmPtr->firm_id == FIRM_MARKET )
		{
			firmMarket = (FirmMarket*) firmPtr;

			MarketGoods* marketGoods = firmMarket->market_goods_array;

			for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
			{
				if( marketGoods->stock_qty > MAX_MARKET_STOCK / 5 )
				{
					if( marketGoods->product_raw_id )
						raw_res[marketGoods->product_raw_id]->add_product_supply_firm(firmRecno);

					else if( marketGoods->raw_id )
						raw_res[marketGoods->raw_id]->add_raw_supply_firm(firmRecno);
				}
			}
		}
	}
}
//-------- End of function RawRes::update_supply_firm --------//


//------- Begin of function RawInfo::RawInfo -----------//

RawInfo::RawInfo() : raw_supply_firm_array(sizeof(short), 70), product_supply_firm_array(sizeof(short), 70)
{
}

//--------- End of function RawInfo::RawInfo -----------//


//------- Begin of function RawInfo::add_raw_supply_firm -----------//

void RawInfo::add_raw_supply_firm(short firmRecno)
{
	err_when( firm_array.is_deleted(firmRecno) );

	raw_supply_firm_array.linkin(&firmRecno);
}

//--------- End of function RawInfo::add_raw_supply_firm --------//


//------- Begin of function RawInfo::add_product_supply_firm -----------//

void RawInfo::add_product_supply_firm(short firmRecno)
{
	err_when( firm_array.is_deleted(firmRecno) );

	product_supply_firm_array.linkin(&firmRecno);
}

//--------- End of function RawInfo::add_product_supply_firm --------//


//---------- Begin of function RawRes::operator[] -----------//

RawInfo* RawRes::operator[](int rawId)
{
	err_if( rawId<1 || rawId>raw_count )
		err_now( "RawRes::operator[]" );

	return raw_info_array+rawId-1;
}

//------------ End of function RawRes::operator[] -----------//


//---------- Begin of function RawRes::put_small_raw_icon -----------//

void RawRes::put_small_raw_icon(int x, int y, int rawId)
{
	char* bitmapPtr = res_icon.read(MAX_RAW*3+rawId);

	Vga::active_buf->put_bitmap_trans(x, y, bitmapPtr);

	help.set_custom_help( x, y, x+RAW_SMALL_ICON_WIDTH-1, y+RAW_SMALL_ICON_HEIGHT-1,
								 raw_res[rawId]->name );
}
//---------- End of function RawRes::put_small_raw_icon -----------//


//---------- Begin of function RawRes::put_small_product_icon -----------//

void RawRes::put_small_product_icon(int x, int y, int rawId)
{
	char* bitmapPtr = res_icon.read(MAX_RAW+rawId);

	Vga::active_buf->put_bitmap_trans(x, y, bitmapPtr);

	String str;

#if(defined(FRENCH))
	char productName[20];
	strcpy(productName, raw_res[rawId]->name);
	strcat(productName, " Products");
	str  = translate.process(productName);
#else
	str  = raw_res[rawId]->name;
	str += translate.process(" Products");
#endif

	help.set_custom_help( x, y, x+RAW_SMALL_ICON_WIDTH-1, y+RAW_SMALL_ICON_HEIGHT-1, str );
}
//---------- End of function RawRes::put_small_product_icon -----------//

