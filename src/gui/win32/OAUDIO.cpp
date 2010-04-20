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

//Filename    : OAUDIO.CPP
//Description : Object Midi Audio and Digitized Sound
//Ownership   : Gilbert

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <mmsystem.h>
#include <string.h>
#include <limits.h>

#include <OSYS.h>
#include <syswin.h>
#include <OBOX.h>
#include <OAUDIO.h>
#include <OVGALOCK.h>

//---------------- Define constant ------------------//
//
// DirectSoundBuffer size = LWAV_STREAM_BUFSIZ * LWAV_BANKS
// if it is going to play a high transfer rate wave
// (e.g. 16-bit 44.1kHz Stereo), increase LWAV_STREAM_BUFSIZ
//
//---------------------------------------------------//

#define LWAV_STREAM_BUFSIZ	0x1000
#define LWAV_BANKS			4
#define LOOPWAV_STREAM_BUFSIZ 0x1000
#define LOOPWAV_BANKS		4

#ifndef DSBCAPS_CTRLDEFAULT
	#define DSBCAPS_CTRLDEFAULT (DSBCAPS_CTRLFREQUENCY|DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME)
#endif


//---------- Define static variables ---------//

static MCI_PLAY_PARMS mci_play;
static MCI_OPEN_PARMS mci_open;
static MCI_SET_PARMS  mci_set;

//--------- Begin of function wavefile_offset -------//
//
// find the "data" tag in a wave file
//
static char * wavefile_data(char *wavfile_buf)
{
	//----- position at WAVEfmt tag size ------//

	char *p = wavfile_buf+0x10;
	DWORD tagSize=*(DWORD *)p;

	//-------- go to next tag field -----------//

	for( p += sizeof(DWORD)+tagSize; strncmp(p, "data", 4) != 0;
		  p += sizeof(DWORD)+tagSize)
	{
		p += 4;						// pointing at size of tag field
		tagSize = *(DWORD *)p;	// get the size of this tage field

		if(p - wavfile_buf > 128)
			return NULL;
	}

	//----- p pointing at the start of "data" tag ------//

	return p;
}
//--------- End of function wavefile_offset------------//


//--------- Begin of function Audio::Audio ----------//

Audio::Audio()
{
	init_flag = 0;
}
//--------- Begin of function Audio::Audio ----------//


//--------- Begin of function Audio::~Audio ----------//

Audio::~Audio()
{
	deinit();
}
//--------- Begin of function Audio::~Audio ----------//


//--------- Begin of function Audio::init ----------//
//
// Initialize the mid driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init()
{
	//-------- init vars -----------//

	run_yield = 0;
	mid_init_flag = 0;     // the init flag is on when the driver is initialized successfully
	wav_init_flag = 0;
	cd_init_flag  = 0;

	mid_flag = 1;
	wav_flag = 1;
	cd_flag  = 1;

	mid_buf = NULL;
	wav_buf = NULL;

	mid_buf_size = 0;
	wav_buf_size = 0;

	int i;
	for(i = 0; i < MAX_WAV_CHANNEL; ++i)
	{
		lp_wav_ch_dsb[i] = NULL;
		wav_serial_no[i] = 0;
	}
	max_lwav_serial_no = 0;

	for(i=0; i < MAX_LONG_WAV_CH; ++i)
	{
		lp_lwav_ch_dsb[i] = NULL;
		lwav_serial_no[i] = 0;
		lwav_fileptr[i] = NULL;
	}
	max_lwav_serial_no = 0;

	for(i=0; i < MAX_LOOP_WAV_CH; ++i)
	{
		lp_loop_ch_dsb[i] = NULL;
		loopwav_fileptr[i] = NULL;
	}

	// wav_volume = 0;
	wav_volume = 100;		// 0(slient) - 100(loudest)

	//--------- init devices ----------//

	if( init_wav() )
	{
		wav_res.init( DIR_RES"A_WAVE2.RES", 0, 0 );      // 2nd 0-don't read all, 3rd 0-don't use vga buffer
		wav_buf 		 = mem_add(DEFAULT_WAV_BUF_SIZE);
		wav_buf_size = DEFAULT_WAV_BUF_SIZE;
	}
/*
	if( init_mid() )
	{
		mid_res.init( DIR_RES"A_MIDI.RES", 0, 0 );      // 2nd 0-don't read all, 3rd 0-don't use vga buffer
		mid_buf 		 = mem_add(DEFAULT_MID_BUF_SIZE);
		mid_buf_size = DEFAULT_MID_BUF_SIZE;
	}
*/
	init_cd();

	//----------------------------------//

	init_flag = wav_init_flag || cd_init_flag;

	return 1;
}
//--------- End of function Audio::init ----------//


//--------- Begin of function Audio::deinit ----------//

void Audio::deinit()
{
	if( init_flag )
	{
		//------- deinit vars --------//

		run_yield = 0;
		init_flag = 0;

		//------- deinit devices -------//

		deinit_wav();
		deinit_mid();
		deinit_cd();
	}
}
//--------- End of function Audio::deinit ----------//


//--------- Begin of function Audio::init_wav ----------//
//
// Initialize digitized wav driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init_wav()
{
	if( wav_init_flag )
		return 1;

	//-------- create DirectSound object -------//

	HRESULT rc=DirectSoundCreate(NULL, &lp_direct_sound, NULL);

	//------------------------------------------//

	if( rc==DS_OK )		// Create succeeded
	{
		lp_direct_sound->SetCooperativeLevel(window.main_hwnd, DSSCL_NORMAL);
		wav_init_flag=1;
	}

	return wav_init_flag;
}
//--------- End of function Audio::init_wav ----------//


//--------- Begin of function Audio::init_mid ----------//
//
// Initialize MIDI mid driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init_mid()
{
	if( mid_init_flag )
		return 1;


	//.. insert code here ...//


	mid_init_flag=1;

	return 1;
}
//--------- End of function Audio::init_mid ----------//


//--------- Begin of function Audio::init_cd ----------//
//
// Initialize the audio CD player
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init_cd()
{
	mci_open.lpstrDeviceType = (LPCSTR) MCI_DEVTYPE_CD_AUDIO;

	if( mciSendCommand( NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID,
							  (DWORD) (LPVOID) &mci_open) == 0 ||
		mciSendCommand( NULL, MCI_OPEN, MCI_OPEN_TYPE |
								  MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE,
								  (DWORD) (LPVOID) &mci_open)== 0 )
	{
		mci_set.dwTimeFormat = MCI_FORMAT_TMSF;

		mciSendCommand( mci_open.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
			(DWORD) (LPVOID) &mci_set);

		cd_init_flag = 1;
		return 1;
	}

	cd_init_flag = 0;
	return 0;
}
//--------- End of function Audio::init_cd ----------//


//--------- Begin of function Audio::deinit_cd ----------//

void Audio::deinit_cd()
{
	if( cd_init_flag )
	{
		stop_cd();

		mciSendString ("close cdaudio", NULL, 0, NULL);

		cd_init_flag = 0;
	}
}
//--------- End of function Audio::deinit_cd ----------//


//--------- Begin of function Audio::deinit_wav ----------//

void Audio::deinit_wav()
{
	stop_wav();

	if( wav_buf )
	{
		mem_del(wav_buf);
		wav_buf = NULL;
	}

	if(wav_init_flag)
	{
		lp_direct_sound->Release();
		wav_init_flag = 0;
	}
}
//--------- End of function Audio::deinit_wav ----------//


//--------- Begin of function Audio::deinit_mid ----------//

void Audio::deinit_mid()
{
	if( !mid_init_flag )
		return;

	stop_mid();

	//.. insert code here ...//
	mem_del(mid_buf);
	mid_buf = NULL;

	mid_init_flag = 0;
}
//--------- End of function Audio::deinit_mid ----------//


