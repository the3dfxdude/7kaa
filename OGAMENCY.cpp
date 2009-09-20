// Filename    : OGAMENCY.CPP
// description : view encyclopedia

#include <ALL.h>
#include <OIMGRES.h>
#include <OVGA.h>
#include <OSYS.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OVGALOCK.h>
#include <OGAME.h>
#include <OMUSIC.h>

//--------- define constant ---------//

#define ENCYC_CLASS_COUNT	7
#define FRYHTAN_MAX_PAGE 3
#ifdef AMPLUS
	#define SUB_CLASS_BUTTON_MAX 10
#else
	#define SUB_CLASS_BUTTON_MAX 8
#endif

enum
{
	ENCYC_PEOPLE=1,
	ENCYC_WEAPONS,
	ENCYC_SHIPS,
	ENCYC_STRUCTURES,
	ENCYC_PALACE,
	ENCYC_GOD,
	ENCYC_FRYHTANS,
};

const int START_SLIDE_MODE_TIME = 60000;		// 60 seconds
const int SLIDE_MODE_TIME = 10000;			// 10 seconds

// static int sub_class_count_array[ENCYC_CLASS_COUNT] = { 7, 5, 4, 8, 7, 7, 1 };
static int sub_class_count_array[ENCYC_CLASS_COUNT] = 
#ifdef AMPLUS
	{ 10, 6, 4, 8, 10, 10, 17 };
#else
	{ 7, 5, 4, 8, 7, 7, 17 };
#endif

static char *race_name[] =
#ifdef AMPLUS
	{ "CHINESE", "EGYPTIAN", "GREEK", "JAPANESE", "MAYA", "INDIAN", "NORMAN", "PERSIAN", "VIKING", "ZULU" };
#else
	{ "CHINESE", "GREEK", "JAPANESE", "MAYA", "NORMAN", "PERSIAN", "VIKING" };
#endif

static char *weapon_name[] =
#ifdef AMPLUS
	{ "CATAPULT", "BALLISTA", "CANNON", "EXPCART", "FLAMETHR", "F_BALLIS" };
#else
	{ "CATAPULT", "BALLISTA", "CANNON", "EXPCART", "FLAMETHR" };
#endif

static char *ship_name[] =
{
	"VESSEL",
	"TRANSPOR",
	"CARAVEL",
	"GALLEON",
};

static char *firm_name[] =
{
	"FORT",
	"FACTORY",
	"WARFACT",
	"MARKET",
	"MINE",
	"INN",
	"SCIENCE",
	"HARBOR",
};

static char *god_name[] =
#ifdef AMPLUS
	{ "CHINESE", "GREEK", "PERSIAN", "NORMAN", "JAPANESE", "MAYA", "VIKING", "EGYPTIAN", "ZULU", "INDIAN" };
#else
	{ "CHINESE", "GREEK", "JAPANESE", "MAYA", "NORMAN", "PERSIAN", "VIKING" };
#endif

static char *monster_name[] =
{
	"HOBGLOB",
	"SKELETON",
	"GREMJERM",
	"GNOLL",
	"GIANTET",
	"HEADLESS",
	"GOBLIN",

	"MAN",
	"GITH",
	"LYW",
	"ROCKMAN",
	"LIZARD",
	"FIREKIN",

	"STRUCT_1",
	"STRUCT_2",
	"STRUCT_3",
	"STRUCT_4",
};

static char monster_page_index[FRYHTAN_MAX_PAGE+1] = { 0, 7, 13, 17 };

static char* button_name_array[ENCYC_CLASS_COUNT] =
{
	"B_PEOP", "B_WEAP", "B_SHIP", "B_STRUCT", "B_PEOP",
	"B_GBEING", "B_FRYH",
};

static char* monster_button_name_array[FRYHTAN_MAX_PAGE] = 
{
	"B_FRYH", "B_FRYH2", "B_FRYH3",
};

static char std_extension[] = ".ICN";
static char pal_extension[] = ".COL";

