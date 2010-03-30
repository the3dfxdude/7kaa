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

#ifdef DEBUG
#include <OSYS.h>
#include <OMISC.h>
#include <OMOUSE.h>
#include <OMOUSE2.h>

#define	MAX_SEED_TABLE_SIZE	10 * 4000
static long	random_seed_backup_table[MAX_SEED_TABLE_SIZE];
static int	random_seed_writen_pos;
static int	random_seed_backup_pos;
static int	random_seed_backup_table_data_size;
static char file_opened_flag = 0;
static long	match_seed;


//-------- Begin of function Sys::sp_create_seed_file --------//
void Sys::sp_create_seed_file(char *filename)
{
	seedCompareFile.file_create(filename);
	//debug_seed_status_flag = 2; // 1 for saving, 2 for comparing
	//random_seed_backup_pos = 0;
	random_seed_writen_pos = 0;

	file_opened_flag = 1;
}
//--------- End of function Sys::sp_create_seed_file ---------//


//-------- Begin of function Sys::sp_close_seed_file --------//
void Sys::sp_close_seed_file()
{
	seedCompareFile.file_close();
	//debug_seed_status_flag = NO_DEBUG_SYN;
	//random_seed_backup_pos = 0;

	file_opened_flag = 0;
	sp_seed_pos_reset();
}
//--------- End of function Sys::sp_close_seed_file ---------//


//-------- Begin of function Sys::sp_load_seed_file --------//
void Sys::sp_load_seed_file()
{
	//debug_seed_status_flag = NO_DEBUG_SYN;

	File headerFile;
	headerFile.file_open("nhead.rs");

	long firstSeed;
	if(headerFile.file_size() >= sizeof(long)*2)
	{
		firstSeed = headerFile.file_get_long();
		if(match_seed) // internal loading
		{
			// do nothing
			int debug = 0;
		}
		else // load from user action
		{
			if(m.get_random_seed() != firstSeed)
				return; // seed file don't match the save game
		}
	}
	else
		return; // seed file don't match the save game

	if(!seedCompareFile.file_open("nseed.rs"))
		return;
	
	random_seed_backup_pos = 0;
	//debug_seed_status_flag = DEBUG_SYN_AUTO_LOAD;

	random_seed_backup_table_data_size = headerFile.file_get_long(); // read the table size
	headerFile.file_close();

	//------------ read the table content ------------//
	for(int i=0; i<random_seed_backup_table_data_size; i++)
		random_seed_backup_table[i] = seedCompareFile.file_get_long();

	file_opened_flag = 1;
}
//--------- End of function Sys::sp_load_seed_file ---------//


//-------- Begin of function Sys::sp_record_match_seed --------//
void Sys::sp_record_match_seed()
{
	match_seed = m.get_random_seed();
}
//--------- End of function Sys::sp_record_match_seed ---------//


//-------- Begin of function Sys::sp_record_seed --------//
void Sys::sp_record_seed()
{
	random_seed_backup_table[random_seed_backup_pos] = m.get_random_seed();
	random_seed_backup_pos++;
}
//--------- End of function Sys::sp_record_seed ---------//


//-------- Begin of function Sys::sp_write_seed --------//
void Sys::sp_write_seed()
{
	if(random_seed_backup_pos==0)
		return;

	if(file_opened_flag==0)
		return;

	/*
	seedCompareFile.file_put_long(match_seed);
	seedCompareFile.file_put_long(random_seed_backup_pos);

	for(int i=0; i<random_seed_backup_pos; i++)
		seedCompareFile.file_put_long(random_seed_backup_table[i]);
	*/
	
	File headerFile;
	headerFile.file_create("nhead.rs");
	headerFile.file_put_long(match_seed);
	headerFile.file_put_long(random_seed_writen_pos + random_seed_backup_pos);
	headerFile.file_close();

	if(random_seed_writen_pos==0)
	{
		for(int i=0; i<random_seed_backup_pos; i++)
			seedCompareFile.file_put_long(random_seed_backup_table[i]);
	}
	else
	{
		seedCompareFile.file_seek(0L, FILE_END);
		for(int i=0; i<random_seed_backup_pos; i++)
			seedCompareFile.file_put_long(random_seed_backup_table[i]);
	}

	random_seed_writen_pos += random_seed_backup_pos;
	random_seed_backup_pos = 0;
}
//--------- End of function Sys::sp_write_seed ---------//


//-------- Begin of function Sys::sp_compare_seed --------//
void Sys::sp_compare_seed()
{
	if(file_opened_flag==0)
		return;

	long	gameSeed, saveSeed;
	
	gameSeed = m.get_random_seed();
	saveSeed = random_seed_backup_table[random_seed_backup_pos];

	if(random_seed_backup_pos>130)
		int debug = 0;

	if( gameSeed != saveSeed )
		err.run( "Error: random seeds not sync." );

	random_seed_backup_pos++;

	if(random_seed_backup_pos>=random_seed_backup_table_data_size)
	{
		sp_close_seed_file();
		//debug_seed_status_flag = 0;
		if(debug_seed_status_flag==DEBUG_SYN_AUTO_LOAD)
			//debug_seed_status_flag = DENUG_SYN_AUTO_SAVE;
			mouse.add_key_event(DIK_LBRACKET, m.get_time()); // save seed for comparison
		else
			debug_seed_status_flag = NO_DEBUG_SYN;
	}
}
//--------- End of function Sys::sp_compare_seed ---------//


//-------- Begin of function Sys::sp_seed_pos_reset --------//
void Sys::sp_seed_pos_reset()
{
	random_seed_backup_pos = 0;
}
//--------- End of function Sys::sp_seed_pos_reset ---------//


//-------- Begin of function Sys::sp_seed_pos_set --------//
void Sys::sp_seed_pos_set(int pos)
{
	random_seed_backup_pos = pos;
}
//--------- End of function Sys::sp_seed_pos_set ---------//
#endif
