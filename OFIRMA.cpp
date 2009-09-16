//Filename    : OFIRMA.CPP
//Description : Object Firm Array

#include <OVGA.H>
#include <OWORLD.H>
#include <OPOWER.H>
#include <ONATION.H>
#include <OSYS.H>
#include <OGAME.H>
#include <OTOWN.H>
#include <OINFO.H>
#include <OFIRMRES.H>
#include <OFIRMALL.H>
#include <OSERES.H>
#include <OLOG.H>

//### begin alex 22/9 ###//
#ifdef DEBUG
#include <OFONT.H>

static unsigned long	last_firm_ai_profile_time = 0L;
static unsigned long	firm_ai_profile_time = 0L;
static unsigned long	last_firm_profile_time = 0L;
static unsigned long	firm_profile_time = 0L;
#endif
//#### end alex 22/9 ####//

//----------- Note ----------------//
//
// go(), start(), end(), fwd(), bkwd() will affect
// the current record no. pointer which will be used by other function
//
// To process all elements in the array, get(recNo) should be used.
//
// Because the sizeof different derived Firm objects are quite largely
// different, it is better to link in pointer instead of the object
// body.
//
// The DynArrayB contain the pointer to Firm objects.
//
//---------------------------------//


//--------- Begin of function FirmArray::FirmArray ---------//
//
FirmArray::FirmArray() : DynArrayB(sizeof(Firm*), 50, DEFAULT_REUSE_INTERVAL_DAYS)
{
}
//----------- End of function FirmArray::FirmArray ---------//


//--------- Begin of function FirmArray::~FirmArray ---------//
//
FirmArray::~FirmArray()
{
   deinit();
}
//----------- End of function FirmArray::~FirmArray ---------//


//--------- Begin of function FirmArray::init ---------//

void FirmArray::init()
{
	process_recno = 1;
}
//---------- End of function FirmArray::init ---------//


//--------- Begin of function FirmArray::deinit ---------//
//
// All firms should be deleted when the system terminated
//
// Chain : delete nation --> delete firm --> delete job --> delete item
//
// Some data is corrupted when the some firms still exist when
// the system terminated
//
void FirmArray::deinit()
{
	if( size()==0 )
		return;

	//----------------------------------//

	int i;

	for( i=1 ; i<=size() ; i++ )
	{
		if( !firm_array.is_deleted(i) )
			firm_array.del_firm(i);
	}

	//----------------------------------//

	zap();       // zap the DynArrayB
}
//----------- End of function FirmArray::deinit ---------//


//--------- Begin of function FirmArray::build_firm ---------//
//
// build_firm() will be called by Nation and FirmRes when setting up
// new firm.
//
// <int> xLoc        = the x location of the firm to be built
// <int> yLoc        = the y location of the firm to be built
// <int> nationRecno = the nation which builds this firm
// <int> firmId      = firm type id.
// [char*] buildCode = the build code of the firm, no need to give if the firm just have one build type
// [short] builderRecno = recno of the builder unit
//
// Return : <int> the record no. of the newly added firm
//
int FirmArray::build_firm(int xLoc, int yLoc, int nationRecno, int firmId, char* buildCode, short builderRecno)
{
	if( !world.can_build_firm(xLoc, yLoc, firmId) )
		return 0;

	//--------- check if there is enough cash ----------//

	if( nationRecno )
	{
		FirmInfo* firmInfo  = firm_res[firmId];
		Nation*   nationPtr = nation_array[nationRecno];

		if( nationRecno )
		{
			if( nationPtr->cash < firmInfo->setup_cost )
				return 0;
		}
	}

	//---------- create and build the firm -------------//

	int firmRecno = create_firm(firmId);

	firm_array[firmRecno]->init( xLoc, yLoc, nationRecno, firmId, buildCode, builderRecno);

	// Firm::init() will set world matrix, it will use Firm::firm_recno to set the location cargo

	//------ pay the land cost to the nation that owns the land ------//

	if( nationRecno )
	{
		Nation* nationPtr = nation_array[nationRecno];

		nationPtr->add_expense( EXPENSE_FIRM, (float)firm_res[firmId]->setup_cost);		// setup cost of the firm
	}

	return firmRecno;
}
//---------- End of function FirmArray::build_firm ---------//


