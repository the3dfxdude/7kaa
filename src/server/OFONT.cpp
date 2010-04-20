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

//Filename    : OFONT.CPP
//Description : Font Object

#include <string.h>

#include <ALL.h>
#include <IMGFUN.h>
#include <OVGA.h>
#include <OSTR.h>
#include <OSYS.h>
#include <ORES.h>
#include <OINFO.h>
#include <OGAME.h>
#include <OMOUSE.h>
#include <OTRANSL.h>
#include <ONATION.h>
#include <OGAME.h>
#include <OHELP.h>
#include <OFONT.h>

//--------------------------------------------------------//
// A font resource file is composed of three parts :
//
// one FontHeader;
// one FontInfo for each character;
// Bitmap data.
//--------------------------------------------------------//

//-------- Define struct FontHeader -------//

#pragma pack(1)
struct FontHeader
{
	unsigned	short	max_width;
	unsigned short max_height;
   unsigned short std_height;
	unsigned short first_char;		// ascii code of the first character
	unsigned	short last_char;		// ascii code of the last character
};
#pragma pack()

//-------- Define struct FontInfo -------//

#pragma pack(1)
struct FontInfo	// info for each character
{
	char				offset_y;
	unsigned char	width;
	unsigned char	height;
	long				bitmap_offset;	// file offset relative to bitmap data
};
#pragma pack()

//--------- Define macro constant ------------//

#define HYPER_FIELD_COLOR 	V_LIGHT_BLUE

//--------- German character table -------//

//#ifdef GERMAN
//#define GERMAN_CHAR_COUNT  	7

//	their DOS ascii characters:			 ü    ä    ß    ö    Ä    Ü    Ö
//static short german_ascii_array[]  = { 252, 228, 223, 246, 196, 220, 214 };
//static short german_bmp_id_array[] = { 121, 100,  95, 116, 128, 151, 145 };
//#endif

//------- Define static member variable --------//

short      Font::hyper_field_count;
HyperField Font::hyper_field_array[MAX_HYPER_FIELD];

static int text_line_count;     // used by text_width() & text_height() only

//----------- Begin of function Font Constructor -------//

Font::Font(char* fontName)
{
	init_flag = 0;
	next_text_ptr = NULL;
	next_text_y = 0;
	line_count = 0;
	font_height = 0;
	std_font_height = 0;
	max_font_height = 0;
	max_font_width = 0;
	space_width = 0;
	inter_char_space = 0;
	first_char = 0;
	last_char = 0;
	font_info_array = NULL;
	font_bitmap_buf = NULL;

	if( fontName )
		init(fontName);
}
//------------- End of function Font Constructor -------//


//----------- Begin of function Font Destructor -------//

Font::~Font()
{
	deinit();
}
//------------- End of function Font Destructor -------//


//-------- Begin of function Font::init -----------//
//
// <char*> fontName     = name of the font type.
//                        (e.g. "STD" will load "FNT_STD.RES" )
//
// [int] interCharSpace = inter-character space
//                        (default:1)
//
// [int] italicShift    = horzontal back shifting for italic font
//                        (default:0)
//
// Return : 1 - succeed;
//          0 - reading palette file error
//
void Font::init(const char* fontName, int interCharSpace, int italicShift)
{
	if( init_flag )
		deinit();

	inter_char_space = interCharSpace;

	//---------- open the font file -----------//

	File fontFile;

	String str;

	str  = DIR_RES;
	str += "FNT_";
	str += fontName;
	str += ".RES";

	fontFile.file_open(str);

	//-------- read in the font header ---------//

	FontHeader fontHeader;

	fontFile.file_read( &fontHeader, sizeof(FontHeader) );

	max_font_width  = fontHeader.max_width;
	max_font_height = fontHeader.max_height;
	std_font_height = fontHeader.std_height;

	font_height		 = std_font_height;		// its default is std_font_height, but can be set to max_font_height 

	first_char  	 = fontHeader.first_char;
	last_char		 = fontHeader.last_char;

	//----------- read in font info ------------//

	int infoArraySize = sizeof(FontInfo) * (last_char-first_char+1);

	font_info_array = (FontInfo*) mem_add( infoArraySize );

	fontFile.file_read( font_info_array, infoArraySize );

	//------- process italic shift --------//

	if( italicShift )
	{
		for( int i=0 ; i<last_char-first_char+1 ; i++ )
			font_info_array[i].width -= italicShift;
	}

	//---------- read in bitmap data ----------//

	int bitmapBufSize = fontFile.file_size() - sizeof(FontHeader) - infoArraySize;

	font_bitmap_buf = mem_add( bitmapBufSize);

	fontFile.file_read( font_bitmap_buf, bitmapBufSize );

	//---- get the width of the space character ----//
	//
	// since in some font, the space char is too narrow,
	// we use the width of 't' as space instead
	//
	//----------------------------------------------//

	space_width = font_info_array['t'-first_char].width;

	//----------------------------------------------//

	fontFile.file_close();

	init_flag = 1;
}
//---------- End of function Font::init ----------//


