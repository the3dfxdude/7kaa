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
#include <version.h>

const char file_magic[] = "7KRP";
const int32_t replay_version = 0;
struct GameVer {
	uint32_t ver1;
	uint32_t ver2;
	uint32_t ver3;
	uint32_t build_flags;

	void set_current_version()
	{
		ver1 = SKVERMAJ;
		ver2 = SKVERMED;
		ver3 = SKVERMIN;
		build_flags = 0;
#ifdef DEBUG
		build_flags |= 0x00000001;
#endif
#ifdef DEV_VERSION
		build_flags |= 0x00000002;
#endif
	}

	int cmp(GameVer *a)
	{
		return ver1 == a->ver1 &&
			ver2 == a->ver2 &&
			ver3 == a->ver3 &&
			build_flags == a->build_flags;
	}
};

ReplayFile::ReplayFile()
{
	mode = ReplayFile::DISABLE;
}

ReplayFile::~ReplayFile()
{
}

void ReplayFile::close()
{
	if( mode == ReplayFile::DISABLE )
		return;
	file.file_close();
	mode = ReplayFile::DISABLE;
}

int ReplayFile::open(const char* filePath, int open_mode)
{
	GameVer current_version;
	current_version.set_current_version();
	if( mode != ReplayFile::DISABLE )
		return 0;
	if( open_mode == ReplayFile::READ )
	{
		char magic[4];
		int32_t file_version;
		GameVer version;
		if( !file.file_open(filePath, 0) )
			return 0;
		if( !file.file_read(&magic, 4) )
			goto out;
		if( memcmp(magic, file_magic, 4) )
			goto out;
		if( file_version != replay_version )
			goto out;
		if( !file.file_read(&version, sizeof(GameVer)) )
			goto out;
// Can warn user of version mismatch
//		if( !current_version.cmp(&version) )
//			box.msg();
		if( !config.read_file(&file, 1) ) // 1-keep system settings
			goto out;
		mode = ReplayFile::READ;
		return 1;
	}
	if( open_mode == ReplayFile::WRITE )
	{
		if( !file.file_create(filePath, 0) )
			return 0;
		file.file_write((void *)file_magic, 4);
		file.file_put_long(replay_version);
		file.file_write(&current_version, sizeof(GameVer));
		config.write_file(&file);
		mode = ReplayFile::WRITE;
		return 1;
	}
	return 1;
out:
	file.file_close();
	return 0;
}

void ReplayFile::read(uint32_t *id, char *data, uint16_t max_size)
{
	if( mode != ReplayFile::READ )
		return;
	uint16_t size;
	file.file_read(&size, sizeof(uint16_t));
	if( size > max_size )
		err.run("msg size too large");
	file.file_read(id, sizeof(uint32_t));
	if( size )
		file.file_read(data, size);
}

// size = sizeof(id) + data buf size
void ReplayFile::write(uint32_t id, char *data, uint16_t size)
{
	if( mode != ReplayFile::WRITE )
		return;
	if( !id )
		return; // not a processed message, so don't save
	err_when( size<4 );
	size -= 4; // don't include size of id, only data
	file.file_write(&size, sizeof(uint16_t));
	file.file_write(&id, sizeof(uint32_t));
	if( size )
		file.file_write(data, size);
}