//-------- define static variables ----------//

static int main_class_id;
static int sub_class_id_array[ENCYC_CLASS_COUNT];
static int fryhtan_page;

//-------- declare static functions ----------//

static int detect_main_class_button();
static int detect_sub_class_button(int n, int firstButton=1);
static void disp_class_buttons();
static int  disp_picture( int selClass, int selSubClass, int firstDisp=0);

//-------- Begin of function Game::view_encyclopedia ---------//

void Game::view_encyclopedia()
{
	music.stop();			// no music for encyclopedia as it reads files from the CDROM

	//-------- backup and switch palette ----------//

	VgaCustomPalette *backupPal = new VgaCustomPalette(NULL);

	//---- load the interface into the back buffer ----//

	image_encyc.put_to_buf( &vga_back, "ENCYC" );

	//-------- hide and change mouse cursor --------//

	mouse.hide();
	mouse_cursor.set_icon(CURSOR_ENCYC);

	main_class_id = 1;

	for( int i=0 ; i<ENCYC_CLASS_COUNT ; i++ )
		sub_class_id_array[i] = 1;

	unsigned long nextDisplayTime = m.get_time() + START_SLIDE_MODE_TIME;

	//------- display the default picture ----//

	disp_picture(main_class_id, sub_class_id_array[main_class_id-1], 1 );

	//------ turn screen dark and blt the buffer ---------//

	vga_front.bar( 0, 0, VGA_WIDTH-1, VGA_HEIGHT-1, 0 );
	sys.blt_virtual_buf();

	//------- bilt the back buffer to the front ---------//

	vga.blt_buf( 0,0, vga_back.buf_width()-1, vga_back.buf_height()-1, 0 );
	mouse.show();

	disp_class_buttons();

	//---------------------------------------------//

	while(1)
	{
		//-------------- yield ---------------//

		sys.yield();
		mouse.get_event();
		sys.blt_virtual_buf();

		//------ detect main class buttons -------//

		int selClass = detect_main_class_button();

		if( selClass && selClass != main_class_id)
		{
			main_class_id = selClass;
			disp_class_buttons();
			disp_picture(main_class_id, sub_class_id_array[main_class_id-1]);
		}

		//------ detect sub class buttons -------//

		int selSubClass = 0;
		if( main_class_id != ENCYC_FRYHTANS )
		{
			selSubClass = detect_sub_class_button(sub_class_count_array[main_class_id-1]);
		}
		else
		{
#ifdef AMPLUS
			// two buttons at the bottom are for switching page
#else
			// (one)two buttons next to the buttons of that page (pictButtonCount) are for switching page
#endif
			// if in monster class, find out which page 
			int monsterSubClass;
			for( monsterSubClass = 1; monsterSubClass <= FRYHTAN_MAX_PAGE &&
				sub_class_id_array[main_class_id-1]-1 >= monster_page_index[monsterSubClass];
				++monsterSubClass);
			err_when( monsterSubClass > FRYHTAN_MAX_PAGE );

			int pictButtonCount = monster_page_index[monsterSubClass] - 
				monster_page_index[monsterSubClass-1];
#ifdef AMPLUS
			int nextPageButton = SUB_CLASS_BUTTON_MAX-1;
			int prevPageButton = SUB_CLASS_BUTTON_MAX;
#else
			int nextPageButton = pictButtonCount+1;
			int prevPageButton = pictButtonCount+2;
#endif

			// translate selSubClass from i-th button to new value of sub_class_count_array[main_class_id-1]
			if( (selSubClass = detect_sub_class_button(pictButtonCount)) > 0)
			{
				// click sub-class buttons
				selSubClass = monster_page_index[monsterSubClass-1]+(selSubClass-1)+1;
			}
			else if( nextPageButton <= SUB_CLASS_BUTTON_MAX &&
				detect_sub_class_button(nextPageButton, nextPageButton) == nextPageButton )
			{
				// click page switching buttons
				// increase monsterSubClass
				if( ++monsterSubClass > FRYHTAN_MAX_PAGE )
					monsterSubClass = 1;

				// switch page, assume the first picture of that page 
				selSubClass = monster_page_index[monsterSubClass-1]+1;
			}
			else if( prevPageButton <= SUB_CLASS_BUTTON_MAX &&
				detect_sub_class_button(prevPageButton, prevPageButton) == prevPageButton )
			{
				// decrease monsterSubClass
				if( --monsterSubClass < 1)
					monsterSubClass = FRYHTAN_MAX_PAGE;

				// switch page, assume the first picture of that page 
				selSubClass = monster_page_index[monsterSubClass-1]+1;
			}
			else
				selSubClass = 0;		// as if not selected anything
		}

		if( selSubClass && selSubClass != sub_class_id_array[main_class_id-1] )
		{
			sub_class_id_array[main_class_id-1]	= selSubClass;

			disp_class_buttons();
			disp_picture(main_class_id, selSubClass);

			nextDisplayTime = m.get_time() + START_SLIDE_MODE_TIME;
		}

		//------ detect the "Return" button -------//

		if( mouse.single_click(6, 552, 165, 592) )		// return button
			break;

		//--------- F9 to capture screen ----------//

		if( mouse.single_click(174, 12, 787, 587, 1) )		// right clicking on the picture to save it
			sys.capture_screen();

		//----------- auto slide show -------------//

		if( m.get_time() > nextDisplayTime )
		{
			//--- display next pictures ---//

			if( ++sub_class_id_array[main_class_id-1] > sub_class_count_array[main_class_id-1] )
			{
				sub_class_id_array[main_class_id-1] = 1;

				if( ++main_class_id > ENCYC_CLASS_COUNT )
					main_class_id = 1;
			}

			disp_class_buttons();

			if( disp_picture(main_class_id, sub_class_id_array[main_class_id-1]) )
				nextDisplayTime = m.get_time() + SLIDE_MODE_TIME;
		}
	}

	//------- exiting: turn dark --------//

//	vga.adjust_brightness(-255);
	vga_front.bar(0,0, vga_front.buf_width()-1, vga_front.buf_height()-1, 0 );
	sys.blt_virtual_buf();

	//-------- restore mouse cursor --------//

	mouse_cursor.set_icon(CURSOR_NORMAL);

	//----- palette restore when backupPal destruct ----//
	{
		vga_front.bar( 0, 0, VGA_WIDTH-1, VGA_HEIGHT-1, 0 );

		VgaFrontLock vgaLock;
		delete backupPal;
	}
}
//--------- End of function Game::view_encyclopedia ---------//