//----------- Start of function Font::deinit ---------//
//
void Font::deinit()
{
	if( font_info_array )
	{
		mem_del( font_info_array );
		font_info_array = NULL;
	}

	if( font_bitmap_buf )
	{
		mem_del( font_bitmap_buf );
		font_bitmap_buf = NULL;
	}

	init_flag = 0;
}
//------------- End of function Font::deinit ---------//


//--------- Start of function Font::put ---------//
//
// write text with pre-inited fonts
//
// int x,y         = location of the font
// char* text      = the text to be put on screen
// [int] clearBack = clear background with back_color or not
//                   (default : 0)
// [int]  x2       = display font up to the right border x2 and also
//                   clear the area between the last character and right border
//
// Return : <int> lastX, the x coordination of the last pixel of last font
//
int Font::put(int x,int y,const char* textPtr, char clearBack, int x2 )
{
	err_when( x<0 || y<0 );

	if( !init_flag )
		return x;

	//-------- process translation ---------//

	short textChar;

	textPtr = translate.process(textPtr);

	//-------------------------------------//

	int textPtrLen = strlen(textPtr);

	if( x2 < 0 ) // default
		x2 = x+max_font_width*textPtrLen;

	x2 = MIN( x2, VGA_WIDTH-1 );

	if( !Vga::use_back_buf )
		mouse.hide_area( x, y, x2, y+font_height );

	int y2 = y+font_height-1;

	//-------------------------------------//

	FontInfo* fontInfo;

	for( int lenCount=1 ; *textPtr && lenCount<=textPtrLen ; textPtr++, lenCount++ )
	{
		textChar = *((unsigned char*)textPtr);         // textChar is <unsiged char>

//#ifdef GERMAN
//		textChar = translate_german_char(textChar);
//#endif

		//--------------- space character ------------------//

		if( textChar == ' ' )
		{
			if( x+space_width > x2 )
				break;

			if( clearBack && !Vga::use_back_buf )	// copy texture from the back buffer as the background color
				vga.blt_buf( x, y, x+space_width-1, y+font_height-1, 0 );

			x += space_width;
		}

		//####### patch begin Gilbert 28/2 ########//
		// --------- control char: FIRST_NATION_COLOR_CODE_IN_TEXT -----------//

		else if( textChar >= FIRST_NATION_COLOR_CODE_IN_TEXT && textChar <= LAST_NATION_COLOR_CODE_IN_TEXT ) 	// display nation color bar in text
		{
			if( x2 >= 0 && x+NATION_COLOR_BAR_WIDTH-1 > x2 )      // exceed right border x2
				break;

			char colorCode = game.color_remap_array[textChar-FIRST_NATION_COLOR_CODE_IN_TEXT].main_color;

			nation_array.disp_nation_color(x, y+2, colorCode);

			x += NATION_COLOR_BAR_WIDTH;
		}

		//####### end begin Gilbert 28/2 ########//

		//------------- normal character ----------------//

		else if( textChar >= first_char && textChar <= last_char )
		{
			fontInfo = font_info_array+textChar-first_char;

			if( x+fontInfo->width > x2 )
				break;

			if( fontInfo->width > 0 )
			{
				if( clearBack && !Vga::use_back_buf )
				{
					if( fontInfo->offset_y > 0 )			// clear the upper space areas of the character
						IMGcopy( vga_front.buf_ptr(), vga_front.buf_pitch(), vga_back.buf_ptr(), vga_back.buf_pitch(),
							x, y, x+fontInfo->width-1, y+fontInfo->offset_y-1 );

					IMGjoinTrans( vga_front.buf_ptr(), vga_front.buf_pitch(), 
						vga_back.buf_ptr(), vga_back.buf_pitch(), x,	y+fontInfo->offset_y, 
						font_bitmap_buf + fontInfo->bitmap_offset );

					int ty = y+fontInfo->offset_y+fontInfo->height-1;

					if( ty < y2 )			// clear the lower space areas of the character
						IMGcopy( vga_front.buf_ptr(), vga_front.buf_pitch(),
							vga_back.buf_ptr(), vga_back.buf_pitch(), x, ty+1, x+fontInfo->width-1, y2 );
				}
				else
				{
					IMGbltTrans( Vga::active_buf->buf_ptr(), Vga::active_buf->buf_pitch(), 
						x, y+fontInfo->offset_y, font_bitmap_buf + fontInfo->bitmap_offset );
				}

				x += fontInfo->width;		// inter-character space
			}
		}
		else
		{
			//------ tab or unknown character -------//

			if( textChar == '\t' )         // Tab
				x += space_width*8;          // one tab = 8 space chars
			else
				x += space_width;
		}

		//--------- inter-character space ---------//

		if( clearBack && !Vga::use_back_buf )	// copy texture from the back buffer as the background color
			vga.blt_buf( x, y, x+inter_char_space-1, y+font_height-1, 0 );
	
		x+=inter_char_space;
	}

	//------ clear remaining area -------//

	if( clearBack && !Vga::use_back_buf )	// copy texture from the back buffer as the background color
		vga.blt_buf( x, y, x2, y+font_height-1, 0 );

	if( !Vga::use_back_buf )
		mouse.show_area();

	return x-1;
}
//----------- End of function Font::put ---------//


