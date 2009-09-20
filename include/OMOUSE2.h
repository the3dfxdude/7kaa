// Filename    : OMOUSE2.H
// Description : constant used in Mouse::is_key

//------ flags used in is_key ---------//

#ifndef __OMOUSE2_H
#define __OMOUSE2_H

enum 
{
	K_CAPITAL_LETTER      = 0x0001,
	K_IS_SHIFT            = 0x0002,
	K_IS_CTRL             = 0x0004,
	K_IS_ALT              = 0x0008,
	K_ON_NUMPAD           = 0x0010,

	K_CASE_INSENSITIVE    = 0x0100,
	K_IGNORE_SHIFT        = 0x0200,
	K_IGNORE_CTRL         = 0x0400,
	K_IGNORE_ALT          = 0x0800,
	K_IGNORE_NUM_LOCK     = 0x1000,
	K_IGNORE_CAP_LOCK     = 0x2000,
	K_IGNORE_NUMPAD       = 0x4000,
	K_TRANSLATE_KEY       = 0x8000,

	// oftenly used composite
	K_CHAR_KEY = K_IGNORE_NUMPAD | K_IGNORE_SHIFT | K_IGNORE_CAP_LOCK | K_TRANSLATE_KEY,
	K_UNIQUE_KEY = K_IGNORE_SHIFT | K_IGNORE_CTRL
		| K_IGNORE_ALT | K_IGNORE_NUM_LOCK | K_IGNORE_CAP_LOCK,
};

#endif