//-------- Begin of static function detect_main_class_button ---------//

static int detect_main_class_button()
{
#ifdef AMPLUS
	enum { BUTTON_X = 14, BUTTON_Y = 14, BUTTON_WIDTH = 146, BUTTON_HEIGHT = 28 };
	enum { BUTTON_Y_SPACING = 30 };
#else
	enum { BUTTON_X = 14, BUTTON_Y = 14, BUTTON_WIDTH = 146, BUTTON_HEIGHT = 31 };
	enum { BUTTON_Y_SPACING = 34 };
#endif

	for( int c=1 ; c<=ENCYC_CLASS_COUNT ; c++ )
	{
		//if( mouse.press_area( BUTTON_X, BUTTON_Y + (c-1)*BUTTON_Y_SPACING, BUTTON_X + BUTTON_WIDTH-1,
		//	BUTTON_Y + c*BUTTON_Y_SPACING-1) )
		if( mouse.single_click( BUTTON_X, BUTTON_Y + (c-1)*BUTTON_Y_SPACING, BUTTON_X + BUTTON_WIDTH-1,
			BUTTON_Y + c*BUTTON_Y_SPACING-1) )
		{
			vga.blt_buf( BUTTON_X, BUTTON_Y + (main_class_id-1)*BUTTON_Y_SPACING, BUTTON_X + BUTTON_WIDTH-1,
				BUTTON_Y + main_class_id*BUTTON_Y_SPACING-1, 0);

			// ###### begin Gilbert 22/9 #######//
			// image_encyc.put_front(BUTTON_X-2, BUTTON_Y + (c-1)*BUTTON_Y_SPACING-2, "B_DOWN");
			image_encyc.put_front(BUTTON_X, BUTTON_Y + (c-1)*BUTTON_Y_SPACING, "B_DOWN");
			// ###### end Gilbert 22/9 #######//
			return c;
		}
	}

	return 0;
}
//-------- End of static function detect_main_class_button ---------//