//#ifdef GERMAN
//--------- Start of function Font::translate_german_char ---------//
//
// <char> textChar  = the character to be translated
//
//short Font::translate_german_char(short textChar)
//{
//	for( int i=0 ; i<GERMAN_CHAR_COUNT ; i++ )
//	{
//		if( german_ascii_array[i] == textChar )
//			return first_char + german_bmp_id_array[i];
//	}
//	return textChar;
//}
//----------- End of function Font::translate_german_char ---------//
//#endif


//--------- Start of function Font::put_char ---------//
//
// <int>  x, y      = the position of the character
// <char> textChar  = the character
//
void Font::put_char(int x, int y, unsigned short textChar)
{
	if( textChar >= first_char && textChar <= last_char )
	{
		FontInfo* fontInfo = font_info_array+textChar-first_char;

		IMGbltTrans( Vga::active_buf->buf_ptr(), Vga::active_buf->buf_pitch(),
			x, y + fontInfo->offset_y, font_bitmap_buf + fontInfo->bitmap_offset );
	}
}
//----------- End of function Font::put_char ---------//


//--------- Start of function Font::right_put ---------//
//
// Display a string in right-justified format.
//
// <int>   x, y    = the right end position of displaying the string
// <char*> textPtr = the character
//
void Font::right_put(int x, int y, char* textPtr)
{
	int textWidth = text_width(textPtr);

	put( x-textWidth, y, textPtr );
}
//----------- End of function Font::right_put ---------//


//--------- Begin of function Font::text_width ----//
//
// Given the textPtr string, then calculate the width (unit:pixel).
//
// char* textPtr         = the textPtr to be put on screen
// [int] textPtrLen      = the length of the textPtr to be put
//                      (default : -1, until NULL character)
// [int] maxDispWidth = the maximum width can be displayed
//                      the textPtr width cannot > maxDispWidth
//
// Note : static var text_line_count is used to pass value to text_height()
//
// Return : <int> the screen width of the textPtr display using this font
//
int Font::text_width(const char* textPtr, int textPtrLen, int maxDispWidth)
{
	int   charWidth, x=0, lenCount, maxLen=0, wordWidth=0;
	short textChar;

	if( !init_flag )
		return x;

	textPtr = translate.process(textPtr);

	if( textPtrLen < 0 )
		textPtrLen = strlen(textPtr);

	text_line_count=1;

	//-------------------------------------//

	for( lenCount=1 ; *textPtr && lenCount<=textPtrLen ; textPtr++, lenCount++, x+=inter_char_space )
	{
		textChar = *((unsigned char*)textPtr);

//#ifdef GERMAN
//		textChar = translate_german_char(textChar);
//#endif

		//-- if the line exceed the given MAX width, advance to next line --//

		if( maxDispWidth && x > maxDispWidth )
		{
			maxLen = maxDispWidth;
			x      = wordWidth;    // the last word of the prev line wraps to next line
			text_line_count++;
		}

		//--- if the textPtr has more than 1 line, get the longest line ---//

		if( textChar == '\n' )
		{
			if( x>maxLen )
				maxLen=x;

			x=0;
			wordWidth=0;
			text_line_count++;
			continue;              // next character
		}

		//-------- control char: FIRST_NATION_COLOR_CODE_IN_TEXT -----------//

		else if( textChar >= FIRST_NATION_COLOR_CODE_IN_TEXT && textChar <= LAST_NATION_COLOR_CODE_IN_TEXT  ) 	// display nation color bar in text
		{
			x += NATION_COLOR_BAR_WIDTH;
			wordWidth = 0;
		}

		//--- add the width of the character to the total line width ---//

		else if( textChar == ' ' )
		{
			x += space_width;
			wordWidth = 0;
		}

		else if( textChar >= first_char && textChar <= last_char )
		{
			charWidth = font_info_array[textChar-first_char].width;

			x         += charWidth;
			wordWidth += charWidth;
		}
		else
		{
			x         += space_width;
			wordWidth += space_width;
		}

		if( maxDispWidth && wordWidth > maxDispWidth )
		{
			x         -= wordWidth - maxDispWidth;
			wordWidth  = maxDispWidth;
		}
	}

	//-------------------------------------------//

	if( maxDispWidth && x > maxDispWidth )
		text_line_count++;

	if( textPtr[-1] == '\n' )       // if last character is line feed, don't count double
		text_line_count--;

	return MAX(maxLen,x);
}
//----------- End of function Font::text_width ----//


