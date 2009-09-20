// Filename    : OGAMCRED.CPP
// Description : viewing credits

#include <OSYS.h>
#include <OVGA.h>
#include <OVGALOCK.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OMUSIC.h>
#include <OIMGRES.h>
#include <OGAME.h>

//------ Declare static functions --------//

/*
static void disp_credits_2();
static void disp_credits_3();
static void disp_credits_4();
static void disp_credits_5();
static void disp_credits(int y, char** creditNameArray);
*/

//------ Begin of function Game::view_credits ------//

void Game::view_credits()
{
	//----- display the first page ------//

	vga.disp_image_file("CREDITS1");

	if( mouse.wait_press(60)==2 )		// return 2 if pressed ESC or right mouse click
	{
		vga.finish_disp_image_file();
		return;								// 60 seconds to time out
	}

	//------ display the 2nd page -----//

	vga.disp_image_file("CREDITS2");

	if( mouse.wait_press(60)==2 )		// return 2 if pressed ESC or right mouse click
	{
		vga.finish_disp_image_file();
		return;
	}

	//------ display the 3rd page -----//

	vga.disp_image_file("CREDITS3");

	if( mouse.wait_press(60)==2 )		// return 2 if pressed ESC or right mouse click
	{
		vga.finish_disp_image_file();
		return;
	}

	//------ display the 4th page -----//

	vga.disp_image_file("CREDITS4");

	if( mouse.wait_press(60)==2 )		// return 2 if pressed ESC or right mouse click
	{
		vga.finish_disp_image_file();
		return;								// 60 seconds to time out
	}

#if(defined(FRENCH))
	//------ display the 5th page -----//

	vga.disp_image_file("CREDITS5");

	if( mouse.wait_press(60)==2 )		// return 2 if pressed ESC or right mouse click
	{
		vga.finish_disp_image_file();
		return;								// 60 seconds to time out
	}
#endif

	vga.finish_disp_image_file();
}
//------ End of function Game::view_credits ------//

/*

//------ Begin of static function disp_credits_2 ------//

static void disp_credits_2()
{
	static char* credit_des_array[] =
	{
		"Executive Producers",
		"Project Manager",
		"Quality Manager",
		"Play Test Coordinator",
		"Manual Editing",
		"Manual Editing and Layout",
		"Product Marketing Manager",
		NULL
	};

	static char* credit_name_array[] =
	{
		"Ray Rutledge and Joe Rutledge",
		"Steve Wartofsky",
		"David Green",
		"Brain K. Davis VII",
		"Arnold Hendrick",
		"Sarah O'Keefe and Alan Pringle von 'Scriptorium Publishing Services, Inc.",
		"Angela Lipscomb",
	};

	//----------------------------------//

	int y=100;

	font_bible.center_put( 0, y, VGA_WIDTH-1, y+font_bible.height()-1, "Interactive Magic" );
	y+=font_bible.height()+20;

	for( int i=0 ; credit_des_array[i] ; i++ )
	{
		font_bible.center_put( 0, y, VGA_WIDTH-1, y+font_bible.height()-1, credit_des_array[i] );
		y+=font_bible.height()+3;

		font_bible.center_put( 0, y, VGA_WIDTH-1, y+font_bible.height()-1, credit_name_array[i] );
		y+=font_bible.height()+16;
	}
}
//------ End of static function disp_credits_2 ------//


//------ Begin of static function disp_credits_3 ------//

static void disp_credits_3()
{
	static char* credit_name_array[] =
	{
		"Joe Allen",
		"Ismini Boinodiris",
		"James Cowgill",
		"Chris Gardner",
		"Carlin Gartrell",
		"Anthony Lazaro",
		"Mike Metrosky",
		"Mike Pearson",
		"Marc Racine",
		"Jason Sircy",
		"Adam Turner",
		"Ted Wagoner",
		"Greg Young",
		NULL
	};

	//----------------------------------//

	int y=100;

	font_bible.center_put( 0, y, VGA_WIDTH-1, y+font_bible.height()-1,
		"Internal Beta-Testers" );

	disp_credits(y, credit_name_array);
}
//------ End of static function disp_credits_3 ------//


//------ Begin of static function disp_credits_4 ------//

static void disp_credits_4()
{
	static char* credit_name_array[] =
	{
		"Richard Arnesen"
		"JP Bernard",
		"Bryan Caldwell",
		"Kent Coleman",
		"Sorin Cristescu",
		"Al Demauro",
		"Troy Denkinger",
		"Chris Edwards",
		"Drew Fudenberg",
		"Michael Garrett",
		"Raymond Graham",
		"Tom Harlin",
		"Leonard Hemsen",
		"Chris Hepner",
		"Tom Hepner",
		"Ben Herd",
		"Benjamin Van Hoeson",
		"Allen Holland",
		"Brian Lander",
		"Steve Lieb",
		"mark Logsdon",
		"Crist-Jan Mannien",
		"David Newman",
		"Tomi J Nissinen",
		"Sven Johansson",
		"Tim Jordan",
		"Greg Ottoman",
		"Ralf Papen",
		"Jim Pedicord",
		"Gaspar Peixoto",
		"David Poythress",
		"Louis Rhodes",
		"Dean Robb",
		"Anthony Sage",
		"Todd Strobl",
		"Bjorn Tidal",
		"Ron Williams",
		"Christopher Yoder",
		NULL
	};

	//----------------------------------//

	int y=260;

	font_bible.center_put( 0, y, VGA_WIDTH-1, y+font_bible.height()-1,
		"External Beta-Testers" );

	disp_credits(y, credit_name_array);
}
//------ End of static function disp_credits_4 ------//


//------ Begin of static function disp_credits ------//

static void disp_credits(int y, char** creditNameArray)
{
	y+=font_bible.height()+13;

	String str;

	for( int i=0 ; creditNameArray[i] ; )
	{
		str = creditNameArray[i];
		i++;

		if( creditNameArray[i] )
		{
			str += ", ";
			str += creditNameArray[i];
			i++;

			if( creditNameArray[i] )
			{
				str += ", ";
				str += creditNameArray[i];
				i++;

				if( creditNameArray[i] )
				{
					str += ", ";
					str += creditNameArray[i];
					i++;

					if( creditNameArray[i] )
						str += ",";
				}
			}
		}

		font_bible.center_put( 0, y, VGA_WIDTH-1, y+font_bible.height()-1, str );
		y+=font_bible.height()+3;
	}
}
//------ End of static function disp_credits ------//

*/