//--------- Begin of function FirmArray::create_firm ---------//
//
// create_firm() will be called directly by :
//
// 1. FirmArray::build_firm()  for setting up a new firm
// 2. FirmArray::read_file() when loading game.
//
// <int> firmId = firm type id
//
// Return : <int> the record no. of the newly added firm
//
int FirmArray::create_firm(int firmId)
{
	Firm* firmPtr;

	switch(firmId)
	{
		case FIRM_BASE:
			firmPtr = new FirmBase;
			break;

		case FIRM_CAMP:
			firmPtr = new FirmCamp;
			break;

		case FIRM_FACTORY:
			firmPtr = new FirmFactory;
			break;

		case FIRM_INN:
			firmPtr = new FirmInn;
			break;

		case FIRM_MARKET:
			firmPtr = new FirmMarket;
			break;

		case FIRM_MINE:
			firmPtr = new FirmMine;
			break;

		case FIRM_RESEARCH:
			firmPtr = new FirmResearch;
			break;

		case FIRM_WAR_FACTORY:
			firmPtr = new FirmWar;
			break;

		case FIRM_HARBOR:
			firmPtr = new FirmHarbor;
			break;

		case FIRM_MONSTER:
			firmPtr = new FirmMonster;
			break;

		default:
			err_now("FirmArray::create_firm()");
	}

	//----------------------------------------//

	linkin(&firmPtr);
	firmPtr->firm_recno = recno();

	return firmPtr->firm_recno;
}
//----------- End of function FirmArray::create_firm ---------//


//--------- Begin of function FirmArray::firm_class_size ---------//
//
// Return the size of the specified class.
// This function will be called by FirmArray::write_file()
//
// <int> id    = the id of the job
//
int FirmArray::firm_class_size(int id)
{
	switch(id)
	{
		case FIRM_BASE:
			return sizeof(FirmBase);

		case FIRM_CAMP:
			return sizeof(FirmCamp);

		case FIRM_FACTORY:
			return sizeof(FirmFactory);

		case FIRM_INN:
			return sizeof(FirmInn);

		case FIRM_MARKET:
			return sizeof(FirmMarket);

		case FIRM_MINE:
			return sizeof(FirmMine);

		case FIRM_RESEARCH:
			return sizeof(FirmResearch);

		case FIRM_WAR_FACTORY:
			return sizeof(FirmWar);

		case FIRM_HARBOR:
			return sizeof(FirmHarbor);

		case FIRM_MONSTER:
			return sizeof(FirmMonster);

		default:
			err_now( "FirmArray::firm_class_size" );
	}

	return 0;
}
//----------- End of function FirmArray::firm_class_size ---------//


//--------- Begin of function FirmArray::del_firm ---------//
//
// Warning : After calling this function, the recno() is still
//           pointing to the deleted record.
//           So go() to a new record to prevent running NULL object
//
// <int> recNo = the no. of the record to be deleted
//               (default : current record no.)
//
void FirmArray::del_firm(int recNo)
{
   Firm* firmPtr = firm_array[recNo];

	int	xLoc = firmPtr->center_x;
	int	yLoc = firmPtr->center_y;

	firmPtr->deinit();   // we must call deinit() first

	delete firmPtr;

	linkout(recNo);
}
//----------- End of function FirmArray::del_firm ---------//


//--------- Begin of function FirmArray::process ---------//
//
// Process all firm in firm_array for action and movement for next frame
//
// Return : 1 - all firm in the FirmArray has been processed
//          0 - only some has been processed, not all
//
int FirmArray::process()
{
   int  i;
	Firm *firmPtr;

	//----- each time process some firm only ------//

	for( i=1 ; i<=size() ; i++ )
	{
		firmPtr = (Firm*) get_ptr(i);

		if( !firmPtr )    // the firm has been deleted
			continue;

		err_when(firmPtr->firm_recno!=i);

		//-------- system yield ---------//

		if( i%20==1 )
			sys.yield();

#if defined(DEBUG) && defined(ENABLE_LOG)
		String logStr;
		logStr = "begin process firm ";
		logStr += firmPtr->firm_recno;
		logStr += " nation=";
		logStr += firmPtr->nation_recno;
		LOG_MSG(logStr);
#endif
	
		if(i==50)
		{

			FirmMarket *mPtr = (FirmMarket*) firmPtr;
			MarketGoods *marketGoods = mPtr->market_goods_array;
			marketGoods++;

			if(marketGoods->stock_qty)
				int debug = 0;
		}


		//-------- process visibility -----------//

		if( firmPtr->nation_recno == nation_array.player_recno ||
			 ( firmPtr->nation_recno &&
				nation_array[firmPtr->nation_recno]->is_allied_with_player ) )
		{
			world.visit( firmPtr->loc_x1, firmPtr->loc_y1, firmPtr->loc_x2, firmPtr->loc_y2, EXPLORE_RANGE-1 );
		}

		//--------- process and process_ai firms ----------//

		if( firmPtr->under_construction )
		{
			LOG_MSG(" process_construction");
			firmPtr->process_construction();
			LOG_MSG(m.get_random_seed() );
		}
		else
		{
			if( i%FRAMES_PER_DAY == int(sys.frame_count%FRAMES_PER_DAY) )	// only process each firm once per day
			{
				//### begin alex 22/9 ###//
				#ifdef DEBUG
				unsigned long profileStartTime = m.get_time();
				#endif
				//#### end alex 22/9 ####//

				LOG_MSG(" next_day");
				firmPtr->next_day();
				LOG_MSG(m.get_random_seed() );

				//### begin alex 22/9 ###//
				#ifdef DEBUG
				firm_profile_time += m.get_time() - profileStartTime;
				#endif
				//#### end alex 22/9 ####//

				//-- if the hit points drop to zero, the firm should be deleted --//

				if( firmPtr->hit_points <=0 )
				{
					se_res.sound( firmPtr->center_x, firmPtr->center_y, 1, 'F', firmPtr->firm_id, "DEST" );
					del_firm( firmPtr->firm_recno );
					continue;
				}

				//--------- process AI ------------//

				#ifdef DEBUG
				if(config.disable_ai_flag==0 && firmPtr->firm_ai)
				#else
				if( firmPtr->firm_ai )
				#endif
				{
					LOG_MSG(" process_common_ai");
					firmPtr->process_common_ai();
					LOG_MSG(m.get_random_seed() );

					//### begin alex 22/9 ###//
					#ifdef DEBUG
					unsigned long profileAiStartTime = m.get_time();
					#endif
					//#### end alex 22/9 ####//

					LOG_MSG(" process_ai");
					firmPtr->process_ai();
					LOG_MSG(m.get_random_seed());

					//### begin alex 22/9 ###//
					#ifdef DEBUG
					firm_ai_profile_time += m.get_time() - profileAiStartTime;
					#endif
					//#### end alex 22/9 ####//

					if( is_deleted(i) )		// the firm may have been deleted in process_ai()
						continue;
				}

				//--- think about having other nations capturing this firm ----//

				if( info.game_date%60==i%60 )		// this is not limited to ai firms only, it is called on all firms as AI can capture other player's firm
					firmPtr->think_capture();
			}
		}

		//-------- process animation ---------//

		LOG_MSG(" process_animation");
		firmPtr->process_animation();
		LOG_MSG( m.get_random_seed() );
	}

	return 0;
}
//----------- End of function FirmArray::process ---------//


