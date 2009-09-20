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