//--------- Begin of function Font::text_height ----//
//
// text_width() must first be called, this function use the result
// from text_width() to calc the textPtr height.
//
// [int] lineSpace = space between lines
//                   ( default : DEFAULT_LINE_SPACE )
//
// Return : <int> the screen height of the textPtr display using this font
//
int Font::text_height(int lineSpace)
{
	return font_height * text_line_count +
			 lineSpace   * (text_line_count-1);
}
//----------- End of function Font::text_height ----//


//--------- Begin of function Font::put_paragraph ------//
//
// Put a paragraph of textPtr on screen
// Given the area of the paragraph
// Automatical word wrap is available
//
// If the background color of the font is set, the area of the paragraph
// will be filled with the background color first.
//
// int x1,y1,x2,y2    = area of the textPtr
//
// char* textPtr         = the textPtr to be put on screen
//
// [int lineSpace]    = no. of pixel space between lines
//                      ( default : 2 )
//
// [int] startLine    = start display at which line
//                      ( default : 1, first line)
//
// [float wordPause]  = time pause between words
//                      ( default : 0 )
//
// [char dispFlag]    = display the textPtr or not, if called by count_line(),
//                      this would be 0.
//                      ( default : 1 )
//
//-------------------------------------------------------//
//
// It will store resulting paramteres :
//
// <char*> next_text_ptr = point to the textPtr just after this paragraph
//                         which has been put to the screen.
//
// <int>   next_textPtr_y   = the endding y position of the paragraph and which
//                         is the y position which next paragraph can be put
//
// <int>   line_count    = the no. of lines has been displayed in current
//                         put_paragraph() session.
//
// <HyperField> hyper_field_array[] = an array of hyper-textPtred field
//
//-------------------------------------------------------//

