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

//Filename    : OVIDEO.CPP
//Description : Video for Windows playback class
//Owner       : Gilbert

// for some .h files to define some IIDs
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <string.h>
#include <objbase.h>
#include <strmif.h>
#include <evcode.h>
#include <control.h>

#include <amvideo.h>
#include <uuids.h>

//#include <mmsystem.h>
//#include <digitalv.h>

#include <ALL.h>
#include <OSTR.h>
#include <OVIDEO.h>

// --------- Define constant -------//
// OATRUE and OAFALSE are defined in classes\base\ctlutil.h under active movie SDK
#define OATRUE -1
#define OAFALSE 0

static long FAR PASCAL video_win_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void create_dummy_window(HINSTANCE hInstance);


// #define FULL_SCREEN_VIDEO
#define CREATE_DUMMY_WINDOW

/*
//--------- Begin of function Video::play() ----------//
//
// <char*> aviFileName - the file name of the AVI file.
// [DWORD]   waitTime  - no. of milli-seconds to wait still after
//								 finishing playing the AVI.
//								 (default: 0)
//
void Video::play(char* aviFileName, DWORD waitTime)
{
	char	 resultStr[101];
	String str;

	str  = "open ";
	str += aviFileName;
	str += " alias mov";

	mciSendString(str, NULL, 0, NULL);
	mciSendString("play mov fullscreen", NULL, 0, NULL);

	//------ wait until the whole video playback process is complete ------//

	while( 1 )
	{
		mciSendString( "status mov mode", resultStr, 100, NULL );

		if( strcmp(resultStr, "stopped")==0 )
			break;
	}

	//-------- wait still after playing the movie ------//

	DWORD curTime = m.get_time();

	while( m.get_time() < curTime+waitTime );

	//------------ close the MCI ---------------//

	mciSendString("close mov"		, NULL, 0, NULL);
	mciSendString("close avivideo", NULL, 0, NULL);
}
//--------- End of function Video::play() ----------//
*/

Video::Video()
{
	CoInitialize(NULL);
	state = UNINITIALIZED;
	pGraph = NULL;
	hGraphNotifyEvent = NULL;
	init_success = 0;
	skip_on_fail_flag = 0;
}

Video::~Video()
{
	CoUninitialize();
}

void Video::set_skip_on_fail()
{
	skip_on_fail_flag = 1;
}

void Video::clear_skip_on_fail()
{
	skip_on_fail_flag = 0;
}

void Video::init()
{
	IMediaEvent *pME;
	HRESULT hr;

	init_success = 0;
	hwnd = NULL;
	if( ( hr = CoCreateInstance(CLSID_FilterGraph,     // CLSID of object
								NULL,                         // Outer unknown
								CLSCTX_INPROC_SERVER,         // Type of server
								IID_IGraphBuilder,            // Interface wanted
								(void **) &pGraph)            // Returned object
								) == 0 )
	{
		// We use this to find out events sent by the filtergraph
		if( (hr = pGraph->QueryInterface(IID_IMediaEvent, (void **) &pME)) == 0)
		{
			if( (hr = pME->GetEventHandle( (OAEVENT*) &hGraphNotifyEvent)) == 0)
			{
				init_success = 1;
				state = STOPPED;
			}
			pME->Release();
		}
	}

	if( hr && !skip_on_fail_flag)
	{
		err.run("video.init error %ld", hr );
	}
}

void Video::deinit()
{
	if( pGraph )
	{
		pGraph->Release();
		pGraph = NULL;
	}
	hGraphNotifyEvent = NULL;
	state = UNINITIALIZED;
}


