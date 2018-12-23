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

//Filename    : GAMEDEF.H
//Description : Maximum game constant

#ifndef __GAMEDEF_H
#define __GAMEDEF_H

//----------- define game version ------------//

//#define GAME_VERSION_STR   "1.1"
extern const char *GAME_VERSION_STR;
//#define GAME_VERSION       110          // Version 1.00, don't change it unless the format of save game files has been changed
extern const int GAME_VERSION;

//-------- Define constant ------------//

#define APP_NAME           "SK"                         // application name
#define WIN_CLASS_NAME     "Seven_Kingdoms"             // for registering window class
#define WIN_TITLE          "Seven Kingdoms"             // window title

//------- Data sub-directories -------//

#ifdef USE_WINDOWS
#define PATH_DELIM "\\"
#else
#define PATH_DELIM "/"
#endif

// config dir
#define CONFIG_ORGANIZATION_NAME "7kfans.com"
#define CONFIG_APPLICATION_NAME "7kaa"

// top level data dirs
#define DEFAULT_DIR_RES      "RESOURCE" PATH_DELIM
#define DEFAULT_DIR_SPRITE   "SPRITE" PATH_DELIM
#define DEFAULT_DIR_SOUND    "SOUND" PATH_DELIM
#define DEFAULT_DIR_IMAGE    "IMAGE" PATH_DELIM
#define DEFAULT_DIR_ENCYC    "ENCYC" PATH_DELIM
#define DEFAULT_DIR_ENCYC2   "ENCYC2" PATH_DELIM
#define DEFAULT_DIR_MOVIE    "MOVIE" PATH_DELIM
#define DEFAULT_DIR_MUSIC    "MUSIC" PATH_DELIM
#define DEFAULT_DIR_TUTORIAL "TUTORIAL" PATH_DELIM
#define DEFAULT_DIR_SCENARIO "SCENARIO" PATH_DELIM
#define DEFAULT_DIR_SCENARI2 "SCENARI2" PATH_DELIM

// encylopedia chapter dirs
#define DEFAULT_DIR_ENCYC_UNIT    "UNIT" PATH_DELIM
#define DEFAULT_DIR_ENCYC_FIRM    "FIRM" PATH_DELIM
#define DEFAULT_DIR_ENCYC_SEAT    "SEAT" PATH_DELIM
#define DEFAULT_DIR_ENCYC_GOD     "GOD" PATH_DELIM
#define DEFAULT_DIR_ENCYC_MONSTER "MONSTER" PATH_DELIM

#define DIR_RES         DEFAULT_DIR_RES
#define DIR_SPRITE      DEFAULT_DIR_SPRITE
#define DIR_SOUND       DEFAULT_DIR_SOUND
#define DIR_IMAGE       sys.dir_image
#define DIR_ENCYC       sys.dir_encyc
#define DIR_ENCYC2	sys.dir_encyc2
#define DIR_MUSIC       sys.dir_music
#define DIR_MOVIE       sys.dir_movie
#define DIR_TUTORIAL		sys.dir_tutorial
#define DIR_SCENARIO		sys.dir_scenario
// see MAX_SCENARIO_PATH in sys
#define DIR_SCENARIO_PATH(p)	sys.dir_scenario_path[p]

//--------- Define direction types -----------//

enum { MAX_DIR=8 };

enum { DIR_N,     // building directions
       DIR_NE,
       DIR_E,
       DIR_SE,
       DIR_S,
       DIR_SW,
       DIR_W,
       DIR_NW
     };

//---------- Link enable/disable status ---------//

#define LINK_DD   0     // both sides disabled
#define LINK_ED   1     // host side enabled, client side disabled
#define LINK_DE   2     // host side disabled, client side enabled
#define LINK_EE   3     // both sides enabled

//------------- define command types ------------//

enum { COMMAND_PLAYER=0, COMMAND_REMOTE=1, COMMAND_AI, COMMAND_AUTO };

//-------- Define maximum game constant ---------//

#define MAX_RACE                  10

#define MAX_COLOR_SCHEME             7    // Maximum no. of color schemes

#define MAX_NATION                   MAX_COLOR_SCHEME // Maximum no. of nation should be equal to the maximum no. of color scheme-1 (the exclude one is for independent nations)

#define MAX_UNIT_ATTACK_TYPE        3

#define MAX_SPRITE_DIR_TYPE         8

#define MAX_LINKED_FIRM_FIRM        23
#define MAX_LINKED_FIRM_TOWN        30
#define MAX_LINKED_TOWN_TOWN        18

#define PROCESS_GOODS_INTERVAL      3     // Process goods in mines, factories and market places once 3 days

#define PEASANT_FOOD_YEAR_PRODUCTION  30  // A peasant produce 2 units of food per month
#define PERSON_FOOD_YEAR_CONSUMPTION  10  // consume one unit of food per person per month

#define NO_FOOD_LOYALTY_DECREASE_INTERVAL	 5  // When your kingdom runs out of food, the loyalty of all your units will decrease 1 point every 5 days.

#define RAW_PRICE       1           // Cost per raw material
#define PRODUCT_PRICE   2           // Cost per product
#define CONSUMER_PRICE  4           // Cost per product for consumers

#define REWARD_COST                 30
#define REWARD_LOYALTY_INCREASE     10

#define UNIT_BETRAY_LOYALTY         30 // if a unit's loyalty is below this, it will betray

#define PROMOTE_LOYALTY_INCREASE    20
#define DEMOTE_LOYALTY_DECREASE     40

#define ATTACK_SLOW_DOWN            4  // how many times attacking damages should be reduced, this lessens all attacking damages, thus slowing down all battles.

#define SPY_KILLED_REPUTATION_DECREASE		3

//-----------------------------------------------//

#endif