void Font::put_paragraph(int x1, int y1, int x2, int y2, const char *textPtr,
								 int lineSpace, int startLine, char dispFlag)
{
	if( !init_flag || y1+font_height-1 > y2 )
		return;

	textPtr = translate.process(textPtr);

	//--------- define vars ---------------//

	int   x,y,wordX;
	int   newWord;
	short textChar;
	const char *wordPtr;

   char  flag_under_line=0;     // attribute control flags
   char  flag_hyper_field=0;

   int   under_line_x1;         // parameters for drawing under line

   HyperField* hyper_field_ptr = hyper_field_array;
	FontInfo*   fontInfo;

	//--------- initialize vars ---------------//

	x       = x1;
	y       = y1;
	wordX   = x1;          // the x of the start of the word
	wordPtr = textPtr;
	newWord = 0;

   line_count=0;

	hyper_field_count = 0;

   //---------- prepare for display font ----------//

	if( dispFlag )
   {
      err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

      if( !Vga::use_back_buf )     // if not say word by word, turn off mouse one time, otherwise turn off mouse one word at a time
			mouse.hide_area( x1,y1,x2,y2 );  // if the mouse cursor is in that area, hide it
	}

	//--------- loop for displaying textPtr ----------//

	while(1)
	{
		//------ space appear, process the previous word ------//

		if ( newWord )
		{
			if( x-1 > x2 )  // it is still okay if x-1==x2 because x is the next pixel, not the last displayed pixel
			{
				if( line_count >= startLine-1 )    // startLine start from 1
				{
					y += font_height + lineSpace;
					if ( y + font_height - 1 > y2 )     // no further space for printing textPtr, end
						break;
				}

				x = x1;
				line_count++;
			}
			else
			{
				x = wordX;
			}

			//--------- Process current word ----------//

			for( ; wordPtr < textPtr && *wordPtr ; wordPtr++, x+=inter_char_space )  // wordPtr point to the current word which will be printed
			{
				textChar = *((unsigned char*)wordPtr); // textChar is <unsiged char>

//#ifdef GERMAN
//				textChar = translate_german_char(textChar);
//#endif

				//---------- control char: '_' -------------//

				if( textChar == '_' )       // e.g. _Title_, a line will be drawn under the word "Title"
				{
					if( flag_under_line )    // current '_' is the closing one, the open '_' has appeared previously
					{
						if( dispFlag && line_count >= startLine-1 )    // startLine start from 1
							IMGbar(Vga::active_buf->buf_ptr(), Vga::active_buf->buf_pitch(),
								under_line_x1, y+font_height, x, y+font_height, HYPER_FIELD_COLOR );
					}
					else
						under_line_x1 = x;

					flag_under_line = !flag_under_line;
					continue;
				}

				//-------- control char: '~' -----------//

				else if( textChar == '~' )       // e.g. ~Captial~, the word "Capital" is a hyper-textPtred field, pressing on it will link to the topic "Capital"
				{
					if( !flag_hyper_field )   // current '~' is the opening one
					{
						hyper_field_ptr->x1       = x;
						hyper_field_ptr->y1       = y;
						hyper_field_ptr->text_ptr = (wordPtr+1);  // skip current '~'
					}
					else                      // current '~' is the closing one
					{
						hyper_field_ptr->x2       = x;
						hyper_field_ptr->y2       = y+font_height;
						hyper_field_ptr->text_len = wordPtr - hyper_field_ptr->text_ptr;

						hyper_field_count++;
						hyper_field_ptr++;
						err_when( hyper_field_count >= MAX_HYPER_FIELD );
					}

					flag_hyper_field = !flag_hyper_field;
					continue;
				}

				//-------- control char: FIRST_NATION_COLOR_CODE_IN_TEXT -----------//

				else if( textChar >= FIRST_NATION_COLOR_CODE_IN_TEXT && textChar <= LAST_NATION_COLOR_CODE_IN_TEXT ) 	// display nation color bar in text
				{
					if( x2 >= 0 && x+NATION_COLOR_BAR_WIDTH-1 > x2 )      // exceed right border x2
						break;

					char colorCode = game.color_remap_array[textChar-FIRST_NATION_COLOR_CODE_IN_TEXT].main_color;

					nation_array.disp_nation_color(x, y+2, colorCode);

					x += NATION_COLOR_BAR_WIDTH;
				}

				//--------------- space character ------------------//

				else if( textChar == ' ' )
				{
					if( x+space_width > x2 )
						break;

					x += space_width;
				}

				//----------- display char ------------//

				else if( textChar >= first_char && textChar <= last_char )
				{
					fontInfo = font_info_array+textChar-first_char;

					// character width = offset of next character - current offset

					if( x2 >= 0 && x+fontInfo->width-1 > x2 )      // exceed right border x2
						break;

					if( fontInfo->width > 0 )
					{
						if( dispFlag && line_count >= startLine-1 )    // startLine start from 1
						{
//							if( flag_hyper_field )
//								mainColor = HYPER_FIELD_COLOR;

							IMGbltTrans( Vga::active_buf->buf_ptr(), Vga::active_buf->buf_pitch(),
								x,	y+fontInfo->offset_y, font_bitmap_buf + fontInfo->bitmap_offset );
						}

						x += fontInfo->width;
					}
				}
			}

			//--------- next line ----------------------//

			if( *textPtr == '\n' )   // next line
			{
				if( line_count >= startLine-1 )    // startLine start from 1
				{
					y += font_height + lineSpace;
					if ( y + font_height - 1 > y2 )     // no further space for printing textPtr, end
						break;
				}

				x = x1;
				textPtr++;
				line_count++;
			}

			if( *textPtr == NULL )     // all paragraph has been printed
				break;

			wordPtr = textPtr;
			wordX   = x;
			newWord = 0;
		}

		//------------ process spacing -------------//

		if( *textPtr == ' ' || *textPtr == '\n' || *textPtr == NULL )    // not space
			newWord = 1;
		else
		{
			textChar = *((unsigned char*)textPtr); // textChar is <unsiged char>

//#ifdef GERMAN
//			textChar = translate_german_char(textChar);
//#endif

			if( textChar >= first_char && textChar <= last_char )
			{
				fontInfo = font_info_array+textChar-first_char;

				if( textChar >= FIRST_NATION_COLOR_CODE_IN_TEXT && textChar <= LAST_NATION_COLOR_CODE_IN_TEXT ) 	// display nation color bar in text
					x += NATION_COLOR_BAR_WIDTH;

				else if( textChar == ' ' )
					x+=space_width;

				else if( fontInfo->width > 0 )   // current font width
					x+=fontInfo->width;

				else
					x+=space_width;
			}
			else
			{
				x+=space_width;
			}

			x+=inter_char_space;
		}

		if( *textPtr && *textPtr != '\n' )    // when finished, remain as NULL for next cycle to detect
			textPtr++;
	}

	//------------ finish displaying ----------------//

	if( dispFlag )
	{
		if( !Vga::use_back_buf )
			mouse.show_area();
	}

//   while( *textPtr == '\n' || *textPtr == '\r' )
//      textPtr++;

	next_text_ptr = textPtr;    // the current pointer in the textPtr
	next_text_y   = y + font_height + lineSpace;
	line_count++;
}
//---------- End of function Font::put_paragraph ------//


//--------- Begin of function Font::count_line ------//
//
// Count the no. of lines of textPtr can be displayed on the given area.
//
// int x1,y1,x2,y2 = area of the textPtr
// char* textPtr      = the textPtr to be put on screen
//
// <int lineSpace> = no. of pixel space between lines
//                   ( default : 0 )
//
// <int& dispLines>  = no. of lines can be displayed in the textPtr area
// <int& totalLines> = total no. of lines of the textPtr
//
void Font::count_line(int x1, int y1, int x2, int y2, const char *textPtr,
							 int lineSpace , int& dispLines, int& totalLines)
{
	dispLines =0;
	totalLines=0;

	while( *textPtr )    // if *textPtr==NULL means EOF
	{
		put_paragraph(x1, y1, x2, y2, textPtr, lineSpace, 1, 0);   // last 0-means don't display the textPtr, only returning the parameters for our calculating of line counts

		if( !dispLines )                  // only set once at the first time
			dispLines = line_count;

		totalLines += line_count;
		textPtr     = next_text_ptr;
	}
}
//---------- End of function Font::count_line ------//


