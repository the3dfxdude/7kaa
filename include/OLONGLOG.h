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

// Filename    : OLONGLOG.H
// Description : 

#ifndef __OLONGLOG_H
#define __OLONGLOG_H

#include <OFILE.h>

class LongLog : public File
{
public:
	LongLog(char suffix);
	~LongLog();

	void printf(char*,...);
};

#ifdef DEBUG
extern LongLog *long_log;
#endif

#endif