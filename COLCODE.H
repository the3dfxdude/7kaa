// Filename    : COLCODE.H
// Description : C include file for defining special color
//               see/update also COLCODE.INC for assembly

// for transparent code repeated for 1 to UNIQUE_REPEAT_CODE_NUM times,
// write FEW_TRANSPARENT_CODE(repeated_times)
// for transparent code repeated for UNIQUE_REPEAT_CODE_NUM+1 to 255 times,
// write two bytes, MANY_TRANSPARENT_CODE and repeated_times

#define TRANSPARENT_CODE			255
#define UNIQUE_REPEAT_CODE_NUM	  7		// total no. of bytes used by transparent pixels and compressed transparent pixels is 7+1 (the last 1 is the first byte of the 2 bytes compression code)
#define FEW_TRANSPARENT_CODE(n) (0xFF-n+1)
#define MANY_TRANSPARENT_CODE   0xf8
#define MIN_TRANSPARENT_CODE    0xf8
#define MAX_TRANSPARENT_CODE    0xff

#define SHADOW_CODE      		  0x00
#define OUTLINE_CODE            0xf2
#define OUTLINE_SHADOW_CODE     0xf3