//-------- Begin of function Font::d3_put --------//
//
// Display field description and field value
//
// <int>   x1,y1      = the coordination of the spinner description
// <char*> desStr     = the spinner description
//
// Return : <int> lastX, the x coordination of the last pixel of last font
//
int Font::d3_put(int x1, int y1, const char* desStr )
{
	int marginSpace = font_height/5;

	int x2 = x1+text_width(desStr)+marginSpace*2-1;

	vga.d3_panel_up( x1, y1, x2, y1+font_height+marginSpace*2-1 );

   put( x1+marginSpace, y1+marginSpace, desStr);

   return x2;
}
//--------- End of function Font::d3_put ---------//


//-------- Begin of function Font::d3_put --------//
//
// Display field description and field value.
// It will display the description on the center of the panel
//
// <int>   x1,y1      = the coordination of the panel
// <int>   x2,y2      =
// <char*> desStr     = the spinner description
//
void Font::d3_put(int x1, int y1, int x2, int y2, const char* desStr)
{
	int tx = x1 + ((x2-x1+1) - text_width(desStr))/2;
   int ty = y1 + ((y2-y1+1) - font_height)/2+1;

	vga.d3_panel_up( x1, y1, x2, y2);

   if( tx<x1+2 )
      tx=x1+2;

   put( tx, ty, desStr, 0, x2-2 );
}
//--------- End of function Font::d3_put ---------//


//-------- Begin of function Font::center_put --------//
//
// Display the textPtr on the center of the given area
//
// <int>   x1,y1   = the coordination of the panel
// <int>   x2,y2   =
// <char*> desStr  = the spinner description
// [char] clearBack = clear background with back_color or not
//                   (default : 0)
//
// Return : <int> lastX, the x coordination of the last pixel of last font
//
int Font::center_put(int x1, int y1, int x2, int y2, const char* desStr, char clearBack)
{
	int tx = x1 + ((x2-x1+1) - text_width(desStr))/2;
	int ty = y1 + ((y2-y1+1) - font_height)/2;

	if( tx<0 )
		tx=0;

	if( clearBack && !Vga::use_back_buf && tx>x1 )	// copy texture from the back buffer as the background color
		vga.blt_buf( x1, y1, tx-1, y2, 0 );

	return put( tx, ty, desStr, clearBack, x2 );
}
//--------- End of function Font::center_put ---------//


//-------- Begin of function Font::put_field --------//
//
// Display field description and field value
//
// format - "Field : 999999" e.g. "List price : $92,232"
//
// <int>   x1,y1  = the coordination of the spinner description
// <char*> desStr = the spinner description
// <int>   x2     = the coordination of the spinner variable
// <int>   value  = value
//
// [int]   format = 1 - 1,000,000 normal, with thousand separator
//                  2 - $1,000,000 add dollar sign, with thousand separator
//                  3 - 100% percentage
//                  (default : 1 )
//
void Font::put_field(int x1, int y1, const char* desStr, int x2, int value, int format )
{
	vga.d3_panel_up( x1, y1, x2, y1+font_height+3 );

	put( x1+2, y1+2, desStr);
	put( x2+4, y1+2, m.format(value,format) );
}
//--------- End of function Font::put_field ---------//


//-------- Begin of function Font::update_field --------//
//
// Update the value of the field which is previously displayed by
// Font::put_field()
//
// <int>   x1,y1  = the coordination of the field variable
// <int>   value  = value
//
// <int>   format = 1 - 1,000,000 normal, with thousand separator
//                  2 - $1,000,000 add dollar sign, with thousand separator
//                  3 - 100% percentage
//                  (default : 1 )
//
// <int>   x2     = the x2 (right border) of the field
//
void Font::update_field(int x1, int y1, int value, int format, int x2)
{
	if( x2<0 )
      x2 = x1+80;

   put( x1+4, y1+2, m.format(value,format), 1, x2 );
}
//--------- End of function Font::update_field ---------//


//-------- Begin of function Font::field --------//
//
// Combine the function of put_field() and update_field()
// depending on the refreshFlag passed
//
// format - "Field : 999999" e.g. "List price : $92,232"
//
// <int>   x1,y1  = the coordination of the spinner description
// <char*> desStr = the spinner description
// <int>   x2     = the coordination of the spinner variable
// <int>   value  = value
//
// <int>   format = 1 - 1,000,000 normal, with thousand separator
//                  2 - $1,000,000 add dollar sign, with thousand separator
//                  3 - 100% percentage
//
// <int> x2          = the x2 (right border) of the field
// <int> refreshFlag = refresh flag, either INFO_REPAINT or INFO_UPDATE
// [char*] helpCode  = help code
//
void Font::field(int xDes, int y1, const char* desStr, int xValue, int value,
					  int format, int xEnd, int refreshFlag, const char* helpCode)
{
	int x2;

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_up( xDes, y1, xValue, y1+font_height+3 );

		put( xDes+2  , y1+2, desStr);
		x2 = put( xValue+4, y1+2, m.format(value,format) );
	}
	else
	{
		x2 = put( xValue+4, y1+2, m.format(value,format), 1, xEnd );
	}

	if( helpCode )
		help.set_help(xDes, y1, x2, y1+font_height+3, helpCode );
}
//--------- End of function Font::field ---------//


