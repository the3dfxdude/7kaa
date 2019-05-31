/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2018 Jesse Allen
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

//Filename    : ReplayFile.cpp
//Description : Replay File IO

#include <string.h>

#include <ReplayFile.h>
#include <OCONFIG.h>
#include <OERROR.h>
#include <ONATIONA.h>
#include <OREMOTEQ.h>
#include <version.h>
#include <ConfigAdv.h>
#include <OBOX.h>
#include <gettext.h>

const char file_magic[] = "7KRP";

const int32_t replay_version = 1;
// version 1 adds ver_cksum

struct GameVer {
	uint32_t ver1;
	uint32_t ver2;
	uint32_t ver3;
	uint32_t flags;

	void set_current_version()
	{
		ver1 = SKVERMAJ;
		ver2 = SKVERMED;
		ver3 = SKVERMIN;
		flags = config_adv.flags;
	}

	int cmp(GameVer *a)
	{
		return ver1 == a->ver1 &&
			ver2 == a->ver2 &&
			ver3 == a->ver3 &&
			flags == a->flags;
	}
};

ReplayFile::ReplayFile()
{
	file_size = 0;
	mode = ReplayFile::DISABLE;
}

ReplayFile::~ReplayFile()
{
}

int ReplayFile::at_eof()
{
	return mode != ReplayFile::READ || file.file_pos() >= file_size;
}

void ReplayFile::close()
{
	if( mode == ReplayFile::DISABLE )
		return;
	file.file_close();
	file_size = 0;
	mode = ReplayFile::DISABLE;
}

int ReplayFile::open_read(const char* filePath, NewNationPara *mpGame, int *mpPlayerCount)
{
	if( mode != ReplayFile::DISABLE )
		return 0;

	GameVer current_version;
	uint32_t ver_cksum;
	char magic[4];
	int32_t file_version;
	GameVer version;
	current_version.set_current_version();
	int random_seed;

	if( !file.file_open(filePath, 0) )
		return 0;
	if( !file.file_read(&magic, 4) )
		goto out;
	if( memcmp(magic, file_magic, 4) )
		goto out;
	file_version = file.file_get_long();
	if( file_version != replay_version )
	{
		box.msg(_("The selected reply file uses an unsupported format"));
		goto out;
	}
	if( !file.file_read(&version, sizeof(GameVer)) )
		goto out;
	ver_cksum = file.file_get_long();
	if( !current_version.cmp(&version) || ver_cksum != config_adv.checksum )
	{
		String msg;
		sprintf(msg, _("Replay version %u.%u.%u.%u.%u mismatches with current game version %u.%u.%u.%u.%u"), version.ver1, version.ver2, version.ver3, version.flags, ver_cksum, current_version.ver1, current_version.ver2, current_version.ver3, current_version.flags, config_adv.checksum);
		box.msg(msg);
	}
	random_seed = file.file_get_long();
	if( !config.read_file(&file, 1) ) // 1-keep system settings
		goto out;
	*mpPlayerCount = file.file_get_short();
	for( int i = 0; i < *mpPlayerCount; ++i )
	{
		mpGame[i].nation_recno = file.file_get_short();
		mpGame[i].dp_player_id = 0;
		mpGame[i].color_scheme = file.file_get_short();
		mpGame[i].race_id      = file.file_get_short();
		file.file_read(&mpGame[i].player_name, HUMAN_NAME_LEN+1);
	}

	info.init_random_seed(random_seed);

	file_size = file.file_size();

	mode = ReplayFile::READ;
	return 1;
out:
	file.file_close();
	return 0;
}

int ReplayFile::open_write(const char* filePath, NewNationPara *mpGame, int mpPlayerCount)
{
	if( mode != ReplayFile::DISABLE )
		return 0;

	GameVer current_version;
	current_version.set_current_version();

	if( !file.file_create(filePath, 0) )
		return 0;
	file.file_write((void *)file_magic, 4);
	file.file_put_long(replay_version);
	file.file_write(&current_version, sizeof(GameVer));
	file.file_put_long(config_adv.checksum);
	file.file_put_long(info.random_seed);
	config.write_file(&file);
	file.file_put_short(mpPlayerCount);
	for( int i = 0; i < mpPlayerCount; ++i )
	{
		file.file_put_short(mpGame[i].nation_recno);
		file.file_put_short(mpGame[i].color_scheme);
		file.file_put_short(mpGame[i].race_id);
		file.file_write(&mpGame[i].player_name, HUMAN_NAME_LEN+1);
	}

	mode = ReplayFile::WRITE;
	return 1;
}

// returns number of bytes read into queue
int ReplayFile::read_queue(RemoteQueue *rq)
{
	if( at_eof() )
		return 0;
	int size = file.file_get_unsigned_short();
	if( size <= 0 )
		return 0;
	if( size > rq->queue_buf_size )
		rq->reserve(size - rq->queue_buf_size);
	file.file_read(rq->queue_buf, size);
	rq->queue_ptr = rq->queue_buf + size;
        rq->queued_size = size;
	return size;
}

void ReplayFile::write_queue(RemoteQueue *rq)
{
	if( mode != ReplayFile::WRITE )
		return;
	if( rq->queued_size <= 0 )
		return;
	file.file_put_unsigned_short(rq->queued_size);
	file.file_write(rq->queue_buf, rq->queued_size);
}