void Video::play( char *fileName, DWORD )
{
	WCHAR wPath[100];
	HRESULT hr;
	IMediaControl *pMC;

	if(!init_success)
		return;

	MultiByteToWideChar( CP_ACP, 0, fileName, -1, wPath, 100 );

	if( (hr = pGraph->RenderFile(wPath, NULL)) == 0)
	{

		// use full screen video interface
		// try to change display mode
		IVideoWindow *iVideoWindow = NULL;
		if( (hr = pGraph->QueryInterface(IID_IVideoWindow, (void **) &iVideoWindow)) == 0)
		{
#ifdef CREATE_DUMMY_WINDOW
			if(hwnd)
			{
				HRESULT hr2 = iVideoWindow->put_MessageDrain((OAHWND) hwnd);
				hr2 = 0;
			}
#endif

#ifdef FULL_SCREEN_VIDEO
			IFilter *iFilter;
			if( pGraph->FindFilterByName(L"Video Renderer", &iFilter) == 0)
			{
				IBasicVideo *iBasicVideo;
				if( iFilter->QueryInterface(IID_IBasicVideo, (void **)&iBasicVideo) == 0)
				{
					IFullScreenVideo *iFullScreenVideo;
					IDirectDrawVideo *iDirectDrawVideo;
					if( iFilter->QueryInterface(IID_IFullScreenVideo, (void **)&iFullScreenVideo) == 0)
					{
						iFullScreenVideo->Release();
					}
					else if( iFilter->QueryInterface(IID_IDirectDrawVideo, (void **)&iDirectDrawVideo) == 0)
					{
						HRESULT hr2;
						hr2 = iDirectDrawVideo->UseWhenFullScreen(OATRUE);
						iDirectDrawVideo->Release();
					}

					iBasicVideo->Release();
				}
				iFilter->Release();
			}
			hr=iVideoWindow->put_FullScreenMode(OATRUE);
#endif

			/* // code to find all filter in the filter graph
			{
				IEnumFilters *iEnumFilters;
				pGraph->EnumFilters(&iEnumFilters);

				ULONG filterCount = 16;
				IFilter *iFilters[16];
				iEnumFilters->Next(filterCount, iFilters, &filterCount);

				for( ULONG j = 0; j < filterCount; ++j )
				{
					FILTER_INFO filterInfo;
					iFilters[j]->QueryFilterInfo(&filterInfo);
					filterInfo.pGraph->Release();
					iFilters[j]->Release();
				}

				iEnumFilters->Release();
			}*/

			iVideoWindow->HideCursor(OATRUE);
			iVideoWindow->put_Visible( OAFALSE );
			iVideoWindow->put_AutoShow( OAFALSE );
			LONG windowStyle;
			iVideoWindow->get_WindowStyle( &windowStyle);
			windowStyle &= ~WS_BORDER & ~WS_CAPTION & ~WS_SIZEBOX & ~WS_THICKFRAME &
				~WS_HSCROLL & ~WS_VSCROLL & ~WS_VISIBLE;
			iVideoWindow->put_WindowStyle( windowStyle);
		}
		else
			iVideoWindow = NULL;
		
		if( (hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pMC)) == 0)
		{
			pMC->Run();					// sometimes it returns 1, but still ok
			state = PLAYING;
			pMC->Release();
		}

		if( iVideoWindow )
		{
			iVideoWindow->put_Visible( OAFALSE );
			LONG windowStyle;
			iVideoWindow->get_WindowStyle( &windowStyle);
			windowStyle &= ~WS_BORDER & ~WS_CAPTION & ~WS_SIZEBOX & ~WS_THICKFRAME &
				~WS_HSCROLL & ~WS_VSCROLL & ~WS_VISIBLE;
			iVideoWindow->put_WindowStyle( windowStyle);

			LONG maxWidth;
			LONG maxHeight;
			hr=iVideoWindow->GetMaxIdealImageSize( &maxWidth, &maxHeight);
#ifdef FULL_SCREEN_VIDEO
#else
			iVideoWindow->put_BorderColor( RGB(0,0,0) );
			iVideoWindow->put_WindowState(SW_MAXIMIZE);

			IBaseFilter *iFilter;
			if( pGraph->FindFilterByName((const WCHAR *)L"Video Renderer", &iFilter) == 0)
			{
				IBasicVideo *iBasicVideo;
				if( iFilter->QueryInterface(IID_IBasicVideo, (void **)&iBasicVideo) == 0)
				{
					LONG screenWidth;
					LONG screenHeight;
					LONG videoWidth;
					LONG videoHeight;
					if( iVideoWindow->get_Width(&screenWidth) == 0 &&
						iVideoWindow->get_Height(&screenHeight) == 0 &&
						iBasicVideo->GetVideoSize(&videoWidth, &videoHeight) == 0)
					{
						// zoom in by 2 if possible
						if( screenWidth >= videoWidth * 2 &&
							screenHeight >= videoHeight * 2)
						{
							videoWidth *= 2;
							videoHeight *= 2;
						}

						// center the video client area
						iBasicVideo->SetDestinationPosition(
							(screenWidth-videoWidth)/2, (screenHeight-videoHeight)/2,
							videoWidth, videoHeight);
					}

					iBasicVideo->Release();
				}
				iFilter->Release();
			}
#endif
			iVideoWindow->HideCursor(OATRUE);
			iVideoWindow->SetWindowForeground(OATRUE);
		}

		if(iVideoWindow)
		{
			iVideoWindow->Release();
			iVideoWindow = NULL;
		}
	}

	if( hr && !skip_on_fail_flag)
		err.run("video.play error %d", hr );
}

