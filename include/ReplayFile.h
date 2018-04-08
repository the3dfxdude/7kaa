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

//Filename    : ReplayFile.h
//Description : Replay File IO

#ifndef __REPLAYFILE_H
#define __REPLAYFILE_H

#include <OFILE.h>

struct NewNationPara;
class RemoteQueue;

class ReplayFile
{
public:
	enum {
		DISABLE,
		READ,
		WRITE,
	};

private:
	File file;
	long file_size;

public:
	int mode;

public:
	ReplayFile();
	~ReplayFile();

	void close();
	int open_read(const char *filePath, NewNationPara *mpGame, int *mpPlayerCount);
	int open_write(const char *filePath, NewNationPara *mpGame, int mpPlayerCount);
	int read_queue(RemoteQueue *rq);
	void write_queue(RemoteQueue *rq);
};

//-----------------------------------------------//

extern ReplayFile replay_file;

#endif
