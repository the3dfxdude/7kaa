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

//Filename    : CmdLine.h
//Description : Command line processing

#ifndef __CMDLINE_H
#define __CMDLINE_H

enum StartupMode
{
	STARTUP_NORMAL,
	STARTUP_MULTI_PLAYER,
	STARTUP_TEST,
	STARTUP_DEMO,
};

struct CmdLine
{
	int		enable_audio;
	int		enable_if;
	int		game_speed;
	int		rnd;
	StartupMode	startup_mode;
	char		*join_host;

	CmdLine();
	~CmdLine();

	int init(int argc, char **argv);
};

//-----------------------------------------------//

extern CmdLine cmd_line;

#endif