void Video::play_until_end( char *fileName, HINSTANCE hInstance, DWORD t)
{
	HANDLE  ahObjects[1];       // Handles that need to be waited on
	const int cObjects = 1;     // Number of objects that we have
	
	if(!init_success)
		return;

	hwnd = NULL;
#ifdef CREATE_DUMMY_WINDOW
	create_dummy_window(hInstance);
#endif

	play(fileName, t);

	while( state == PLAYING )
	{
		if( (ahObjects[ 0 ] = hGraphNotifyEvent) == NULL)
		{
			state = STOPPED;
			break;
		}
		DWORD Result = MsgWaitForMultipleObjects( cObjects, ahObjects,
			FALSE, INFINITE, QS_ALLINPUT);
		
		// Have we received an event notification
		if( Result >= WAIT_OBJECT_0 && Result < (WAIT_OBJECT_0 + cObjects) )
		{
			if( Result == WAIT_OBJECT_0 )
				on_graph_notify();
		}
		else if( Result == WAIT_OBJECT_0 + cObjects )
		{
			if( hwnd )
			{
				// message in the message queue
				MSG msg;
				while( PeekMessage(&msg, hwnd, 0, ~0UL, PM_NOREMOVE) )
				{
					if( !GetMessage(&msg, hwnd, 0, ~0UL) )
						break;
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		else
		{
			// other event to wait ...
		}
	}

	if( hwnd )
	{
		PostMessage( hwnd, WM_CLOSE, 0, 0 );

		//handle outstanding message
		MSG msg;
		while( GetMessage(&msg, hwnd, 0, ~0UL) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	hwnd = NULL;
}

void Video::stop()
{

	HRESULT hr;
	IMediaControl *pMC;

	if(!init_success)
		return;

	// Obtain the interface to our filter graph
	if( (hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pMC))== 0 )
	{
		hr = pMC->Stop();
		pMC->Release();

		// rewind to the beginning
		IMediaPosition *pMP;
		if( (hr=pGraph->QueryInterface( IID_IMediaPosition, (void **) &pMP))==0)
		{
			pMP->put_CurrentPosition( 0);
			pMP->Release();
		}
	}
	// force it to stop
	state = STOPPED;

	if( hr && !skip_on_fail_flag)
		err.run("video.stop error %d", hr );
}

void Video::abort()
{
	HRESULT hr;
	IMediaControl *pMC;

	if(!init_success)
		return;


	// Obtain the interface to our filter graph
	if( (hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pMC)) == 0)
	{
		// Ask the filter graph to stop and release the interface
		hr = pMC->Stop();
		pMC->Release();

		// rewind to the beginning
		IMediaPosition *pMP;
		if( (hr=pGraph->QueryInterface( IID_IMediaPosition, (void **) &pMP))==0)
		{
			pMP->put_CurrentPosition( 0);
			pMP->Release();
		}
	}
	state = STOPPED;

	if( hr && !skip_on_fail_flag)
		err.run("video.abort error %d", hr);
}


void Video::on_graph_notify()
{
	IMediaEvent *pME;
	LONG lEventCode;
        LONG_PTR lParam1, lParam2;
	HRESULT hr;

	if( (hr=pGraph->QueryInterface(IID_IMediaEvent, (void **) &pME)) == 0)
	{
		if( (hr=pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0)) == 0)
		{
			switch(lEventCode)
			{
			case EC_COMPLETE:
				stop();
				break;

			case EC_USERABORT:
			case EC_ERRORABORT:
				abort();
				break;
			}
		}
		pME->Release();
	}

	if( hr && !skip_on_fail_flag)
		err.run("video.on_graph_notify error %d", hr);
}


static void create_dummy_window(HINSTANCE hInstance)
{

   //--------- register window class --------//

   WNDCLASS    wc;
   BOOL        rc;

   wc.style          = CS_DBLCLKS;
   wc.lpfnWndProc    = video_win_proc;
   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hInstance      = hInstance;
   wc.hIcon          = NULL;	// LoadIcon( hInstance, MAKEINTATOM(IDI_ICON1));
   wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
   wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
   wc.lpszMenuName   = NULL;
   wc.lpszClassName  = "Seven Kingdoms Video Window";

   rc = RegisterClass( &wc );

   if( !rc )
      return;

   video.hwnd = CreateWindowEx(
       WS_EX_APPWINDOW | WS_EX_TOPMOST,
       "Seven Kingdoms Video Window",
		 "Seven Kingdoms",
		 WS_POPUP,
		 0,
		 0,
       GetSystemMetrics(SM_CXSCREEN),
       GetSystemMetrics(SM_CYSCREEN),
		 NULL,
       NULL,
       hInstance,
       NULL );

   if( !video.hwnd)
      return;

   UpdateWindow( video.hwnd );
   SetFocus( video.hwnd );

   return;
}


static long FAR PASCAL video_win_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message )
   {
      case WM_CREATE:
         video.hwnd = hWnd;
			break;

		//case WM_SYSKEYUP:
		//	if( (wParam==27 && lParam==0x80010001) || (wParam==9 && lParam==0xa00f0001) )
		//		pause();
		//	break;

       case WM_DESTROY:
          video.hwnd = NULL;
          PostQuitMessage( 0 );
          break;

		 case WM_LBUTTONDOWN:
			 video.stop();
			 PostMessage(hWnd, WM_CLOSE, 0, 0);
			 break;

       default:
			 break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