//------- Begin of function Audio::play_mid -------//
//
// Play a midi mid from the mid resource file
//
// <char*> midName = name of the mid in the resource file
//
// return : <int> 1 - mid loaded and is playing
//                0 - mid not played
//
int Audio::play_mid(char* midName)
{
	if( !mid_init_flag || !mid_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	stop_mid();	    // stop currently playing mid if any

	//------ Load mid file -------//

	int   dataSize;

	File* filePtr = mid_res.get_file(midName, dataSize);

	if( !filePtr )
		return 0;

	if( dataSize > mid_buf_size )
	{
		mid_buf_size = dataSize;
		mid_buf = mem_resize( mid_buf, mid_buf_size );
	}

	if( !filePtr->file_read( mid_buf, dataSize ) )
		return 0;

	//-------- Play mid file --------//

	//.. insert code here ...//


	return 1;
}
//------- End of function Audio::play_mid -------//


//------- Begin of function Audio::stop_mid -------//
//
void Audio::stop_mid()
{
	if( !mid_init_flag || !mid_flag )
		return;

	//.. insert code here ...//

	mciSendCommand(mci_open.wDeviceID, MCI_STOP, NULL, NULL);
}
//------- End of function Audio::stop_mid -------//


//------- Begin of function Audio::play_wav -------//
//
// Play digitized wav from the wav resource file
//
// <char*>        wavName = name of the wav in the resource file
// long vol = volume (0 to 100)
// long pan = pan (-10000 to 10000)
//
// return : <int> non-zero - wav loaded and is playing, return a serial no. to be referred in stop_wav and is_wav_playing
//                0 - wav not played
//
int Audio::play_wav(char* wavName, DsVolume dsVolume)
{
/*
	//---- redirect to play_long_wav -------//

	String str;

   str  = DIR_SOUND;
	str += wavName;
	str += ".WAV";

	if( m.is_file_exist(str) )
		return play_long_wav(str);
	else
		return 0;

	//------------------------------------//
*/
	if( !wav_init_flag || !wav_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	//-------- Load wav file header-------//
	int   dataSize;
	DWORD wavDataOffset, wavDataLength;

	File* filePtr = wav_res.get_file(wavName, dataSize);

	if( !filePtr )
		return 0;

	// load small part of the wave file (first 128 bytes) enough to hold
	// the hold header
//#define LOAD_FULL_WAVE
#ifdef LOAD_FULL_WAVE
	if( dataSize > wav_buf_size )
	{
		wav_buf_size = dataSize;
		wav_buf = mem_resize( wav_buf, wav_buf_size );
	}
	if( !filePtr->file_read( wav_buf, dataSize))
#else
	if( !filePtr->file_read( wav_buf, 128 ) )
#endif
		return 0;

// short-cut to test play_resided_wave()
#ifdef LOAD_FULL_WAVE
	return play_resided_wav(wav_buf);
#endif

	// determine the wave data offset and length
	char * dataTag = wavefile_data(wav_buf);
	if (!dataTag)
	{
		err_now("Invalid wave file format");
		return 0;		// invalid RIFF WAVE format
	}

	wavDataOffset = (dataTag - wav_buf) + 4 + sizeof(DWORD);
	wavDataLength = *(DWORD *)(dataTag+4);

#ifndef LOAD_FULL_WAVE
	// seek to the start of wave data
	filePtr->file_seek(wavDataOffset-128, SEEK_CUR);
#endif

	//------- Create DirectSoundBuffer to store a wave ------//
	LPDIRECTSOUNDBUFFER lpDsb;
	DSBUFFERDESC dsbDesc;
	HRESULT hr;
	DWORD dsbStatus;

		// set up DSBUFFERDESC structure
	memset(&dsbDesc, 0, sizeof(DSBUFFERDESC));	// zero it out
	dsbDesc.dwSize = sizeof(DSBUFFERDESC);
	dsbDesc.dwFlags = DSBCAPS_CTRLDEFAULT;			// Need defaul controls (pan, volume, frequency)
	dsbDesc.dwBufferBytes = wavDataLength;
	dsbDesc.lpwfxFormat = (LPWAVEFORMATEX) (wav_buf+0x14);
	// ------- assign buffer to a channel number ----------//
	lpDsb = NULL;
	int chanNum;
	for( chanNum = 0; chanNum < MAX_WAV_CHANNEL; ++chanNum)
		if(lp_wav_ch_dsb[chanNum] == NULL || (lp_wav_ch_dsb[chanNum]->GetStatus(&dsbStatus),
			!(dsbStatus & DSBSTATUS_PLAYING)))
		{
			if(lp_wav_ch_dsb[chanNum])
			{
				lp_wav_ch_dsb[chanNum]->Release();
				lp_wav_ch_dsb[chanNum] = NULL;
			}
			// found an idle channel, create DirectSoundBuffer
			hr = lp_direct_sound->CreateSoundBuffer(&dsbDesc, &lpDsb, NULL);
			if (DS_OK != hr)
			{
				// failed!
				err_now("Cannot create DirectSoundBuffer");
				return 0;
			}
			lp_wav_ch_dsb[chanNum] = lpDsb;
			break;
		}
	if( chanNum >= MAX_WAV_CHANNEL)
	{
		return 0;
	}
	// Note : if not found, just play the sound, don't play the sound
	// increase MAX_WAV_CHANNEL

	//------- copy sound data to DirectSoundBuffer--------//
	// unlock vga_front
	VgaFrontLock vgaLock;

	// lock the buffer first
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	LPVOID lpvPtr2;
	DWORD dwBytes2;

	hr = lpDsb->Lock(0, wavDataLength, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	if(DS_OK != hr)
	{
		// fail to lock
		err_now("Cannot lock DirectSoundBuffer");
		return 0;
	}

	// write to pointers
#ifdef LOAD_FULL_WAVE
	memcpy(lpvPtr1, wav_buf+wavDataOffset, dwBytes1);
#else
	filePtr->file_read(lpvPtr1, dwBytes1);
#endif
	if(lpvPtr2)
	{
#ifdef LOAD_FULL_WAVE
		memcpy(lpvPtr2, wav_buf+wavDataOffset+dwBytes1, dwBytes2);
#else
		filePtr->file_read(lpvPtr2, dwBytes2);
#endif
	}
	// unlock data back
	hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	if(DS_OK != hr)
	{
		// fail to unlock
		err_now("Cannot unlock DirectSoundBuffer");
		return 0;
	}

	//------- Set volume and pan -----------//
	lpDsb->SetVolume(dsVolume.ds_vol);
	lpDsb->SetPan(dsVolume.ds_pan);

	//------- Play wav file --------//
	if(lpDsb->Play(0, 0, 0) != DS_OK)
	{
		// fail to play
		err_now("Cannot play DirectSoundBuffer");
		return 0;
	}

	return wav_serial_no[chanNum] = assign_serial(max_wav_serial_no);
}
//------- End of function Audio::play_wav -------//


//------- Begin of function Audio::play_wav -------//
//
// Play digitized wav from the wav resource file
//
// short resIdx = index of wave file in A_WAVE2.RES
// long vol = volume (0 to 100)
// long pan = pan (-10000 to 10000)
//
// return : <int> 1 - wav loaded and is playing
//                0 - wav not played
//
int Audio::play_wav(short resIdx, DsVolume dsVolume)
{
	if( !wav_init_flag || !wav_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	//-------- Load wav file header-------//
	int   dataSize;
	DWORD wavDataOffset, wavDataLength;

	File* filePtr = wav_res.get_file(resIdx, dataSize);

	if( !filePtr )
		return 0;

	// load small part of the wave file (first 128 bytes) enough to hold
	// the hold header
#ifdef LOAD_FULL_WAVE
	if( dataSize > wav_buf_size )
	{
		wav_buf_size = dataSize;
		wav_buf = mem_resize( wav_buf, wav_buf_size );
	}
	if( !filePtr->file_read( wav_buf, dataSize))
#else
	if( !filePtr->file_read( wav_buf, 128 ) )
#endif
		return 0;

// short-cut to test play_resided_wave()
#ifdef LOAD_FULL_WAVE
	return play_resided_wav(wav_buf);
#endif

	// determine the wave data offset and length
	char * dataTag = wavefile_data(wav_buf);
	if (!dataTag)
	{
		err_now("Invalid wave file format");
		return 0;		// invalid RIFF WAVE format
	}

	wavDataOffset = (dataTag - wav_buf) + 4 + sizeof(DWORD);
	wavDataLength = *(DWORD *)(dataTag+4);

#ifndef LOAD_FULL_WAVE
	// seek to the start of wave data
	filePtr->file_seek(wavDataOffset-128, SEEK_CUR);
#endif

	//------- Create DirectSoundBuffer to store a wave ------//
	LPDIRECTSOUNDBUFFER lpDsb;
	DSBUFFERDESC dsbDesc;
	HRESULT hr;
	DWORD dsbStatus;

		// set up DSBUFFERDESC structure
	memset(&dsbDesc, 0, sizeof(DSBUFFERDESC));	// zero it out
	dsbDesc.dwSize = sizeof(DSBUFFERDESC);
	dsbDesc.dwFlags = DSBCAPS_CTRLDEFAULT;			// Need defaul controls (pan, volume, frequency)
	dsbDesc.dwBufferBytes = wavDataLength;
	dsbDesc.lpwfxFormat = (LPWAVEFORMATEX) (wav_buf+0x14);
	// ------- assign buffer to a channel number ----------//
	lpDsb = NULL;
	int chanNum;
	for( chanNum = 0; chanNum < MAX_WAV_CHANNEL; ++chanNum)
		if(lp_wav_ch_dsb[chanNum] == NULL || (lp_wav_ch_dsb[chanNum]->GetStatus(&dsbStatus),
			!(dsbStatus & DSBSTATUS_PLAYING)))
		{
			if(lp_wav_ch_dsb[chanNum])
			{
				lp_wav_ch_dsb[chanNum]->Release();
				lp_wav_ch_dsb[chanNum] = NULL;
			}
			// found an idle channel, create DirectSoundBuffer
			hr = lp_direct_sound->CreateSoundBuffer(&dsbDesc, &lpDsb, NULL);
			if (DS_OK != hr)
			{
				// failed!
				err_now("Cannot create DirectSoundBuffer");
				return 0;
			}
			lp_wav_ch_dsb[chanNum] = lpDsb;
			break;
		}
	if( chanNum >= MAX_WAV_CHANNEL)
	{
		return 0;
	}
	// Note : if not found, just play the sound, don't play the sound
	// increase MAX_WAV_CHANNEL
	
	//------- copy sound data to DirectSoundBuffer--------//
	// unlock vga_front
	VgaFrontLock vgaLock;

	// lock the buffer first
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	LPVOID lpvPtr2;
	DWORD dwBytes2;

	hr = lpDsb->Lock(0, wavDataLength, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	if(DS_OK != hr)
	{
		// fail to lock
		err_now("Cannot lock DirectSoundBuffer");
		return 0;
	}

	// write to pointers
#ifdef LOAD_FULL_WAVE
	memcpy(lpvPtr1, wav_buf+wavDataOffset, dwBytes1);
#else
	filePtr->file_read(lpvPtr1, dwBytes1);
#endif
	if(lpvPtr2)
	{
#ifdef LOAD_FULL_WAVE
		memcpy(lpvPtr2, wav_buf+wavDataOffset+dwBytes1, dwBytes2);
#else
		filePtr->file_read(lpvPtr2, dwBytes2);
#endif
	}
	// unlock data back
	hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	if(DS_OK != hr)
	{
		// fail to unlock
		err_now("Cannot unlock DirectSoundBuffer");
		return 0;
	}

	//------- Set volume and pan -----------//
	lpDsb->SetVolume(dsVolume.ds_vol);
	lpDsb->SetPan(dsVolume.ds_pan);

	//------- Play wav file --------//
	if(lpDsb->Play(0, 0, 0) != DS_OK)
	{
		// fail to play
		err_now("Cannot play DirectSoundBuffer");
		return 0;
	}

	return wav_serial_no[chanNum] = assign_serial(max_wav_serial_no);
}
//------- End of function Audio::play_wav -------//


//------- Begin of function Audio::play_resided_wav -------//
//
// Play digitized wav from the wav file in memory
//
// <char*>        wavBuf = point to the wav in memory
// long vol = volume (0 to 100)
// long pan = pan (-10000 to 10000)
//
// return : <int> 1 - wav loaded and is playing
//                0 - wav not played
//
int Audio::play_resided_wav(char* wavBuf, DsVolume dsVolume)
{
	if( !wav_init_flag || !wav_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	//-------- Load wav file header-------//
	DWORD wavDataOffset, wavDataLength;

	// determine the wave data offset and length
	char * dataTag = wavefile_data(wavBuf);
	if (!dataTag)
	{
		err_now("Invalid wave file format");
		return 0;		// invalid RIFF WAVE format
	}

	wavDataOffset = (dataTag - wavBuf) + 4 + sizeof(DWORD);
	wavDataLength = *(DWORD *)(dataTag+4);

	//------- Create DirectSoundBuffer to store a wave ------//
	LPDIRECTSOUNDBUFFER lpDsb;
	DSBUFFERDESC dsbDesc;
	HRESULT hr;
	DWORD dsbStatus;

		// set up DSBUFFERDESC structure
	memset(&dsbDesc, 0, sizeof(DSBUFFERDESC));	// zero it out
	dsbDesc.dwSize = sizeof(DSBUFFERDESC);
	dsbDesc.dwFlags = DSBCAPS_CTRLDEFAULT;			// Need defaul controls (pan, volume, frequency)
	dsbDesc.dwBufferBytes = wavDataLength;
	dsbDesc.lpwfxFormat = (LPWAVEFORMATEX) (wavBuf+0x14);
	// ------- assign buffer to a channel number ----------//
	lpDsb = NULL;
	int chanNum;
 	for( chanNum = 0; chanNum < MAX_WAV_CHANNEL; ++chanNum)
		if(lp_wav_ch_dsb[chanNum] == NULL || (lp_wav_ch_dsb[chanNum]->GetStatus(&dsbStatus),
			!(dsbStatus & DSBSTATUS_PLAYING)))
		{
			if(lp_wav_ch_dsb[chanNum])
			{
				lp_wav_ch_dsb[chanNum]->Release();
				lp_wav_ch_dsb[chanNum] = NULL;
			}
			// found an idle channel, create DirectSoundBuffer
			hr = lp_direct_sound->CreateSoundBuffer(&dsbDesc, &lpDsb, NULL);
			if (DS_OK != hr)
			{
				// failed!
				err_now("Cannot create DirectSoundBuffer");
				return 0;
			}
			lp_wav_ch_dsb[chanNum] = lpDsb;
			break;
		}
	if( chanNum >= MAX_WAV_CHANNEL)
	{
		return 0;
	}
	// Note : if not found, just play the sound, don't play the sound
	// increase MAX_WAV_CHANNEL
	
	//------- copy sound data to DirectSoundBuffer--------//
	// unlock vga_front
	VgaFrontLock vgaLock;

	// lock the buffer first
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	LPVOID lpvPtr2;
	DWORD dwBytes2;

	hr = lpDsb->Lock(0, wavDataLength, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	if(DS_OK != hr)
	{
		// fail to lock
		err_now("Cannot lock DirectSoundBuffer");
		return 0;
	}

	// write to pointers
	memcpy(lpvPtr1, wavBuf+wavDataOffset, dwBytes1);
	if(lpvPtr2)
	{
		memcpy(lpvPtr2, wavBuf+wavDataOffset+dwBytes1, dwBytes2);
	}
	// unlock data back
	hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	if(DS_OK != hr)
	{
		// fail to unlock
		err_now("Cannot unlock DirectSoundBuffer");
		return 0;
	}

	//------- Set volume -----------//
	lpDsb->SetVolume(dsVolume.ds_vol);
	lpDsb->SetPan(dsVolume.ds_pan);

	//------- Play wav file --------//
	if(lpDsb->Play(0, 0, 0) != DS_OK)
	{
		// fail to play
		err_now("Cannot play DirectSoundBuffer");
		return 0;
	}

	return wav_serial_no[chanNum] = assign_serial(max_wav_serial_no);
}
//------- End of function Audio::play_resided_wav -------//

// ###### begin Gilbert 6/12 ########//
//------- Begin of function Audio::get_free_wav_ch --------//
int Audio::get_free_wav_ch()
{
	int count = 0;
	DWORD dsbStatus;
	for( int chanNum = 0; chanNum < MAX_WAV_CHANNEL; ++chanNum)
	{
		if( lp_wav_ch_dsb[chanNum] != NULL && (lp_wav_ch_dsb[chanNum]->GetStatus(&dsbStatus),
			!(dsbStatus & DSBSTATUS_PLAYING)) )
		{
			lp_wav_ch_dsb[chanNum]->Release();
			lp_wav_ch_dsb[chanNum] = NULL;
		}

		if( !lp_wav_ch_dsb[chanNum] )
			count++;
	}
	
	return count;
}
//------- End of function Audio::get_free_wav_ch --------//
// ###### end Gilbert 6/12 ########//

//------- Begin of function Audio::stop_wav ------------//
//
// stop a short sound effect started by play_wav or play_resided_wav
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
// return 1 - channel is found and stopped / channel not found
// return 0 - cannot stop the channel
//
int Audio::stop_wav(int serial)
{
	for( int chanNum = 0; chanNum < MAX_WAV_CHANNEL; ++chanNum)
	{
		if(lp_wav_ch_dsb[chanNum] != NULL && wav_serial_no[chanNum] == serial)
		{
			lp_wav_ch_dsb[chanNum]->Stop();
			lp_wav_ch_dsb[chanNum]->Release();
			lp_wav_ch_dsb[chanNum] = NULL;
			wav_serial_no[chanNum] = 0;
			return 1;
		}
	}
	return 0;
}
//------- End of function Audio::stop_wav ------------//

//------- Begin of function Audio::is_wav_playing ------------//
//
// return wheather a short sound effect is stopped
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
int Audio::is_wav_playing(int serial)
{
	DWORD dsbStatus;
	for( int chanNum = 0; chanNum < MAX_WAV_CHANNEL; ++chanNum)
	{
		if(lp_wav_ch_dsb[chanNum] != NULL && wav_serial_no[chanNum] == serial 
			&& lp_wav_ch_dsb[chanNum]->GetStatus(&dsbStatus) )
		{
			return dsbStatus & DSBSTATUS_PLAYING;
		}
	}
	return 0;
}
//------- End of function Audio::is_wav_playing ------------//

//------- Begin of function Audio::play_long_wav --------//
//
// Play digitized wav from the wav file
// suitable for very large wave file
//
// <char*>        wavName = name of the wave file
//
// return : <int> 1 - wav loaded and is playing
//                0 - wav not played
// note : it uses streaming DirectSoundBuffer
// Audio::yield() keeps on feeding data to it

// Create a DirectSoundBuffer of size lwav_buf_size[c]*LWAV_BANKS
// divide into LWAV_BANKS parts. Each time Audio::yield() is called,
// load wave file into one part. lwav_bank[c] record which part to be
// filled next for channel c.

int Audio::play_long_wav(const char *wavName, DsVolume dsVolume)
{
	if( !wav_init_flag || !wav_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	//-------- Load wav file header-------//
	DWORD wavDataOffset,wavDataLength;

	if( LWAV_STREAM_BUFSIZ*LWAV_BANKS > wav_buf_size )
	{
		wav_buf_size = LWAV_STREAM_BUFSIZ*LWAV_BANKS;
		wav_buf = mem_resize( wav_buf, wav_buf_size );
	}

	// File* filePtr = (File *)mem_add(sizeof(File));		// new File;
	File* filePtr = new File;		// new File;
	if(!filePtr->file_open(wavName,0,0))
	{
		char errmsg[60];
		sprintf(errmsg, "Cannot open %s", wavName);
		box.msg(errmsg);
		delete filePtr;
		return 0;
	}

	if( !filePtr )
	{
		delete filePtr;
		return 0;
	}

	// load small part of the wave file (first 128 bytes) enough to hold
	// the hold header
	if( !filePtr->file_read( wav_buf, 128 ) )
	{
		delete filePtr;
		return 0;
	}

	// determine the wave data offset
	char * dataTag = wavefile_data(wav_buf);
	if (!dataTag)
	{
		err_now("Invalid wave file format");
		delete filePtr;
		return 0;		// invalid RIFF WAVE format
	}
	wavDataOffset = (dataTag - wav_buf) + 4 + sizeof(DWORD);
	wavDataLength = *(DWORD *)(dataTag+4);

	// seek to the start of wave data
	long temp1 = filePtr->file_seek(wavDataOffset);

	WORD OptBufferSize=LWAV_STREAM_BUFSIZ,
		MinRemainder =(WORD)(wavDataLength % (OptBufferSize * LWAV_BANKS));
	//------- find out the best buffer size -------//
	// store it in OptBufferSize
	// criteria : below or equal to LWAV_STREAM_BUFSIZ and
	// minimize wavDataLength % (OptBufferSize * LWAV_BANKS)
	// i.e. minimize the truncation to the wave file
	for(WORD TryBufSiz=LWAV_STREAM_BUFSIZ-0x200; TryBufSiz <= LWAV_STREAM_BUFSIZ;
	TryBufSiz+=0x20)
	{
		WORD TryRemainder = (WORD)(wavDataLength % (TryBufSiz * LWAV_BANKS));
		if(TryRemainder < MinRemainder)
		{
			MinRemainder = TryRemainder;
			OptBufferSize = TryBufSiz;
		}
	}

	//------- Create DirectSoundBuffer to store a wave ------//
	LPDIRECTSOUNDBUFFER lpDsb;
	DSBUFFERDESC dsbDesc;
	HRESULT hr;
	DWORD dsbStatus;

		// set up DSBUFFERDESC structure
	memset(&dsbDesc, 0, sizeof(DSBUFFERDESC));	// zero it out
	dsbDesc.dwSize = sizeof(DSBUFFERDESC);
	dsbDesc.dwFlags = DSBCAPS_CTRLDEFAULT;			// Need defaul controls (pan, volume, frequency)
	dsbDesc.dwBufferBytes = OptBufferSize * LWAV_BANKS;
	dsbDesc.lpwfxFormat = (LPWAVEFORMATEX) (wav_buf+0x14);
	// ------- assign buffer to a channel number ----------//
	lpDsb = NULL;
	int chanNum;
	for( chanNum = 0; chanNum < MAX_LONG_WAV_CH; ++chanNum)
		if(lp_lwav_ch_dsb[chanNum] == NULL || (lp_lwav_ch_dsb[chanNum]->GetStatus(&dsbStatus),
			!(dsbStatus & DSBSTATUS_PLAYING)))
		{
			if(lp_lwav_ch_dsb[chanNum])
			{
				lp_lwav_ch_dsb[chanNum]->Release();
				lp_lwav_ch_dsb[chanNum] = NULL;
				// mem_del(lwav_fileptr[chanNum]);	// delete lwav_fileptr[chanNum];
				delete lwav_fileptr[chanNum];
				lwav_fileptr[chanNum] = NULL;
			}
			// found an idle channel, create DirectSoundBuffer
			hr = lp_direct_sound->CreateSoundBuffer(&dsbDesc, &lpDsb, NULL);
			if (DS_OK != hr)
			{
				// failed!
				err_now("Cannot create Stream DirectSoundBuffer");
				delete filePtr;
				return 0;
			}
			lp_lwav_ch_dsb[chanNum] = lpDsb;
			lwav_fileptr[chanNum] = filePtr;			// no need to delete filePtr any more
			lwav_bank[chanNum] = 0;
			lwav_bufsiz[chanNum] = OptBufferSize;
			break;
		}
	if( chanNum >= MAX_LONG_WAV_CH)
	{
		delete filePtr;
		return 0;
	}
	// Note : if not found, just play the sound, don't play the sound
	// increase MAX_LONG_WAV_CH
	
	//------- copy sound data to DirectSoundBuffer--------//
	// unlock vga_front
	VgaFrontLock vgaLock;

	// lock the buffer first
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	LPVOID lpvPtr2;
	DWORD dwBytes2;

	// load wave data into buffer
	// load data before lock DirectSoundBuffer in case the wave file
	// is very short
	DWORD startPos = filePtr->file_pos();
	if( !filePtr->file_read(wav_buf, OptBufferSize*LWAV_BANKS))
	{
		// file error
		err_now("Missing wave file");
		return 0;
	}
	DWORD readStreamSize = filePtr->file_pos() - startPos;
	DWORD playFlag = DSBPLAY_LOOPING;
	hr = lpDsb->Lock(0, OptBufferSize*LWAV_BANKS, &lpvPtr1, &dwBytes1,
		&lpvPtr2, &dwBytes2, 0);
	if(DS_OK != hr)
	{
		// fail to lock
		err_now("Cannot lock DirectSoundBuffer");
		return 0;
	}

	// write to pointers
	memcpy(lpvPtr1, wav_buf, MIN(dwBytes1, readStreamSize));
	if( dwBytes1 > readStreamSize )
	{	// end of file, fill the remaining with zero
		memset((char *)lpvPtr1+readStreamSize, 0, dwBytes1 - readStreamSize);
		playFlag &= ~DSBPLAY_LOOPING;
	}
	else
	{
		readStreamSize -= dwBytes1;
		if(lpvPtr2 && dwBytes2 > 0)
		{
			memcpy(lpvPtr2, wav_buf+dwBytes1, MIN(dwBytes2, readStreamSize));
			if( dwBytes2 > readStreamSize )
			{ // end of file, fill the remaining with zero
				memset((char *)lpvPtr2+readStreamSize, 0 , dwBytes2 - readStreamSize);
				playFlag &= ~DSBPLAY_LOOPING;
			}
		}
	}

	// unlock data back
	hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	if(DS_OK != hr)
	{
		// fail to unlock
		err_now("Cannot unlock DirectSoundBuffer");
		return 0;
	}

	//------- Set volume -----------//
	lpDsb->SetVolume(dsVolume.ds_vol);
	lpDsb->SetPan(dsVolume.ds_pan);

	//------- Play wav file --------//
	if(lpDsb->Play(0, 0, playFlag) != DS_OK)
	{
		// fail to play
		err_now("Cannot play DirectSoundBuffer");
		return 0;
	}
	run_yield = 1;
	return lwav_serial_no[chanNum] = assign_serial(max_lwav_serial_no);
}
//------- End of function Audio::play_long_wav ----------//

//------- Begin of function Audio::stop_long_wav ------------//
//
// stop a short sound effect started by play_long_wav
//
// <int>        the serial no returned by play_long_wav
//
// return 1 - channel is found and stopped / channel not found
// return 0 - cannot stop the channel
//
int Audio::stop_long_wav(int serial)
{
	for( int chanNum = 0; chanNum < MAX_LONG_WAV_CH; ++chanNum)
	{
		if(lp_lwav_ch_dsb[chanNum] != NULL && lwav_serial_no[chanNum] == serial)
		{
			lp_lwav_ch_dsb[chanNum]->Stop();
			lp_lwav_ch_dsb[chanNum]->Release();
			lp_lwav_ch_dsb[chanNum] = NULL;
			delete lwav_fileptr[chanNum];
			lwav_fileptr[chanNum] = NULL;
			lwav_serial_no[chanNum] = 0;
			return 1;
		}
	}
	return 0;
}
//------- End of function Audio::stop_long_wav ------------//

//------- Begin of function Audio::is_long_wav_playing ------------//
//
// return wheather a short sound effect is stopped
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
int Audio::is_long_wav_playing(int serial)
{
	DWORD dsbStatus;
	for( int chanNum = 0; chanNum < MAX_LONG_WAV_CH; ++chanNum)
	{
		if(lp_lwav_ch_dsb[chanNum] != NULL && lwav_serial_no[chanNum] == serial
			&& lp_lwav_ch_dsb[chanNum]->GetStatus(&dsbStatus) == DS_OK )
		{
			return dsbStatus & DSBSTATUS_PLAYING;
		}
	}
	return 0;
}
//------- End of function Audio::is_long_wav_playing ------------//


//------- Begin of function Audio::play_loop_wav -------//
//
// Play digitized wav from the wav resource file
//
// <char*>        wavName = name of the wav in the resource file
// int				repeatOffset = offset of wave data to play on repeat
//											i.e. 0 to start of wave data
//
// return : <int> channel number (1 - MAX_LOOP_WAV_CH)
//          0     not played
//
int	Audio::play_loop_wav(const char *wavName, int repeatOffset, DsVolume dsVolume)
{
	if( !wav_init_flag || !wav_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	//-------- Load wav file header-------//
	DWORD wavDataOffset,wavDataLength;

	// File* filePtr = (File *)mem_add(sizeof(File));		// new File;
	File* filePtr = new File;


	if(!filePtr->file_open(wavName,0,0))
	{
		char errmsg[60];
		sprintf(errmsg, "Cannot open %s", wavName);
		box.msg(errmsg);
		delete filePtr;
		return 0;
	}

	if( !filePtr )
		return 0;

	// load small part of the wave file (first 128 bytes) enough to hold
	// the hold header
	if( !filePtr->file_read( wav_buf, 128 ) )
	{
		delete filePtr;
		return 0;
	}

	// determine the wave data offset
	char * dataTag = wavefile_data(wav_buf);
	if (!dataTag)
	{
		err_now("Invalid wave file format");
		delete filePtr;
		return 0;		// invalid RIFF WAVE format
	}
	wavDataOffset = (dataTag - wav_buf) + 4 + sizeof(DWORD);
	wavDataLength = *(DWORD *)(dataTag+4);

	// seek to the start of wave data
	long temp1 = filePtr->file_seek(wavDataOffset);

	WORD OptBufferSize=LOOPWAV_STREAM_BUFSIZ;

	//------- Create DirectSoundBuffer to store a wave ------//
	LPDIRECTSOUNDBUFFER lpDsb;
	DSBUFFERDESC dsbDesc;
	HRESULT hr;
	DWORD dsbStatus;

		// set up DSBUFFERDESC structure
	memset(&dsbDesc, 0, sizeof(DSBUFFERDESC));	// zero it out
	dsbDesc.dwSize = sizeof(DSBUFFERDESC);
	dsbDesc.dwFlags = DSBCAPS_CTRLDEFAULT;			// Need defaul controls (pan, volume, frequency)
	dsbDesc.dwBufferBytes = OptBufferSize * LWAV_BANKS;
	dsbDesc.lpwfxFormat = (LPWAVEFORMATEX) (wav_buf+0x14);
	// ------- assign buffer to a channel number ----------//
	lpDsb = NULL;
	int chanNum;
	for( chanNum = 0; chanNum < MAX_LOOP_WAV_CH; ++chanNum)
		if(lp_loop_ch_dsb[chanNum] == NULL || (lp_loop_ch_dsb[chanNum]->GetStatus(&dsbStatus),
			!(dsbStatus & DSBSTATUS_PLAYING)))
		{
			if(lp_loop_ch_dsb[chanNum])
			{
				lp_loop_ch_dsb[chanNum]->Release();
				lp_loop_ch_dsb[chanNum] = NULL;
				// mem_del(loopwav_fileptr[chanNum]);	// delete lwav_fileptr[chanNum];
				delete loopwav_fileptr[chanNum];
				loopwav_fileptr[chanNum] = NULL;
			}
			// found an idle channel, create DirectSoundBuffer
			hr = lp_direct_sound->CreateSoundBuffer(&dsbDesc, &lpDsb, NULL);
			if (DS_OK != hr)
			{
				// failed!
				err_now("Cannot create Stream DirectSoundBuffer");
				return 0;
			}
			lp_loop_ch_dsb[chanNum] = lpDsb;
			loopwav_fileptr[chanNum] = filePtr;			// no need to delete filePtr any more
			loopwav_bank[chanNum] = 0;
			repeat_offset[chanNum] = wavDataOffset + repeatOffset;
			loopwav_fade_rate[chanNum] = 0;
			break;
		}
	if( chanNum >= MAX_LOOP_WAV_CH)
	{
		delete filePtr;
		return 0;
	}
	// Note : if not found, just play the sound, don't play the sound
	
	//------- copy sound data to DirectSoundBuffer--------//
	// unlock vga_front
	VgaFrontLock vgaLock;

	// lock the buffer first
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	LPVOID lpvPtr2;
	DWORD dwBytes2;

	// load wave data into buffer
	// load data before lock DirectSoundBuffer in case the wave file
	// is very short
	DWORD startPos = filePtr->file_pos();
	if( !filePtr->file_read(wav_buf, OptBufferSize*LOOPWAV_BANKS))
	{
		// file error
		err_now("Missing wave file");
		return 0;
	}
	DWORD readStreamSize = filePtr->file_pos() - startPos;
	DWORD playFlag = DSBPLAY_LOOPING;
	hr = lpDsb->Lock(0, OptBufferSize*LOOPWAV_BANKS, &lpvPtr1, &dwBytes1,
		&lpvPtr2, &dwBytes2, 0);
	if(DS_OK != hr)
	{
		// fail to lock
		err_now("Cannot lock DirectSoundBuffer");
		return 0;
	}

	// write to pointers, assume wave file repeating size is
	// larger than OptBufferSize * LOOPWAV_BANKS
	memcpy(lpvPtr1, wav_buf, MIN(dwBytes1, readStreamSize));
	if( dwBytes1 < readStreamSize )
	{
		readStreamSize -= dwBytes1;
		if(lpvPtr2 && dwBytes2 > 0)
		{
			memcpy(lpvPtr2, wav_buf+dwBytes1, MIN(dwBytes2, readStreamSize));
		}
	}

	// unlock data back
	hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	if(DS_OK != hr)
	{
		// fail to unlock
		err_now("Cannot unlock DirectSoundBuffer");
		return 0;
	}

	//------- Set volume -----------//
	lpDsb->SetVolume(dsVolume.ds_vol);
	lpDsb->SetPan(dsVolume.ds_pan);

	//------- Play wav file --------//
	if(lpDsb->Play(0, 0, playFlag) != DS_OK)
	{
		// fail to play
		err_now("Cannot play DirectSoundBuffer");
		return 0;
	}
	run_yield = 1;
	return chanNum+1;
}
//------- End of function Audio::play_loop_wav ---------//


//------- Begin of function Audio::volume_loop_wav -------//
void	Audio::volume_loop_wav(int ch, DsVolume dsVolume)
{
	int chanNum = ch-1;
	if( chanNum < 0 || chanNum >= MAX_LOOP_WAV_CH)
		return;
	if(lp_loop_ch_dsb[chanNum])
	{
		lp_loop_ch_dsb[chanNum]->SetVolume(dsVolume.ds_vol);
		lp_loop_ch_dsb[chanNum]->SetPan(dsVolume.ds_pan);
		
		// stop fading
		loopwav_fade_rate[chanNum] = 0;
	}
}
//------- End of function Audio::volume_loop_wav -------//


//------- Begin of function Audio::fade_out_loop_wav -------//
//
// <int> fadeRate, time for volume 100 wave drop to slience
//
void Audio::fade_out_loop_wav(int ch, int fadeRate)
{
	int chanNum = ch-1;
	if( chanNum < 0 || chanNum >= MAX_LOOP_WAV_CH)
		return;
	if(lp_loop_ch_dsb[chanNum])
	{
		loopwav_fade_rate[chanNum] = fadeRate;
		loopwav_fade_time[chanNum] = m.get_time();
	}
}
//------- End of function Audio::fade_out_loop_wav -------//


//------- Begin of function Audio::get_loop_wav_volume -------//
DsVolume Audio::get_loop_wav_volume(int ch)
{
	int chanNum = ch-1;
	if( chanNum < 0 || chanNum >= MAX_LOOP_WAV_CH)
	{
		RelVolume rel = RelVolume(0,0);
		return DsVolume(rel);
	}

	LONG volume;
	LONG pan;
	LPDIRECTSOUNDBUFFER lpDsb= lp_loop_ch_dsb[chanNum];
	if( lpDsb && lpDsb->GetVolume(&volume) == DS_OK &&
		lpDsb->GetPan(&pan) == DS_OK )
	{
		return DsVolume(volume, pan);
	}
	RelVolume rel = RelVolume(0,0);
	return DsVolume(rel);
}
//------- End of function Audio::get_loop_wav_volume -------//


//------- Begin of function Audio::is_loop_wav_fading -------//
int Audio::is_loop_wav_fading(int ch)
{
	int chanNum = ch-1;
	if( chanNum < 0 || chanNum >= MAX_LOOP_WAV_CH)
		return 0;

	return lp_loop_ch_dsb[chanNum] && loopwav_fade_rate[chanNum];
}
//------- End of function Audio::is_loop_wav_fading -------//


//------- Begin of function Audio::yield ---------------//
void	Audio::yield()
{
#ifndef WIN32
	// unlock vga_front
	VgaFrontLock vgaLock;
#endif

	if( !run_yield)
		return;

	run_yield = 0;			// suspend recursive Audio::yield();

#ifdef WIN32
	// unlock vga_front
	VgaFrontLock vgaLock;
#endif

	// set break point beyond this point
	int i;
	for(i = 0; i < MAX_LONG_WAV_CH; ++i)
	{
		if( !lp_lwav_ch_dsb[i] )
			continue;

		// if a wav is not play, or buffer lost, stop it
		LPDIRECTSOUNDBUFFER& lpDsb = lp_lwav_ch_dsb[i];
		DWORD dsbStatus;
		if( lpDsb->GetStatus(&dsbStatus) != DS_OK)
			err_here();
		if( !(dsbStatus & DSBSTATUS_PLAYING) || 
			(dsbStatus & DSBSTATUS_BUFFERLOST) && lpDsb->Restore() != DS_OK )
		{
			lpDsb->Stop();
			lpDsb->Release();
			lpDsb = NULL;
			// mem_del(lwav_fileptr[i]);
			delete lwav_fileptr[i];
			lwav_fileptr[i] = NULL;
			continue;
		}

		char writeTooFast = 1;

		// buffer lost, succeeded in restoring
		if( dsbStatus & DSBSTATUS_BUFFERLOST )
		{
			writeTooFast = 0;
		}
		else
		{
		// perform flow control
			DWORD tmpPlayCursor, tmpWriteCursor;
			if( lpDsb->GetCurrentPosition(&tmpPlayCursor, &tmpWriteCursor) == DS_OK)
			{
				writeTooFast = ((short)(tmpPlayCursor / lwav_bufsiz[i]) == lwav_bank[i]);
			}
		}
		
		if(!writeTooFast)
		{
			// lock a channel for lwav_bufsiz[i]
			LPVOID lpvPtr1;
			DWORD dwBytes1;
			LPVOID lpvPtr2;
			DWORD dwBytes2;
			File *filePtr = lwav_fileptr[i];
			HRESULT hr = lpDsb->Lock(lwav_bank[i]*lwav_bufsiz[i], lwav_bufsiz[i],
				&lpvPtr1, &dwBytes1,	&lpvPtr2, &dwBytes2, 0);
			lwav_bank[i] = (lwav_bank[i] + 1) % LWAV_BANKS;	// next bank to fill
			if(DS_OK != hr)
			{
				// fail to lock
				err_now("Cannot lock DirectSoundBuffer");
				run_yield = 1;
				return;
			}

			long startPos;
			long readStreamSize;
			DWORD playFlag = DSBPLAY_LOOPING;

			// write to pointers
			startPos = filePtr->file_pos();

//			filePtr->file_read(wav_buf, dwBytes1);
//			readStreamSize = filePtr->file_pos() - startPos;	// bytes read in
//			memcpy(lpvPtr1, wav_buf, readStreamSize);
			filePtr->file_read(lpvPtr1, dwBytes1);
			readStreamSize = filePtr->file_pos() - startPos;	// bytes read in

			if((long)dwBytes1 > readStreamSize)
			{	// end of file, fill the remaining with zero
//				memset((char *)lpvPtr1+readStreamSize, 0, dwBytes1 - readStreamSize);
				playFlag &= ~DSBPLAY_LOOPING;		// clear DSBPLAY_LOOPING
			}
			else
			{
				if( lpvPtr2 && dwBytes2 > 0)
				{
					startPos = filePtr->file_pos();
					filePtr->file_read(lpvPtr2, dwBytes2);
					readStreamSize = filePtr->file_pos() - startPos;	// bytes read in
					if((long)dwBytes2 > readStreamSize)
					{	// end of file
//						memset((char *)lpvPtr2+readStreamSize, 0, dwBytes2 - readStreamSize);
						playFlag &= ~DSBPLAY_LOOPING;		// clear DSBPLAY_LOOPING
					}
				}			
			}

			// unlock data back
			hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
			if(DS_OK != hr)
			{
				// fail to unlock
				err_now("Cannot unlock DirectSoundBuffer");
				run_yield = 1;
				return;
			}
			// load file into a channel, on last stream, don't loop back
			//------- Play wav file --------//
			if(lpDsb->Play(0, 0, playFlag) != DS_OK)
			{
			// fail to play
				err_now("Cannot play DirectSoundBuffer");
				run_yield = 1;
				return;
			}
		}
	}

	for(i = 0; i < MAX_LOOP_WAV_CH; ++i)
	{
		if( !lp_loop_ch_dsb[i] )
			continue;

		// if a channel is not playing, or can't restore release it
		LPDIRECTSOUNDBUFFER& lpDsb = lp_loop_ch_dsb[i];
		DWORD dsbStatus;
		if( lpDsb->GetStatus(&dsbStatus) != DS_OK )
			err_here();
		if( !(dsbStatus & DSBSTATUS_PLAYING) ||
			(dsbStatus & DSBSTATUS_BUFFERLOST) && lpDsb->Restore() != DS_OK )
		{
			lpDsb->Stop();
			lpDsb->Release();
			lpDsb = NULL;
			// mem_del(loopwav_fileptr[i]);
			delete loopwav_fileptr[i];
			loopwav_fileptr[i] = NULL;
			continue;
		}

		char writeTooFast = 1;

		// buffer lost, succeeded in restoring
		if( dsbStatus & DSBSTATUS_BUFFERLOST )
		{
			writeTooFast = 0;
		}
		else
		{
			// perform flow control
			DWORD tmpPlayCursor, tmpWriteCursor;
			if( lpDsb->GetCurrentPosition(&tmpPlayCursor, &tmpWriteCursor) == DS_OK)
			{
				writeTooFast = ((short)(tmpPlayCursor / LOOPWAV_STREAM_BUFSIZ) == loopwav_bank[i]);
			}

		}
		
		if(!writeTooFast)
		{
			// lock a channel for loopwav_bufsiz[i]
			LPVOID lpvPtr1;
			DWORD dwBytes1;
			LPVOID lpvPtr2;
			DWORD dwBytes2;
			File *filePtr = loopwav_fileptr[i];
			HRESULT hr = lpDsb->Lock(loopwav_bank[i]*LOOPWAV_STREAM_BUFSIZ, LOOPWAV_STREAM_BUFSIZ,
				&lpvPtr1, &dwBytes1,	&lpvPtr2, &dwBytes2, 0);
			loopwav_bank[i] = (loopwav_bank[i] + 1) % LOOPWAV_BANKS;	// next bank to fill
			if(DS_OK != hr)
			{
				// fail to lock
				err_now("Cannot lock DirectSoundBuffer");
				run_yield = 1;
				return;
			}

			long startPos;
			long readStreamSize;
			DWORD playFlag = DSBPLAY_LOOPING;

			// write to pointers
			startPos = filePtr->file_pos();
			filePtr->file_read(lpvPtr1, dwBytes1);
			readStreamSize = filePtr->file_pos() - startPos;	// bytes read in

			if((long)dwBytes1 > readStreamSize)
			{	// end of file, seek to beginning and read again
				filePtr->file_seek(repeat_offset[i]);
				filePtr->file_read((char *)lpvPtr1+readStreamSize, dwBytes1-readStreamSize);
			}

			// unlock data back
			hr = lpDsb->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
			if(DS_OK != hr)
			{
				// fail to unlock
				err_now("Cannot unlock DirectSoundBuffer");
				run_yield = 1;
				return;
			}

			// set volume if fading
			if( loopwav_fade_rate[i] )
			{
				DWORD nextFadeTime = m.get_time();
				LONG	volume;
				if( DS_OK == (hr = lpDsb->GetVolume(&volume)) )
				{
					// calculate new volume
					volume -= (nextFadeTime - loopwav_fade_time[i]) * loopwav_fade_rate[i];
					if( volume < DSBVOLUME_MIN )
						volume = DSBVOLUME_MIN;
					else if( volume > DSBVOLUME_MAX )
						volume = DSBVOLUME_MAX;
					if( DS_OK == (hr = lpDsb->SetVolume(volume)) )
					{
						loopwav_fade_time[i] = nextFadeTime;
					}
				}
			}

			// load file into a channel, on last stream, don't loop back
			//------- Play wav file --------//
			if(lpDsb->Play(0, 0, playFlag) != DS_OK)
			{
			// fail to play
				err_now("Cannot play DirectSoundBuffer");
				run_yield = 1;
				return;
			}
		}
	}

	run_yield = 1;		// resume Audio::yield();
}

//------- End of function Audio::yield ---------------//


//------- Begin of function Audio::stop_wav -------//
//
void Audio::stop_wav()
{
	if( !wav_init_flag || !wav_flag )
		return;

	// ---------- stop all short wave  ------------- //
	int i;
	for(i = 0; i < MAX_WAV_CHANNEL; ++i)
		if(lp_wav_ch_dsb[i])
			{
			lp_wav_ch_dsb[i]->Stop();
			lp_wav_ch_dsb[i]->Release();
			lp_wav_ch_dsb[i] = NULL;
			}

	// ----------- stop all long wave ------------- //
	stop_long_wav();

	// ------------ stop all loop wave ------------//
	for(i = 1; i <= MAX_LOOP_WAV_CH; ++i)
	{
		stop_loop_wav(i);
	}

}
//------- End of function Audio::stop_wav -------//

//------- Begin of function Audio::stop_long_wav -------//
//
void Audio::stop_long_wav()
{
	if( !wav_init_flag || !wav_flag )
		return;

	for(int i = 0; i < MAX_LONG_WAV_CH; ++i)
		if(lp_lwav_ch_dsb[i])
		{
			lp_lwav_ch_dsb[i]->Stop();
			lp_lwav_ch_dsb[i]->Release();
			lp_lwav_ch_dsb[i] = NULL;
			// mem_del(lwav_fileptr[i]);
			delete lwav_fileptr[i];
			lwav_fileptr[i] = NULL;
		}
}
//------- End of function Audio::stop_long_wav -------//


//------- Begin of function Audio::stop_loop_wav -------//
void	Audio::stop_loop_wav(int ch)
{
	int chanNum = ch-1;
	if( chanNum < 0 || chanNum >= MAX_LOOP_WAV_CH)
		return;
	if(lp_loop_ch_dsb[chanNum])
	{
		lp_loop_ch_dsb[chanNum]->Stop();
		lp_loop_ch_dsb[chanNum]->Release();
		lp_loop_ch_dsb[chanNum] = NULL;
		// mem_del(loopwav_fileptr[chanNum]);	// delete lwav_fileptr[chanNum];
		delete loopwav_fileptr[chanNum];
		loopwav_fileptr[chanNum] = NULL;
	}
}
//------- End of function Audio::stop_loop_wav ---------//


//------- Begin of function Audio::play_cd -------//
//
// <int> trackId - the id. of the CD track to play.
//
int Audio::play_cd(int trackId, int volume)
{
	if( !cd_init_flag || !cd_flag )
		return 0;

	// ###### begin Gilbert 3/10 ########//
	MCIERROR mciError;
	DWORD maxTrack = 99;

	// Get the number of tracks; 
	// limit to number that can be displayed (20).
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	mciError = mciSendCommand(mci_open.wDeviceID, MCI_STATUS, 
		MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms);
	if( !mciError )
		maxTrack = mciStatusParms.dwReturn;

	//--- Send an MCI_PLAY command to the CD Audio driver ---//

	mci_open.lpstrDeviceType = (LPCSTR) MCI_DEVTYPE_CD_AUDIO;

	mci_play.dwFrom = MCI_MAKE_TMSF(trackId , 0, 0, 0);
	mci_play.dwTo 	= MCI_MAKE_TMSF(trackId+1, 0, 0, 0);

	DWORD cmdFlag = MCI_FROM;
	if( (DWORD) trackId < maxTrack )
		cmdFlag |= MCI_TO;				// if MCI_TO is missing, play until the end
	mciError = mciSendCommand( mci_open.wDeviceID, MCI_PLAY, cmdFlag,
		(DWORD) (LPVOID) &mci_play);
	if( mciError )
		return 0;

	return 1;
	// ###### end Gilbert 3/10 ########//
}
//------- End of function Audio::play_cd -------//


//------- Begin of function Audio::stop_cd -------//
//
void Audio::stop_cd()
{
	if( !cd_init_flag || !cd_flag )
		return;

	DWORD dwResult;
	dwResult = mciSendCommand(mci_open.wDeviceID, MCI_STOP,
		MCI_WAIT, (DWORD)(LPVOID)NULL);
}
//------- End of function Audio::stop_cd -------//


//------- Begin of function Audio::is_mid_playing -------//
//
int Audio::is_mid_playing()
{
	if( !mid_init_flag || !mid_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	//... insert code here ...//

	return 0;
}
//------- End of function Audio::is_mid_playing -------//


//------- Begin of function Audio::is_wav_playing -------//
//
int Audio::is_wav_playing()
{
	int playingChannelCount = 0;
	DWORD dwStatus;
	if( !wav_init_flag || !wav_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	//------ find any wav channel is playing -----//
	// update lp_wav_ch_dsb[x] if necessary
	for(int ch=0; ch < MAX_WAV_CHANNEL; ++ch)
		if( lp_wav_ch_dsb[ch])
			if( lp_wav_ch_dsb[ch]->GetStatus(&dwStatus) == DS_OK)
				if (dwStatus & DSBSTATUS_PLAYING )
					// a channel is playing
					playingChannelCount++;
				else
					// is not playing, clear it
					lp_wav_ch_dsb[ch] = NULL;
			else
				// GetStatus not ok, clear it
				lp_wav_ch_dsb[ch] = NULL;
		else
		{
			// nothing
		}

	return (playingChannelCount > 0);
}
//------- End of function Audio::is_wav_playing -------//


//------- Begin of function Audio::is_cd_playing -------//
//
int Audio::is_cd_playing()
{
	if( !cd_init_flag || !cd_flag )   // a initialized and workable midi device can be disabled by user setting
		return 0;

	MCI_STATUS_PARMS status;
	DWORD dwResult;

	//
	// get the current status
	//

	status.dwItem = MCI_STATUS_MODE;
	dwResult = mciSendCommand(mci_open.wDeviceID,	MCI_STATUS,	
		MCI_WAIT | MCI_STATUS_ITEM, (DWORD)(LPVOID)&status);
	if (dwResult == 0)
		return status.dwReturn == MCI_MODE_PLAY;
	return 0;
}
//------- End of function Audio::is_cd_playing -------//


//----------------- Begin of Audio::toggle_mid -----------------//
//
void Audio::toggle_mid(int midFlag)
{
	if( !midFlag )
		stop_mid();

	mid_flag = midFlag;
}
//------------------- End of Audio::toggle_mid ------------------//


//----------------- Begin of Audio::toggle_wav -----------------//
//
void Audio::toggle_wav(int wavFlag)
{
	if( !wavFlag )
		stop_wav();

	wav_flag = wavFlag;
}
//------------------- End of Audio::toggle_wav ------------------//


//----------------- Begin of Audio::toggle_cd -----------------//
//
void Audio::toggle_cd(int cdFlag)
{
	if( !cdFlag )
		stop_cd();

	cd_flag = cdFlag;
}
//------------------- End of Audio::toggle_cd ------------------//


//-------------- Begin of Audio::set_mid_volume -------------//
//
// Set mid volume
//
// <int> midVolume = mid volume, 0-100
//
void Audio::set_mid_volume(int midVolume)
{
	if( !mid_init_flag )
		return;

	//.. insert code here ...//

}
//--------------- End of Audio::set_mid_volume --------------//


//-------------- Begin of Audio::set_wav_volume -------------//
//
// Set wav volume
//
// <int> wavVolume = wav volume, 0-100
//
void Audio::set_wav_volume(int wavVolume)
{
	if( !wav_init_flag )
		return;

	LONG dsVolume;
	long dsVolDiff = (wavVolume - wav_volume) * 100;

	// change volume for all channels
	int i;
	for( i = 0; i < MAX_WAV_CHANNEL; ++i)
	{
		if( lp_wav_ch_dsb[i] && 
			lp_wav_ch_dsb[i]->GetVolume(&dsVolume) == DS_OK)
		{
			dsVolume += dsVolDiff;
			if( dsVolume > DSBVOLUME_MAX )
				dsVolume = DSBVOLUME_MAX;
			if( dsVolume < DSBVOLUME_MIN )
				dsVolume = DSBVOLUME_MIN;
			lp_wav_ch_dsb[i]->SetVolume(dsVolume);
		}
	}

	for( i = 0; i < MAX_LONG_WAV_CH; ++i)
	{
		if( lp_lwav_ch_dsb[i] &&
			lp_lwav_ch_dsb[i]->GetVolume(&dsVolume) == DS_OK)
		{
			dsVolume += dsVolDiff;
			if( dsVolume > DSBVOLUME_MAX )
				dsVolume = DSBVOLUME_MAX;
			if( dsVolume < DSBVOLUME_MIN )
				dsVolume = DSBVOLUME_MIN;
			lp_lwav_ch_dsb[i]->SetVolume(dsVolume);
		}
	}

	for( i = 0; i < MAX_LOOP_WAV_CH; ++i)
	{
		if( lp_loop_ch_dsb[i] &&
			lp_loop_ch_dsb[i]->GetVolume(&dsVolume) == DS_OK)
		{
			dsVolume += dsVolDiff;
			if( dsVolume > DSBVOLUME_MAX )
				dsVolume = DSBVOLUME_MAX;
			if( dsVolume < DSBVOLUME_MIN )
				dsVolume = DSBVOLUME_MIN;
			lp_loop_ch_dsb[i]->SetVolume(dsVolume);
		}
	}

	wav_volume = wavVolume;
}
//--------------- End of Audio::set_wav_volume --------------//


//--------------- Begin of Audio::get_wav_volume --------------//
int Audio::get_wav_volume() const
{
  return wav_volume;
}
//--------------- End of Audio::get_wav_volume --------------//


//-------------- Begin of Audio::set_cd_volume -------------//
//
// Set cd volume
//
// <int> cdVolume = cd volume, 0-100
//
void Audio::set_cd_volume(int cdVolume)
{
	if( !cd_init_flag )
		return;

	//.. insert code here ...//

}
//--------------- End of Audio::set_cd_volume --------------//


//-------------- Begin of function Audio::assign_serial ----------//
int Audio::assign_serial(int &s)
{
	if( s == INT_MAX)
		return s = 1;
	return ++s;
}
//-------------- End of function Audio::assign_serial ----------//


// ------------ Begin of function Audio::volume_long_wav -------//
void Audio::volume_long_wav(int serial, DsVolume dsVolume)
{
	if( is_long_wav_playing(serial) )
	{
		for( int chanNum = 0; chanNum < MAX_LONG_WAV_CH; ++chanNum)
		{
			if(lp_lwav_ch_dsb[chanNum] != NULL && lwav_serial_no[chanNum] == serial )
			{
				lp_lwav_ch_dsb[chanNum]->SetVolume(dsVolume.ds_vol);
				// lp_lwav_ch_dsb[chanNum]->SetPan(dsVolume.ds_pan);
				break;
			}
		}
	}
}
// ------------ End of function Audio::volume_long_wav -------//