//-------- Begin of function Font::put_field --------//
//
// Display field description and field value
//
// format - "Field : 999999" e.g. "List price : $92,232"
//
// <int>    x1,y1  = the coordination of the spinner description
// <char*>  desStr = the spinner description
// <int>    x2     = the coordination of the spinner variable
// <double> value  = value
//
// [int]    format = 1 - 1,000,000 normal, with thousand separator
//                   2 - $1,000,000 add dollar sign, with thousand separator
//                   3 - 100% percentage
//                   (default : 1 )
//
void Font::put_field(int x1, int y1, const char* desStr, int x2, double value, int format)
{
	vga.d3_panel_up( x1, y1, x2, y1+font_height+3 );

	put( x1+2, y1+2, desStr);
	put( x2+4, y1+2, m.format(value,format) );
}
//--------- End of function Font::put_field ---------//


//-------- Begin of function Font::update_field --------//
//
// Update the value of the field which is previously displayed by
// Font::put_field()
//
// <int>   x1,y1  = the coordination of the field variable
// <double>  value  = value
//
// <int>   format = 1 - 1,000,000 normal, with thousand separator
//                  2 - $1,000,000 add dollar sign, with thousand separator
//                  3 - 100% percentage
//                  (default : 1 )
//
// <int>   x2     = the x2 (right border) of the field
//
void Font::update_field(int x1, int y1, double value, int format, int x2)
{
	if( x2<0 )
      x2 = x1+80;

   put( x1+4, y1+2, m.format(value,format), 1, x2 );
}
//--------- End of function Font::update_field ---------//


//-------- Begin of function Font::field --------//
//
// Combine the function of put_field() and update_field()
// depending on the refreshFlag passed
//
// format - "Field : 999999" e.g. "List price : $92,232"
//
// <int>   x1,y1  = the coordination of the spinner description
// <char*> desStr = the spinner description
// <int>   x2     = the coordination of the spinner variable
// <double>  value  = value
//
// <int>   format = 1 - 1,000,000 normal, with thousand separator
//                  2 - $1,000,000 add dollar sign, with thousand separator
//                  3 - 100% percentage
//
// <int> x2          = the x2 (right border) of the field
// <int> refreshFlag = refresh flag, either INFO_REPAINT or INFO_UPDATE
// [char*] helpCode  = help code
//
void Font::field(int xDes, int y1, const char* desStr, int xValue, double value,
					  int format, int xEnd, int refreshFlag, char* helpCode )
{
	int x2;

   if( refreshFlag == INFO_REPAINT )
   {
		vga.d3_panel_up( xDes, y1, xValue, y1+font_height+3 );

      put( xDes+2  , y1+2, desStr);
      x2 = put( xValue+4, y1+2, m.format(value,format) );
   }
   else
   {
		x2 = put( xValue+4, y1+2, m.format(value,format), 1, xEnd );
	}

	if( helpCode )
		help.set_help(xDes, y1, x2, y1+font_height+3, helpCode );
}
//--------- End of function Font::field ---------//


//-------- Begin of function Font::put_field --------//
//
// Display field description and field value
//
// format - "Field : 999999" e.g. "List price : $92,232"
//
// <int>   x1,y1  = the coordination of the spinner description
// <char*> desStr = the spinner description
// <int>   x2     = the coordination of the spinner variable
// <char*> value  = value
//
void Font::put_field(int x1, int y1, const char* desStr, int x2, char* value)
{
	vga.d3_panel_up( x1, y1, x2, y1+font_height+3 );

   put( x1+2, y1+2, desStr);
   put( x2+4, y1+2, value);
}
//--------- End of function Font::put_field ---------//


//-------- Begin of function Font::update_field --------//
//
// Update the value of the field which is previously displayed by
// Font::put_field()
//
// <int>   x1,y1  = the coordination of the field variable
// <char*> value  = value
// <int>   x2     = the x2 (right border) of the field
//
void Font::update_field(int x1, int y1, const char* value, int x2)
{
	if( x2<0 )
      x2 = x1+80;

   put( x1+4, y1+2, value, 1, x2 );
}
//--------- End of function Font::update_field ---------//


