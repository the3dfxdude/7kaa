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

//Filename    : OVIDEO.H
//Description : Header file of Video for Windows playback class
//Owner       : Gilbert

#ifndef __OVIDEO_H
#define __OVIDEO_H

#include <strmif.h>

typedef enum tagState {UNINITIALIZED, STOPPED, PAUSED, PLAYING } State;

//---------- Define class Video ----------//

class Video
{
public:
	State state;
	IGraphBuilder *pGraph;
	HANDLE hGraphNotifyEvent;
	int	init_success;
	int	skip_on_fail_flag;
	HWND	hwnd;

public:
	Video();
	~Video();
	void	init();
	void	deinit();
	void	set_skip_on_fail();
	void	clear_skip_on_fail();

	void	play(char*, DWORD=0);
	void	play_until_end(char *, HINSTANCE hInstance, DWORD = 0);
	void	stop();
	void	abort();

private:
	void	on_graph_notify();
};

//----------------------------------------//

extern Video video;

#endif

