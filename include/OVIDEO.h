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

