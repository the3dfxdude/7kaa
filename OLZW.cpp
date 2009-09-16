// Filename    : OLZW.CPP
// Description : LZW compression and decompression


#include <ALL.H>
#include <OFILE.H>
#include <OLZW.H>

// --------- define constant ------------//

#define BITS                       15
#define MAX_CODE                   ( ( 1 << BITS ) - 1 )
#define TABLE_SIZE                 35023L
#define END_OF_STREAM              256
#define BUMP_CODE                  257
#define FLUSH_CODE                 258
#define FIRST_CODE                 259
#define UNUSED                     0xffff

static unsigned short BITS_MASK[] = 
{
	0x0000, 
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff, 
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff,
};

// --------- define macro ----------//
#define DICT( i ) dict[i]

// --------- define struct Dictionary ---------//

struct Dictionary
{
	unsigned short code_value;
	unsigned short parent_code;
	unsigned char character;
};


BitStream::BitStream() : bit_offset(0)
{
}

BitStream::~BitStream()
{
}

unsigned short BitStream::input_bits(unsigned stringLen)
{
	bit_offset += stringLen;
	return 0;
}

void BitStream::output_bits(unsigned short stringCode, unsigned stringLen)
{
	bit_offset += stringLen;
}

BitMemStream::BitMemStream(unsigned char *p) : BitStream(), bytePtr(p)
{
}

unsigned short BitMemStream::input_bits(unsigned stringLen)
{
	unsigned char *p = bytePtr + bit_offset / 8;
	int s = bit_offset % 8;
	
	// get longer bits
	unsigned long ul = *(unsigned long *)p >> s;

	(void) BitStream::input_bits(stringLen);

	// mask off leading bits
	return (unsigned short)ul & BITS_MASK[stringLen];
}

void BitMemStream::output_bits(unsigned short stringCode, unsigned stringLen)
{
	// fill low bit first
	unsigned char *p = bytePtr + bit_offset / 8;
	int s = bit_offset % 8;

	// mask off unused bit
	*(unsigned long *)p &= BITS_MASK[s];
	// stringCode &= bitMask[stringCode];

	// shift stringCode and put them to outPtr
	*(unsigned long *)p |= (unsigned long)stringCode << s;

	BitStream::output_bits(stringCode, stringLen);
}


BitFileRead::BitFileRead(File *f) : filePtr(f), last_offset(0)
{
	f->file_read(&residue, sizeof(residue));
}

unsigned short BitFileRead::input_bits(unsigned stringLen)
{
	if( bit_offset + stringLen > (last_offset+sizeof(residue))*8 )
	{
		// find byte to read
		long byteFetch = bit_offset/8 - last_offset;
		if( byteFetch >= sizeof(residue) )		// residue >>= 32 does not change to 0
			residue = 0;
		else
			residue >>= 8*byteFetch;
		filePtr->file_read( sizeof(residue)-byteFetch+(unsigned char *)&residue, byteFetch );
		last_offset += byteFetch;
	}

	err_when( bit_offset + stringLen > (last_offset+sizeof(residue))*8 );
	int s = bit_offset - last_offset*8;
	unsigned long ul = residue >> s;

	(void) BitStream::input_bits(stringLen);

	// mask off leading bits
	return (unsigned short)ul & BITS_MASK[stringLen];
}
	
void BitFileRead::output_bits(unsigned short stringCode, unsigned stringLen)
{
	err_here();
	BitStream::output_bits(stringCode, stringLen);
}


BitFileWrite::BitFileWrite(File *f) : filePtr(f), residue(0), residue_len(0)
{
}

BitFileWrite::~BitFileWrite()
{
	// flush output
	filePtr->file_write(&residue, sizeof(residue));
}

unsigned short BitFileWrite::input_bits(unsigned stringLen)
{
	err_here();
	return BitStream::input_bits(stringLen);
}
	
void BitFileWrite::output_bits(unsigned short stringCode, unsigned stringLen)
{
	if( residue_len + stringLen > sizeof(residue)*8 )
	{
		// number of byte to write, is residue_len / 8
		int byteFlush = residue_len / 8;
		filePtr->file_write( &residue, byteFlush );
		if( byteFlush >= sizeof(residue))			// if byteFlush == 4, residue >>= 32 does not set residue to 0
			residue = 0;
		else
			residue >>= byteFlush * 8;
		residue_len -= byteFlush * 8;
	}
	err_when( residue_len + stringLen > sizeof(residue)*8 );
	residue |= (unsigned long) stringCode << residue_len;
	residue_len += stringLen;

	BitStream::output_bits(stringCode, stringLen);
}


// --------- begin of function Lzw::Lzw -------------//
Lzw::Lzw()
{
	dict = NULL;
	decode_stack = NULL;
}
// --------- end of function Lzw::Lzw -------------//


