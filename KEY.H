//Filename    : KEY.H
//Description : Header file for Object Keyboard input handler

#ifndef __KEY_H
#define __KEY_H

//-------------------------------------------------------//
//
// Note : The higher byte is the scan code, and the lower byte is the
//        ascii code.
//
//        It can be used to compare the return code of m1getKey().
//
//        Or for comparing KEYscanCode and KEYascCode.
//        However, before comparsion, if KEYascCode is not 0,
//        then KEYscanCode need to be reset to 0 first.
//
//        e.g. if( KEYascCode )
//                KEYscanCode = 0;
//
//-----------------------------------------------------//


//-----------------------------------------------//

#define   KEY_BACK_SPACE   0x08
#define   KEY_TAB          0x09
#define   KEY_ESC          0x1B

#define   KEY_RETURN       0xD
#define   KEY_HOME         0x4700
#define   KEY_END          0x4F00
#define   KEY_PGUP         0x4900
#define   KEY_PGDN         0x5100
#define   KEY_UP           0x4800
#define   KEY_DOWN         0x5000
#define   KEY_RIGHT        0x4D00
#define   KEY_LEFT         0x4B00
#define   KEY_CENTER       0x4C00    // Extended keyboard only
#define   KEY_SHIFT_TAB    0x0F00
#define   KEY_INS          0x5200
#define   KEY_DEL          0x5300

#define   KEY_F1           0x3B00
#define   KEY_F2           0x3C00
#define   KEY_F3           0x3D00
#define   KEY_F4           0x3E00
#define   KEY_F5           0x3F00
#define   KEY_F6           0x4000
#define   KEY_F7           0x4100
#define   KEY_F8           0x4200
#define   KEY_F9           0x4300
#define   KEY_F10          0x4400
#define   KEY_F11          0x5700
#define   KEY_F12          0x5800

// 104-key keyboard
#define   KEY_LWIN         0xDB00
#define   KEY_RWIN         0xDC00
#define   KEY_APPS         0xDD00


/*
#define   KEY_CTRL_A       0x01
#define   KEY_CTRL_B       0x02
#define   KEY_CTRL_C       0x03
#define   KEY_CTRL_D       0x04
#define   KEY_CTRL_E       0x05
#define   KEY_CTRL_F       0x06
#define   KEY_CTRL_G       0x07
#define   KEY_CTRL_H       0x08
#define   KEY_CTRL_I       0x09
#define   KEY_CTRL_J       0x0A
#define   KEY_CTRL_K       0x0B
#define   KEY_CTRL_L       0x0C
#define   KEY_CTRL_M       0x0D
#define   KEY_CTRL_N       0x0E
#define   KEY_CTRL_O       0x0F
#define   KEY_CTRL_P       0x10
#define   KEY_CTRL_Q       0x11
#define   KEY_CTRL_R       0x12
#define   KEY_CTRL_S       0x13
#define   KEY_CTRL_T       0x14
#define   KEY_CTRL_U       0x15
#define   KEY_CTRL_V       0x16
#define   KEY_CTRL_W       0x17
#define   KEY_CTRL_X       0x18
#define   KEY_CTRL_Y       0x19
#define   KEY_CTRL_Z       0x1A

#define   KEY_CTRL_HOME    0x7700
#define   KEY_CTRL_UP      0x8D00    // Extended only
#define   KEY_CTRL_PGUP    0x8400
#define   KEY_CTRL_LEFT    0x7300
#define   KEY_CTRL_CENTER  0x8F00    // Extended only
#define   KEY_CTRL_RIGHT   0x7400
#define   KEY_CTRL_END     0x7500
#define   KEY_CTRL_DOWN    0x9100
#define   KEY_CTRL_PGDN    0x7600
#define   KEY_CTRL_INS     0x9200
#define   KEY_CTRL_DEL     0x9300

#define   KEY_ALT_1        0x7800
#define   KEY_ALT_2        0x7900
#define   KEY_ALT_3        0x7A00
#define   KEY_ALT_4        0x7B00
#define   KEY_ALT_5        0x7C00
#define   KEY_ALT_6        0x7D00
#define   KEY_ALT_7        0x7E00
#define   KEY_ALT_8        0x7F00
#define   KEY_ALT_9        0x8000
#define   KEY_ALT_0        0x8100
#define   KEY_ALT_MINUS    0x8200
#define   KEY_ALT_EQUAL    0x8300

#define   KEY_ALT_Q        0x1000
#define   KEY_ALT_W        0x1100
#define   KEY_ALT_E        0x1200
#define   KEY_ALT_R        0x1300
#define   KEY_ALT_T        0x1400
#define   KEY_ALT_Y        0x1500
#define   KEY_ALT_U        0x1600
#define   KEY_ALT_I        0x1700
#define   KEY_ALT_O        0x1800
#define   KEY_ALT_P        0x1900

#define   KEY_ALT_A        0x1E00
#define   KEY_ALT_S        0x1F00
#define   KEY_ALT_D        0x2000
#define   KEY_ALT_F        0x2100
#define   KEY_ALT_G        0x2200
#define   KEY_ALT_H        0x2300
#define   KEY_ALT_J        0x2400
#define   KEY_ALT_K        0x2500
#define   KEY_ALT_L        0x2600

#define   KEY_ALT_Z        0x2C00
#define   KEY_ALT_X        0x2D00
#define   KEY_ALT_C        0x2E00
#define   KEY_ALT_V        0x2F00
#define   KEY_ALT_B        0x3000
#define   KEY_ALT_N        0x3100
#define   KEY_ALT_M        0x3200

#define   KEY_CTRL_F1      0x5E00
#define   KEY_CTRL_F2      0x5F00
#define   KEY_CTRL_F3      0x6000
#define   KEY_CTRL_F4      0x6100
#define   KEY_CTRL_F5      0x6200
#define   KEY_CTRL_F6      0x6300
#define   KEY_CTRL_F7      0x6400
#define   KEY_CTRL_F8      0x6500
#define   KEY_CTRL_F9      0x6600
#define   KEY_CTRL_F10     0x6700

#define   KEY_SHIFT_F1     0x5400
#define   KEY_SHIFT_F2     0x5500
#define   KEY_SHIFT_F3     0x5600
#define   KEY_SHIFT_F4     0x5700
#define   KEY_SHIFT_F5     0x5800
#define   KEY_SHIFT_F6     0x5900
#define   KEY_SHIFT_F7     0x5A00
#define   KEY_SHIFT_F8     0x5B00
#define   KEY_SHIFT_F9     0x5C00
#define   KEY_SHIFT_F10    0x5D00

#define   KEY_ALT_F1       0x6800
#define   KEY_ALT_F2       0x6900
#define   KEY_ALT_F3       0x6A00
#define   KEY_ALT_F4       0x6B00
#define   KEY_ALT_F5       0x6C00
#define   KEY_ALT_F6       0x6D00
#define   KEY_ALT_F7       0x6E00
#define   KEY_ALT_F8       0x6F00
#define   KEY_ALT_F9       0x7000
#define   KEY_ALT_F10      0x7100
*/
//-------------------------------------------------//

#endif