//-------- Begin of static function detect_sub_class_button ---------//

static int detect_sub_class_button(int n, int firstButton)
{
#ifdef AMPLUS
	enum { BUTTON_X = 14, BUTTON_Y = 236, BUTTON_WIDTH = 146, BUTTON_HEIGHT = 28 };
	enum { BUTTON_Y_SPACING = 31, MAX_BUTTON = 10 };
#else
	enum { BUTTON_X = 14, BUTTON_Y = 266, BUTTON_WIDTH = 146, BUTTON_HEIGHT = 31 };
	enum { BUTTON_Y_SPACING = 35, MAX_BUTTON = 8 };
#endif

	for( int c=firstButton ; c<=n && c<=SUB_CLASS_BUTTON_MAX ; c++ )
	{
		//if( mouse.press_area( BUTTON_X, BUTTON_Y + (c-1)*BUTTON_Y_SPACING, BUTTON_X + BUTTON_WIDTH-1,
		//	BUTTON_Y + c*BUTTON_Y_SPACING-1) )
		if( mouse.single_click( BUTTON_X, BUTTON_Y + (c-1)*BUTTON_Y_SPACING, BUTTON_X + BUTTON_WIDTH-1,
			BUTTON_Y + c*BUTTON_Y_SPACING-1) )
		{
			// int subClassId = sub_class_id_array[main_class_id-1];

			// vga.blt_buf( BUTTON_X, BUTTON_Y + (subClassId-1)*BUTTON_Y_SPACING, BUTTON_X + BUTTON_WIDTH-1,
			// BUTTON_Y + subClassId*BUTTON_Y_SPACING-1, 0);

			// ###### begin Gilbert 22/9 #######//
			// image_encyc.put_front(BUTTON_X-2, BUTTON_Y + (c-1)*BUTTON_Y_SPACING-2, "B_DOWN");
			image_encyc.put_front(BUTTON_X, BUTTON_Y + (c-1)*BUTTON_Y_SPACING, "B_DOWN");
			// ###### end Gilbert 22/9 #######//
			return c;
		}
	}

	return 0;
}
//-------- End of static function detect_sub_class_button ---------//


