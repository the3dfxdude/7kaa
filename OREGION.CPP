// Filename    : OREGION.CPP
// Description : Class RegionArray , info on regions

#include <stdlib.h>
#include <OINFO.H>
#include <OREGION.H>
#include <OREGIONS.H>

//--------- Define static function ---------//

static int sort_region_function( const void *a, const void *b );

// --------- Begin of function RegionArray::RegionArray -------//
RegionArray::RegionArray()
{
	init_flag = 0;
}
// --------- End of function RegionArray::RegionArray -------//


// --------- Begin of function RegionArray::~RegionArray -------//
RegionArray::~RegionArray()
{
	deinit();
}
// --------- End of function RegionArray::~RegionArray -------//


// --------- Begin of function RegionArray::init -------//
void RegionArray::init(int maxRegion)
{
	deinit();

	region_info_count = maxRegion;

	int connectBit;
	if( maxRegion > 0)
	{
		// --------- allocate memory for RegionInfo --------//
		region_info_array = (RegionInfo *)mem_add( sizeof(RegionInfo) * maxRegion);
		memset(region_info_array, 0, sizeof(RegionInfo) * maxRegion );

		// ---- calculate the no. of bit required to store connection ----//
		connectBit = (maxRegion-1) * (maxRegion) /2;
		// region 1 needs 0 bit
		// region 2 needs 1 bit
		// region 3 needs 2 bits
		// region 4 needs 3 bits...
	}
	else
	{
		region_info_array = NULL;
		connectBit = 0;
	}

	if( connectBit > 0)
	{
		connect_bits = (unsigned char *) mem_add( (connectBit + 7) / 8 );
		memset(connect_bits, 0, (connectBit + 7) /8 );
	}
	else
	{
		connect_bits = NULL;
	}

	//------ initialize adj_offset_bit and area -------//

	int j = 0;

	for(int i=0 ; i<maxRegion ; i++)
	{
		region_info_array[i].region_id = i+1;
		region_info_array[i].adj_offset_bit = j;
		region_info_array[i].region_size = 0;

		j += i;			// j += regionId-1;
	}

	err_when(j != connectBit );

	//------------------------------------------------//

	init_flag = 1;
}
// --------- End of function RegionArray::init -------//


// --------- Begin of function RegionArray::deinit -------//
void RegionArray::deinit()
{
	if( init_flag )
	{
		if(region_info_array)
			mem_del( region_info_array );
		if(region_stat_array)
			mem_del( region_stat_array );
		if(connect_bits)
			mem_del( connect_bits );

		init_flag = 0;
	}
}
// --------- End of function RegionArray::deinit -------//


//--------- Begin of function RegionArray::next_day -------//

void RegionArray::next_day()
{
	if( info.game_date%7 == 0 )
		update_region_stat();
}
//--------- End of function RegionArray::next_day -------//


//--------- Begin of function RegionArray::set_region -------//
void RegionArray::set_region(int reg, RegionType regType)
{
	err_when( reg <= 0 || reg > region_info_count);
	region_info_array[reg-1].region_type = regType;
}
//--------- End of function RegionArray::set_region -------//


//--------- Begin of function RegionArray::inc_size -------//
void RegionArray::inc_size(int reg)
{
	err_when( reg <= 0 || reg > region_info_count);
	region_info_array[reg-1].region_size++;
}
//--------- End of function RegionArray::inc_size -------//


//--------- Begin of function RegionArray::set_adjacent -------//
void RegionArray::set_adjacent(int reg1, int reg2)
{
	err_when( reg1 < 0 || reg1 > region_info_count);
	err_when( reg2 < 0 || reg2 > region_info_count);
	if( reg1 == 0 || reg2 == 0)
		return;

	int bitOffset;
	if( reg1 == reg2 )
		return;
	if( reg1 > reg2)
	{
		bitOffset = region_info_array[reg1 -1].adj_offset_bit + (reg2 -1);
	}
	else
	{
		bitOffset = region_info_array[reg2 -1].adj_offset_bit + (reg1 -1);
	}
	connect_bits[bitOffset / 8] |= 1 << (bitOffset % 8);
}
//--------- End of function RegionArray::set_adjacent -------//


//--------- Begin of function RegionArray::is_adjacent -------//
int RegionArray::is_adjacent(int reg1, int reg2)
{
	err_when( reg1 <= 0 || reg1 > region_info_count);
	err_when( reg2 <= 0 || reg2 > region_info_count);
	int bitOffset;

	if( reg1 == reg2 )
		return TRUE;
	if( reg1 > reg2 )
	{
		bitOffset = region_info_array[reg1 -1].adj_offset_bit + (reg2 -1);
	}
	else
	{
		bitOffset = region_info_array[reg2 -1].adj_offset_bit + (reg1 -1);
	}
	return connect_bits[bitOffset / 8] & (1 << (bitOffset % 8));
}
//--------- End of function RegionArray::is_adjacent -------//


//--------- Begin of function RegionArray::sort_region -------//

void RegionArray::sort_region()
{
	//--- initialize the region_sorted_array first ----//

	for( int i=0 ; i<region_info_count ; i++ )
		region_sorted_array[i] = i+1;

	//----------- sort it now -----------//

	qsort( region_sorted_array, region_info_count, sizeof(region_sorted_array[0]), sort_region_function );
}
//--------- End of function RegionArray::sort_region -------//


//------ Begin of function sort_region_function ------//
//
static int sort_region_function( const void *a, const void *b )
{
	return region_array[*((BYTE*)b)]->region_size - region_array[*((BYTE*)a)]->region_size;
}
//------- End of function sort_region_function ------//

#ifdef DEBUG

//--------- Begin of function RegionArray::get_sorted_region -------//

RegionInfo* RegionArray::get_sorted_region(int recNo)
{
	err_when( recNo<1 || recNo>region_info_count );

	return operator[]( region_sorted_array[recNo-1] );
}
//--------- End of function RegionArray::get_sorted_region -------//


//--------- Begin of function RegionArray::get_region_stat -------//

RegionStat* RegionArray::get_region_stat(int regionId)
{
	err_when( regionId<1 || regionId>region_info_count );

	int regionStatId = region_info_array[regionId-1].region_stat_id;

	err_when( regionStatId<1 || regionStatId>region_stat_count );

	return region_stat_array+regionStatId-1;
}
//--------- End of function RegionArray::get_region_stat -------//


//--------- Begin of function RegionArray::get_region_stat2 -------//

RegionStat* RegionArray::get_region_stat2(int regionStatId)
{
	err_when( regionStatId<1 || regionStatId>region_stat_count );

	return region_stat_array+regionStatId-1;
}
//--------- End of function RegionArray::get_region_stat2 -------//


// --------- Begin of function RegionArray::region_type -------//
RegionType RegionArray::region_type(int region)
{
	err_when(region <= 0 || region > region_info_count);
	return region_info_array[region-1].region_type;
}
// --------- End of function RegionArray::region_type -------//


//--------- Begin of function RegionArray::operator[] -------//
RegionInfo *RegionArray::operator[](int region)
{
	err_when(region <= 0 || region > region_info_count);

	return region_info_array+region-1;
}
//--------- End of function RegionArray::operator[] -------//

#endif
