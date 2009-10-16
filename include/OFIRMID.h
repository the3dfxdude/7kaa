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

//Filename    : OFIRMID.H
//Description : Identity no. of all firm types

#ifndef __OFIRMID_H
#define __OFIRMID_H

//--------- Define no. of firms ------------//

enum { MAX_FIRM_TYPE = 10 };

//--- Define the firm id no. according to the order in FIRM.DBF ---//

enum { FIRM_BASE=1,
		 FIRM_FACTORY,
		 FIRM_INN,
		 FIRM_MARKET,
		 FIRM_CAMP,
		 FIRM_MINE,
		 FIRM_RESEARCH,
		 FIRM_WAR_FACTORY,
		 FIRM_HARBOR,
		 FIRM_MONSTER,
	  };

//------------------------------------------//

#endif