//--------- Begin of function FirmArray::next_month ---------//
//
void FirmArray::next_month()
{
	int	 i;
	Firm*  firmPtr;

	LOG_MSG("begin FirmArray::next_month");
	LOG_MSG(m.get_random_seed() );
	for(i=1; i <=size() ; i++)
	{
		firmPtr = (Firm*)get_ptr(i);

		if( firmPtr && !firmPtr->under_construction )
		{
			LOG_MSG("Firm next_month");
			LOG_MSG( i );
			firmPtr->next_month();
			LOG_MSG(m.get_random_seed() );
		}
	}
	LOG_MSG("end FirmArray::next_month");
	LOG_MSG(m.get_random_seed() );
}
//----------- End of function FirmArray::next_month -----------//


//--------- Begin of function FirmArray::next_year ---------//
//
void FirmArray::next_year()
{
	int	 i;
	Firm*  firmPtr;

	LOG_MSG("begin FirmArray::next_year");
	LOG_MSG(m.get_random_seed() );
	for(i=1; i <=size() ; i++)
	{
		firmPtr = (Firm*)get_ptr(i);

		if( firmPtr && !firmPtr->under_construction )
		{
			LOG_MSG("Firm next_month");
			LOG_MSG( i );
			firmPtr->next_year();
			LOG_MSG(m.get_random_seed() );
		}
	}
	LOG_MSG("end FirmArray::next_year");
	LOG_MSG(m.get_random_seed() );
}
//----------- End of function FirmArray::next_year -----------//