//-------- Begin of function Font::field --------//
//
// Combine the function of put_field() and update_field()
// depending on the refreshFlag passed
//
// format - "Field : 999999" e.g. "List price : $92,232"
//
// <int>   x1,y1  = the coordination of the spinner description
// <char*> desStr = the spinner description
// <int>   x2     = the coordination of the spinner variable
// <char*> value  = value
//
// <int> x3          = the x3 (right border) of the field
// <int> refreshFlag = refresh flag, either INFO_REPAINT or INFO_UPDATE
// [char*] helpCode  = help code
//
void Font::field(int xDes, int y1, const char* desStr, int xValue, const char* value,
					  int xEnd, int refreshFlag, const char* helpCode )
{
	int x2;

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_up( xDes, y1, xValue, y1+font_height+3 );

		put( xDes+2  , y1+2, desStr);
		x2 = put( xValue+4, y1+2, value );
	}
	else
	{
		x2 = put( xValue+4, y1+2, value, 1, xEnd );
	}

	if( helpCode )
		help.set_help(xDes, y1, x2, y1+font_height+3, helpCode );
}
//--------- End of function Font::field ---------//


//-------- Begin of function Font::disp --------//
//
// Display a value and clear the background
//
// <int>   x1,y1  = the coordination of the field variable
// <int>   value  = value
//
// <int>   format = 1 - 1,000,000 normal, with thousand separator
//                  2 - $1,000,000 add dollar sign, with thousand separator
//                  3 - 100% percentage
//                  (default : 1 )
//
// <int>   x2     = the x2 (right border) of the field
//
// Return : <int> lastX, the x coordination of the last pixel of last font
//
int Font::disp(int x1, int y1, int value, int format, int x2)
{
	if( x2<0 )
      x2 = x1+80;

   int lastX = put( x1, y1, m.format(value,format), 1, x2 );

   return lastX;
}
//--------- End of function Font::disp ---------//


//-------- Begin of function Font::disp --------//
//
// Display a value and clear the background
//
// <int>    x1,y1  = the coordination of the field variable
// <double> value  = value
//
// <int>    format = 1 - 1,000,000 normal, with thousand separator
//                   2 - $1,000,000 add dollar sign, with thousand separator
//                   3 - 100% percentage
//                   (default : 1 )
//
// <int>   x2      = the x2 (right border) of the field
//
// Return : <int> lastX, the x coordination of the last pixel of last font
//
int Font::disp(int x1, int y1, double value, int format, int x2)
{
	if( x2<0 )
      x2 = x1+80;

   int lastX = put( x1, y1, m.format(value,format), 1, x2 );

   return lastX;
}
//--------- End of function Font::disp ---------//


//-------- Begin of function Font::disp --------//
//
// Display a value and clear the background
//
// <int>   x1,y1     = the coordination of the field variable
// <char*> textPtr   = pointer to the textPtr
// <int>   x2        = the x2 (right border) of the field
//
// Return : <int> lastX, the x coordination of the last pixel of last font
//
int Font::disp(int x1, int y1, const char* textPtr, int x2)
{
	if( x2<0 )
      x2 = x1+80;

	int lastX = put( x1, y1, textPtr, 1, x2 );

   return lastX;
}
//--------- End of function Font::disp ---------//


//--------- Begin of function Font::put_char_to_buffer ---------//
void Font::put_char_to_buffer(char* dest, int destPitch, int x, int y, unsigned short textChar)
{
	if( textChar >= first_char && textChar <= last_char )
	{
		FontInfo* fontInfo = font_info_array+textChar-first_char;
		char *fontBitmap = font_bitmap_buf + fontInfo->bitmap_offset;

		IMGbltTrans( dest, destPitch, x, y + fontInfo->offset_y, fontBitmap );
	}
}
//--------- End of function Font::put_char_to_buffer ---------//


//--------- Begin of function Font::put_to_buffer ---------//
void Font::put_to_buffer(char* dest, int destPitch, int x1, int y1, const char *text)
{
	int x2 = destPitch;			// width of buffer
	while( *text != '\0' && x1 < x2)
	{
		int charSize = sizeof(unsigned char);		// 1 for byte character, 2 for word character
		unsigned short textChar = *(unsigned char *)text;

//#ifdef GERMAN
//		textChar = translate_german_char(textChar);
//#endif
		
		//if( // unicode or a word character )
		//{
		//	textChar = *(unsigned short *)buffer;			// unicode
		// textChar = *(unsigned char *)text << 8 + *(1+(unsigned char *)text);	// big-5...
		//	charSize = sizeof(unsigned short);
		//}

		// --------- advance to next character------------//

		int charWidth = textChar == ' ' ? space_width :
			(font_info_array+textChar-first_char)->width;

		if( x1 + charWidth <= x2 )
			put_char_to_buffer( dest, destPitch, x1, y1, textChar);

		x1 += charWidth;
		text += charSize;
	}
}
//--------- End of function Font::put_to_buffer ---------//


//--------- Begin of function Font::center_put_to_buffer ---------//
void Font::center_put_to_buffer(char* dest, int destPitch, int x1, int y1, int x2, int y2, char *desStr)
{
	int tx = x1 + ((x2-x1) - text_width(desStr))/2;
   int ty = y1 + ((y2-y1) - font_height)/2;

   if( tx<0 )
		tx=0;

   put_to_buffer( dest, destPitch, tx, ty, desStr);
}
//--------- End of function Font::center_put_to_buffer ---------//
