/***********************************************************************
 *
 *  DMOUSE.VXD - DirectMouse
 *
 *  this VxD lets a Win32 app get direct mouse input, without the windows
 *  user interface filtering/mapping them.
 *
 *  multiple apps can open DMOUSE.VXD, but it is assumes only
 *  one app will be talking to DMOUSE at a time.
 *
 *  example:
 *      #include "dmouse.h"
 *      h = DMouseOpen();
 *      ....
 *
 *      DMOUSE_STATE dm;
 *      DMouseGetState(h, &dm);
 *
 *      if (dm.mouse_dx || dm.mouse_dy)
 *          mouse has moved
 *
 *      if (dm.mouse_state & MOUSE_BUTTON_DOWN_MASK)
 *          mouse has gone down (from last call)
 *
 *      if (dm.mouse_state & MOUSE_BUTTON_UP_MASK)
 *          mouse has gone up (from last call)
 *
 *      if (dm.mouse_state & MOUSE_BUTTON_ASYNC_MASK)
 *          a mouse button is down now!
 *
 *      ....
 *      DMouseClose(h);
 *
 *  ToddLa
 *
 ***********************************************************************/

#ifndef __WINDOWS_
#include <windows.h>
#endif

#define DMOUSE_VXD  "\\\\.\\DMOUSE.VXD"

#define DMOUSE_GET_STATE        1
#define DMOUSE_GET_STATE_PTR    2

/***********************************************************************
 *  DMOUSE_STATE
 *
 *  mouse_dx        - delta (in mickeys) that the mouse has moved in x
 *  mouse_dy        - delta (in mickeys) that the mouse has moved in y
 *  mouse_state     - contains the current state of the mouse buttons
 *                    contains the buttons that have gone up (from last query)
 *                    contains the buttons that have gone down (from last query)
 *
 ***********************************************************************/

struct DMOUSE_STATE
{
    DWORD   mouse_delta_x;
    DWORD   mouse_delta_y;
    DWORD   mouse_state;
};

#define MOUSE_STATE_CHANGE      0x80000000

#define MOUSE_BUTTON_ASYNC_MASK 0x0000000F
#define MOUSE_BUTTON1_ASYNC     0x00000008
#define MOUSE_BUTTON2_ASYNC     0x00000004
#define MOUSE_BUTTON3_ASYNC     0x00000002
#define MOUSE_BUTTON4_ASYNC     0x00000001

#define MOUSE_BUTTON_DOWN_MASK  0x00000F00
#define MOUSE_BUTTON1_DOWN      0x00000800
#define MOUSE_BUTTON2_DOWN      0x00000400
#define MOUSE_BUTTON3_DOWN      0x00000200
#define MOUSE_BUTTON4_DOWN      0x00000100

#define MOUSE_BUTTON_UP_MASK    0x0000F000
#define MOUSE_BUTTON1_UP        0x00008000
#define MOUSE_BUTTON2_UP        0x00004000
#define MOUSE_BUTTON3_UP        0x00002000
#define MOUSE_BUTTON4_UP        0x00001000

/***********************************************************************
 *
 *  DMouseOpen
 *
 *  open a instance of the DMOUSE VxD and return you a handle
 *
 ***********************************************************************/

__inline HANDLE DMouseOpen()
{
    HANDLE h;

    h = CreateFile(DMOUSE_VXD, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, NULL);

    if (h == INVALID_HANDLE_VALUE)
        return NULL;

    return h;
}

/***********************************************************************
 *
 *  DMouseClose
 *
 *  close a instance of the DMOUSE VxD
 *
 ***********************************************************************/

__inline void DMouseClose(HANDLE h)
{
    if (h != NULL && h != INVALID_HANDLE_VALUE)
        CloseHandle(h);
}

/***********************************************************************
 *
 *  DMouseGetState
 *
 *  read the current mouse state.
 *
 *  this will return the amount the mouse has moved from the last time
 *  you called this service. (mouse_delta_x, mouse_delta_y)
 *
 *  if a button has gone down from the last time you called this
 *  service the associated bit (MOUSE_BUTTONx_DOWN) will be set in
 *  mouse_state
 *
 *  if a button has gone up from the last time you called this
 *  service the associated bit (MOUSE_BUTTONx_UP) will be set in
 *  mouse_state
 *
 *  if a button is down right now the associated bit (MOUSE_BUTTONx_ASYNC)
 *  will be set in mouse_state
 *
 ***********************************************************************/

__inline BOOL DMouseGetState(HANDLE h, DMOUSE_STATE *p)
{
    DWORD cb;

    if (h == NULL || h == INVALID_HANDLE_VALUE)
        return FALSE;

    return DeviceIoControl(h, DMOUSE_GET_STATE,
        NULL, 0, p, sizeof(DMOUSE_STATE), &cb, 0);
}

/***********************************************************************
 *
 *  DMouseGetStatePointer
 *
 *  returns a pointer to the internal "live" MOUSE_STATE
 *
 *  in general it is better to use DMouseGetState, but this
 *  service is useful for "peeking" at the button state quickly
 *
 *  to determine if the mouse state has changed, you need to look
 *  at the MOUSE_STATE_CHANGE bit in mouse_state.
 *
 ***********************************************************************/

__inline DMOUSE_STATE * DMouseGetStatePointer(HANDLE h)
{
    DWORD cb;
    DMOUSE_STATE *p = NULL;

    if (h == NULL || h == INVALID_HANDLE_VALUE)
        return NULL;

    DeviceIoControl(h, DMOUSE_GET_STATE_PTR,
        NULL, 0, &p, sizeof(p), &cb, 0);

    return p;
}