//--------- Begin of function FirmArray::draw_dot ---------//
//
// Draw tiny dots on map window representing the location of the firm
//
void FirmArray::draw_dot()
{
	char*	  		vgaBufPtr = vga_back.buf_ptr();
	char*			writePtr;
	int	  		i, x, y;
	Firm*   		firmPtr;
	FirmBuild*  firmBuild;
	char*   	   nationColorArray = nation_array.nation_color_array;
	char	  		nationColor;
	int			vgaBufPitch = vga_back.buf_pitch();
	const excitedColorCount = 4;
	char excitedColorArray[MAX_NATION+1][excitedColorCount];
	for( i = 0; i <= MAX_NATION; ++i )
	{
		if( i == 0 || !nation_array.is_deleted(i) )
		{
			char *remapTable = game.get_color_remap_table(i, 0);
			excitedColorArray[i][0] = remapTable[0xe0];
			excitedColorArray[i][1] = remapTable[0xe1];
			excitedColorArray[i][2] = remapTable[0xe2];
			excitedColorArray[i][3] = remapTable[0xe3];
		}
		else
		{
			excitedColorArray[i][0] = 
			excitedColorArray[i][1] = 
			excitedColorArray[i][2] = 
			excitedColorArray[i][3] = (char) V_WHITE;
		}
	}

	for(i=1; i <=size() ; i++)
	{
		firmPtr = (Firm*)get_ptr(i);

		if( !firmPtr )
			continue;

		firmBuild = firm_res.get_build(firmPtr->firm_build_id);

		writePtr = vgaBufPtr + (MAP_Y1+firmPtr->loc_y1)*vgaBufPitch + (MAP_X1+firmPtr->loc_x1);

		nationColor = info.game_date - firmPtr->last_attacked_date > 2 ?
			nationColorArray[firmPtr->nation_recno] :
			excitedColorArray[firmPtr->nation_recno][sys.frame_count % excitedColorCount];

		char shadowColor = (char) VGA_GRAY;

		// ###### begin Gilbert 17/10 #######//
		int	firmWidth = firmBuild->loc_width;
		int	firmHeight = firmBuild->loc_height;
		if( firmPtr->nation_recno == 0 &&
			firmWidth == STD_TOWN_LOC_WIDTH && firmHeight == STD_TOWN_LOC_HEIGHT)
		{
			--firmWidth;		// all monster are all 4x4, may mixed with independent town on mini-map
			--firmHeight;
		}

		for( y=firmHeight ; y>0 ; y--, writePtr+=vgaBufPitch-firmWidth )
		{
			for( x=firmWidth ; x>0 ; x--, writePtr++ )
			{
				if( *writePtr != UNEXPLORED_COLOR )
					*writePtr = nationColor;
			}

			if( *(writePtr+vgaBufPitch) != UNEXPLORED_COLOR)
				*(writePtr+vgaBufPitch) = shadowColor;
		}
		for( x = firmWidth; x>0; x--)
		{
			if( *(++writePtr) != UNEXPLORED_COLOR )
				*writePtr = shadowColor;
		}
		// ###### end Gilbert 17/10 #######//
	}
}
//----------- End of function FirmArray::draw_dot -----------//

//### begin alex 22/9 ###//
//--------- Begin of function FirmArray::draw_profile ---------//
void FirmArray::draw_profile()
{
#ifdef DEBUG	
	static unsigned long lastDrawTime = m.get_time();

	if(m.get_time() >= lastDrawTime + 1000)
	{
		last_firm_profile_time = firm_profile_time;
		lastDrawTime = m.get_time();
		firm_profile_time = 0L;
	}

	String str;
	str  = "Firm  : ";
	font_news.disp( ZOOM_X1+10, ZOOM_Y1+60, str, MAP_X2);

	str = "";
	str += last_firm_ai_profile_time;
	font_news.disp( ZOOM_X1+60, ZOOM_Y1+60, str, MAP_X2);

	str = "";
	str += last_firm_profile_time;
	font_news.disp( ZOOM_X1+100, ZOOM_Y1+60, str, MAP_X2);
#endif	
}
//----------- End of function FirmArray::draw_profile -----------//
//#### end alex 22/9 ####//

/*

//--------- Begin of function FirmArray::skip ---------//
//
// Skip to prev/next firms within the firm filter.
//
// <int> skipDirection = -1 - backward one record
//                        1 - forward one record
//
void FirmArray::skip(int skipDirection)
{
	int  firmRecno=recno();
	Firm *firmPtr;

	while(1)
	{
      firmRecno += ( skipDirection>0 ? 1 : -1 );

      //------- loop to front/back --------//

      if( firmRecno > size() )
         firmRecno = firmRecno - size();

      if( firmRecno < 1 )
         firmRecno = firmRecno + size();

      //----- if no firm under current firm filter ----//

      if( firmRecno == recno() )   // after one loop, there is still no match
         return;

      //--------- if it is deleted --------//

      if( is_deleted(firmRecno) )
         continue;

      //------ if it is not in firm filter -------//

      if( !firm_array[firmRecno]->draw_map_loc(2) )     //only draw when the firm is in filter
			continue;

		//--------- go to the firm ----------//

      world.go_firm(firmRecno);
		return;
   }
}
//---------- End of function FirmArray::skip ----------//

*/

#ifdef DEBUG

//------- Begin of function FirmArray::operator() -----//

Firm* FirmArray::operator()()
{
   Firm* firmPtr = (Firm*) get_ptr();   // if recno()==0, get_ptr() returns NULL

	err_if( !firmPtr )
      err_now( "FirmArray[recno()] is deleted" );

   return firmPtr;
}

//--------- End of function FirmArray::operator() ----//


//------- Begin of function FirmArray::operator[] -----//

Firm* FirmArray::operator[](int recNo)
{
   Firm* firmPtr = (Firm*) get_ptr(recNo);

   if( !firmPtr )
      err.run( "FirmArray[] is deleted" );

   return firmPtr;
}

//--------- End of function FirmArray::operator[] ----//

#endif