// --------- begin of function Lzw::~Lzw -------------//
Lzw::~Lzw()
{
	free_storage();
}
// --------- end of function Lzw::~Lzw -------------//


// --------- begin of function Lzw::initialize_storage -------------//
void Lzw::initialize_storage()
{
	if( !dict )
		dict = (Dictionary *) mem_add(sizeof(Dictionary) * TABLE_SIZE);

	if( !decode_stack )
		decode_stack = (unsigned char *)mem_add(sizeof(unsigned char) * TABLE_SIZE);
}
// --------- end of function Lzw::initialize_storage -------------//


// --------- begin of function Lzw::free_storage -------------//
void Lzw::free_storage()
{
	if( decode_stack )
	{
		mem_del(decode_stack);
		decode_stack = NULL;
	}
	if( dict )
	{
		mem_del(dict);
		dict = NULL;
	}
}
// --------- end of function Lzw::free_storage -------------//


// --------- begin of function Lzw::initialize_dictionary -------------//
void Lzw::initialize_dictionary()
{
	unsigned short i;

	for ( i = 0 ; i < TABLE_SIZE ; i++ )
		DICT( i ).code_value = UNUSED;
	next_code = FIRST_CODE;
	// putc( 'F', stdout );
	current_code_bits = 9;
	next_bump_code = 511;
}
// --------- end of function Lzw::initialize_dictionary -------------//

// nul output, to find output size in bits
long Lzw::compress( unsigned char *inPtr, long inByteLen)
{
	BitStream nulStream;
	return basic_compress( inPtr, inByteLen, &nulStream);
}

// compressed data in memory
long Lzw::compress( unsigned char *inPtr, long inByteLen, unsigned char *outPtr)
{
	BitMemStream memStream(outPtr);
	return basic_compress( inPtr, inByteLen, &memStream);
}

// set outPtr to NULL to find the decompressed size
long Lzw::expand( unsigned char *inPtr, long inBitLen, unsigned char *outPtr)
{
	BitMemStream memStream(inPtr);
	return basic_expand( &memStream, outPtr );
}

// compressed data in file
long Lzw::compress( unsigned char *inPtr, long inByteLen, File *outFile)
{
	BitFileWrite fileStream(outFile);
	return basic_compress( inPtr, inByteLen, &fileStream);
}

// set outPtr to NULL to find the decompressed size
long Lzw::expand( File *inFile, unsigned char *outPtr)
{
	BitFileRead fileStream(inFile);
	return basic_expand( &fileStream, outPtr);
}


// --------- begin of function Lzw::basic_compress -------------//
// compress a memory block to another memory block
// <unsigned char *> inPtr           address of decompressed input data
// <long> inByteLen                  length of input data (in byte)
// <BitStream *> outStream           output stream, BitMemStream or BitFileWrite
//
// return in no. of bits, the size of the compressed data
// call free_storage after compress to free allocated space, if it will
// not going to compress/decompress soon
long Lzw::basic_compress( unsigned char *inPtr, long inByteLen, BitStream *outStream)
{
	unsigned char character;
	unsigned short stringCode;
	unsigned short index;

	initialize_storage();
	initialize_dictionary();

	if ( inByteLen == 0 )
		stringCode = END_OF_STREAM;
	else
	{
		stringCode = *inPtr++;
		inByteLen--;
	}

	while ( inByteLen-- > 0)
	{
		character = *inPtr++;
		index = find_child_node( stringCode, character );
		if ( DICT( index ).code_value != UNUSED )
			stringCode = DICT( index ).code_value;
		else
		{
			DICT( index ).code_value = next_code++;
			DICT( index ).parent_code = stringCode;
			DICT( index ).character = character;
			outStream->output_bits( stringCode, current_code_bits );
			stringCode = character;
			if ( next_code > MAX_CODE )
			{
				outStream->output_bits( FLUSH_CODE, current_code_bits );
				initialize_dictionary();
			}
			else if ( next_code > next_bump_code )
			{
				outStream->output_bits( BUMP_CODE, current_code_bits );
				current_code_bits++;
				next_bump_code <<= 1;
				next_bump_code |= 1;
			}
		}
	}

	outStream->output_bits( stringCode, current_code_bits );
	outStream->output_bits( END_OF_STREAM, current_code_bits);

	//	free_storage();
	return outStream->bit_offset;
}
// --------- end of function Lzw::basic_compress -------------//