//-------- Begin of static function disp_class_buttons ---------//
//
static void disp_class_buttons()
{
	// ###### begin Gilbert 22/9 #######//
#ifdef AMPLUS
	enum { BUTTON_X=14, BUTTON_WIDTH=146, CLASS_BUTTON_Y=14, CLASS_BUTTON_Y_SPACING=30, 
		SUBCLASS_BUTTON_Y=236, SUBCLASS_TOP_BORDER=3, BUTTON_Y_SPACING=31 };
#else
	enum { BUTTON_X=14, BUTTON_WIDTH=146, CLASS_BUTTON_Y=14, CLASS_BUTTON_Y_SPACING=34, 
		SUBCLASS_BUTTON_Y=266, SUBCLASS_TOP_BORDER=4, BUTTON_Y_SPACING=35 };
#endif
	// ###### end Gilbert 22/9 #######//

	int subClassId = sub_class_id_array[main_class_id-1];

	if( main_class_id != ENCYC_FRYHTANS )
	{
		vga_back.put_bitmap(12, SUBCLASS_BUTTON_Y-SUBCLASS_TOP_BORDER,
			image_encyc.get_ptr(button_name_array[main_class_id-1]) );
	}
	else
	{
		// if in monster class, find out which page 
		int monsterSubClass;
		for( monsterSubClass = 1; monsterSubClass <= FRYHTAN_MAX_PAGE &&
			sub_class_id_array[main_class_id-1]-1 >= monster_page_index[monsterSubClass];
			++monsterSubClass);
		err_when( monsterSubClass > FRYHTAN_MAX_PAGE );
		vga_back.put_bitmap(12, SUBCLASS_BUTTON_Y-SUBCLASS_TOP_BORDER, 
			image_encyc.get_ptr(monster_button_name_array[monsterSubClass-1]) );

		// modify subClassId
		subClassId -= monster_page_index[monsterSubClass-1];
	}

	vga.blt_buf( BUTTON_X, 14, BUTTON_X+BUTTON_WIDTH-1, VGA_HEIGHT-40, 0 );

	int y=CLASS_BUTTON_Y;
	// ###### begin Gilbert 22/9 #######//
	image_encyc.put_front( BUTTON_X, y + (main_class_id-1)*CLASS_BUTTON_Y_SPACING, "B_DOWN");
	// ###### end Gilbert 22/9 #######//

	y=SUBCLASS_BUTTON_Y;
	// #### begin Gilbert 22/9 #######//
	image_encyc.put_front( BUTTON_X, y + (subClassId-1)*BUTTON_Y_SPACING, "B_DOWN");
	// #### end Gilbert 22/9 #######//
}
//-------- End of static function disp_class_buttons ---------//


