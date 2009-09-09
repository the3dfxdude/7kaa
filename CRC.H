// Filename    : CRC.H
// Description : header of assembly function crc.asm

#ifndef __CRC_H
#define __CRC_H

typedef unsigned char CRC_TYPE;
const CRC_LEN = sizeof(CRC_TYPE);

extern "C"
{
	CRC_TYPE _stdcall crc8(unsigned char *, int);
}


#endif