// --------- begin of function Lzw::basic_expand -------------//
// decompress a memory block to another memory block
// <BitStream *> inStream            bit stream, can be BitMemStream or BitFileRead
// <unsigned char *> outPtr          address of decompressed output data
//                                   (NULL to find the size of decompressed data)
//
// return the no. of byte of the decompressed data
// call free_storage after decompress to free allocated space, if it will
// not going to compress/decompress soon
long Lzw::basic_expand( BitStream *inStream, unsigned char *outPtr)
{
	unsigned short newCode;
	unsigned short oldCode;
	unsigned char character;
	unsigned int count;

	long outByteLen = 0;

	initialize_storage();
	for ( ; ; )
	{
		initialize_dictionary();
		oldCode = inStream->input_bits( current_code_bits );

		if ( oldCode == END_OF_STREAM )
		{
			// free_storage();
			return outByteLen;
		}
		character = (unsigned char) oldCode;
		if( outPtr )
			outPtr[outByteLen] = character;
		outByteLen++;

		for ( ; ; )
		{
			newCode = inStream->input_bits( current_code_bits );
			if ( newCode == END_OF_STREAM )
			{
				// free_storage();
				return outByteLen;
			}
			if ( newCode == FLUSH_CODE )
				break;
			if ( newCode == BUMP_CODE )
			{
				current_code_bits++;
				continue;
			}
			if ( newCode >= next_code )
			{
				decode_stack[ 0 ] = character;
				count = decode_string( 1, oldCode );
			}
			else
				count = decode_string( 0, newCode );
			character = decode_stack[ count - 1 ];
			if( outPtr )
			{
				while ( count > 0 )
					outPtr[outByteLen++] = decode_stack[ --count ];
			}
			else
			{
				outByteLen += count;
			}

			err_when( next_code >= TABLE_SIZE);
			DICT( next_code ).parent_code = oldCode;
			DICT( next_code ).character = character;
			next_code++;
			oldCode = newCode;
		}
	}

	//	free_storage();
	return outByteLen;
}
// --------- end of function Lzw::basic_expand -------------//


// --------- begin of function Lzw::find_child_node -------------//
//
// This hashing routine is responsible for finding the table location
// for a string/character combination.  The table index is created
// by using an exclusive OR combination of the prefix and character.
// This code also has to check for collisions, and handles them by
// jumping around in the table.
//
unsigned short Lzw::find_child_node( unsigned short parentCode, unsigned char childChar )
{
	unsigned short index;
	unsigned short offset;

	index = ( (unsigned short) childChar << ( BITS - 8 ) ) ^ parentCode;
	if ( index == 0 )
		offset = 1;
	else
		offset = TABLE_SIZE - index;
	for ( ; ; )
	{
		if ( DICT( index ).code_value == UNUSED )
			return( index );
		if ( DICT( index ).parent_code == parentCode &&
			 DICT( index ).character == childChar )
			return( index );
		if ( index >= offset )
			index -= offset;
		else
			index += TABLE_SIZE - offset;
	}
}
// --------- end of function Lzw::find_child_node -------------//

// --------- begin of function Lzw::decode_string -------------//
//
// This routine decodes a string from the dictionary, and stores it
// in the decode_stack data structure.  It returns a count to the
// calling program of how many characters were placed in the stack.
//
unsigned int Lzw::decode_string( unsigned int count, unsigned short code )
{
	unsigned short initCode = code;

	while ( code > 255 )
	{
		err_when(count >= TABLE_SIZE);
		decode_stack[ count++ ] = DICT( code ).character;
		code = DICT( code ).parent_code;
	}
	decode_stack[ count++ ] = (char) code;
	return( count );
}
// --------- end of function Lzw::decode_string -------------//


/*
// --------- begin of function Lzw::output_bits -------------//
//
// if outPtr is null to find the compressed length
// make sure outPtr buffer is long enough
//
void Lzw::output_bits( unsigned char *outPtr, long &outLen, unsigned short stringCode, unsigned int stringLen )
{
	// fill low bit first
	if( outPtr )
	{
		outPtr += outLen / 8;
		int s = outLen % 8;
		int r = 8 - s;

		// mask off unused bit
		*(unsigned long *)outPtr &= bitMask[s];
		// stringCode &= bitMask[stringCode];

		// shift stringCode and put them to outPtr
		*(unsigned long *)outPtr |= (unsigned long)stringCode << s;
	}
	
	outLen += stringLen;
}
// --------- end of function Lzw::output_bits -------------//

// --------- begin of function Lzw::input_bits -------------//
// inPtr is allocated at least 32 bits longer than inCnt
unsigned short Lzw::input_bits( unsigned char *inPtr, long &inCnt, unsigned int stringLen )
{
	inPtr += inCnt / 8;
	int s = inCnt % 8;
	
	// get longer bits
	unsigned long ul = *(unsigned long *)inPtr;
	ul >>= s;

	inCnt += stringLen;

	// mask off leading bits
	return (unsigned short)ul & BITS_MASK[stringLen];
}
// --------- end of function Lzw::input_bits -------------//
*/