//-------- Begin of static function disp_picture ---------//
//
// <int> selClass, selSubClass - currently selected class and subclass id.
// [int] firstDisp - whether it is the first display action
//						   when the encyclopedia is called upl.
//
static int disp_picture( int selClass, int selSubClass, int firstDisp)
{
	if( !DIR_ENCYC[0] )
		return 0;

	char filename[64];
	char palname[64];
	filename[0] = '\0';

#ifdef AMPLUS
	// alternative path
	char filename2[64];
	char palname2[64];
	filename2[0] = '\0';
#endif

	switch( selClass )
	{
		case ENCYC_PEOPLE:
			strcpy( filename, DIR_ENCYC );
			strcat( filename, "UNIT\\" );
			strcat( filename, race_name[selSubClass-1] );
			strcpy( palname, filename);
			strcat( palname, pal_extension);
			strcat( filename, std_extension );
#ifdef AMPLUS
			if( DIR_ENCYC2[0] )
			{
				strcpy( filename2, DIR_ENCYC2 );
				strcat( filename2, "UNIT\\" );
				strcat( filename2, race_name[selSubClass-1] );
				strcpy( palname2, filename2);
				strcat( palname2, pal_extension);
				strcat( filename2, std_extension );
			}
#endif
			break;

		case ENCYC_WEAPONS:
			strcpy( filename, DIR_ENCYC );
			strcat( filename, "UNIT\\" );
			strcat( filename, weapon_name[selSubClass-1] );
			strcpy( palname, filename);
			strcat( palname, pal_extension);
			strcat( filename, std_extension );
#ifdef AMPLUS
			if( DIR_ENCYC2[0] )
			{
				strcpy( filename2, DIR_ENCYC2 );
				strcat( filename2, "UNIT\\" );
				strcat( filename2, weapon_name[selSubClass-1] );
				strcpy( palname2, filename2);
				strcat( palname2, pal_extension);
				strcat( filename2, std_extension );
			}
#endif
			break;

		case ENCYC_SHIPS:
			strcpy( filename, DIR_ENCYC );
			strcat( filename, "UNIT\\" );
			strcat( filename, ship_name[selSubClass-1] );
			strcpy( palname, filename);
			strcat( palname, pal_extension);
			strcat( filename, std_extension );
#ifdef AMPLUS
			if( DIR_ENCYC2[0] )
			{
				strcpy( filename2, DIR_ENCYC2 );
				strcat( filename2, "UNIT\\" );
				strcat( filename2, ship_name[selSubClass-1] );
				strcpy( palname2, filename2);
				strcat( palname2, pal_extension);
				strcat( filename2, std_extension );
			}
#endif
			break;

		case ENCYC_STRUCTURES:
			strcpy( filename, DIR_ENCYC );
			strcat( filename, "FIRM\\" );
			strcat( filename, firm_name[selSubClass-1] );
			strcpy( palname, filename);
			strcat( palname, pal_extension);
			strcat( filename, std_extension );
#ifdef AMPLUS
			if( DIR_ENCYC2[0] )
			{
				strcpy( filename2, DIR_ENCYC2 );
				strcat( filename2, "FIRM\\" );
				strcat( filename2, firm_name[selSubClass-1] );
				strcpy( palname2, filename2);
				strcat( palname2, pal_extension);
				strcat( filename2, std_extension );
			}
#endif
			break;

		case ENCYC_PALACE:
			strcpy( filename, DIR_ENCYC );
			strcat( filename, "SEAT\\" );
			strcat( filename, race_name[selSubClass-1] );
			strcpy( palname, filename);
			strcat( palname, pal_extension);
			strcat( filename, std_extension );
#ifdef AMPLUS
			if( DIR_ENCYC2[0] )
			{
				strcpy( filename2, DIR_ENCYC2 );
				strcat( filename2, "SEAT\\" );
				strcat( filename2, race_name[selSubClass-1] );
				strcpy( palname2, filename2);
				strcat( palname2, pal_extension);
				strcat( filename2, std_extension );
			}
#endif
			break;

		case ENCYC_GOD:
			strcpy( filename, DIR_ENCYC );
			strcat( filename, "GOD\\" );
			strcat( filename, god_name[selSubClass-1] );
			strcpy( palname, filename);
			strcat( palname, pal_extension);
			strcat( filename, std_extension );
#ifdef AMPLUS
			if( DIR_ENCYC2[0] )
			{
				strcpy( filename2, DIR_ENCYC2 );
				strcat( filename2, "GOD\\" );
				strcat( filename2, god_name[selSubClass-1] );
				strcpy( palname2, filename2);
				strcat( palname2, pal_extension);
				strcat( filename2, std_extension );
			}
#endif
			break;

		case ENCYC_FRYHTANS:
			strcpy( filename, DIR_ENCYC );
			strcat( filename, "MONSTER\\" );
			strcat( filename, monster_name[selSubClass-1] );
			strcpy( palname, filename);
			strcat( palname, pal_extension);
			strcat( filename, std_extension );
#ifdef AMPLUS
			if( DIR_ENCYC2[0] )
			{
				strcpy( filename2, DIR_ENCYC2 );
				strcat( filename2, "MONSTER\\" );
				strcat( filename2, monster_name[selSubClass-1] );
				strcpy( palname2, filename2);
				strcat( palname2, pal_extension);
				strcat( filename2, std_extension );
			}
#endif
			break;

		default:
			err_here();
			return 0;
	}

	File pictFile;
	char *palNamePtr = NULL;

	if( 
#ifdef AMPLUS
		// search DIR_ENCYC2 first
		filename2[0] && m.is_file_exist(filename2) && pictFile.file_open(filename2,0) && (palNamePtr = palname2) ||
#endif
		filename[0] && m.is_file_exist(filename) && pictFile.file_open(filename,0) && (palNamePtr = palname) )
	{
		vga_back.put_large_bitmap(174, 12, &pictFile);

		if( !firstDisp )
			vga_front.bar(174,12,787,587, 0x00);		// wipe the picture screen

		if( palNamePtr && m.is_file_exist(palNamePtr) )
		{
			VgaFrontLock vgaLock;
			VgaCustomPalette::set_custom_palette(palNamePtr);
		}

		if( !firstDisp )
			vga.blt_buf(174,12,787,587, 0);

		return 1;
	}

	return 0;
}

//-------- End of static function disp_picture ---------//

