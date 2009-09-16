//Filename    : OF_BASE.H
//Description : Header of FirmBase

#ifndef __OF_BASE_H
#define __OF_BASE_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

#ifndef __OSKILL_H
#include <OSKILL.h>
#endif

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//------- Define constant -----------//

#define  	MAX_BASE_PRAYER		30
#define		MAX_PRAY_POINTS		400

//------- Define class FirmBase --------//

class FirmBase : public Firm
{
public:
	short		god_id;
	short  	god_unit_recno;		// unit recno of the summoned god

	float		pray_points;

public:
	FirmBase();
	~FirmBase();

	void		init_derived();

	void 		assign_unit(int unitRecno);
	void 		assign_overseer(int overseerRecno);

	void 		change_nation(int newNationRecno);

	void 		put_info(int refreshFlag);
	void 		detect_info();

	void		next_day();
	void		process_ai();

	int		can_invoke();
	void		invoke_god();

	virtual	FirmBase* cast_to_FirmBase() { return this; };

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

private:
	void 		disp_base_info(int dispY1, int refreshFlag);
	void 		disp_god_info(int dispY1, int refreshFlag);

	void		train_unit();
	void 		recover_hit_point();

	//------------- AI actions --------------//

	void 		think_assign_unit();
	void 		think_invoke_god();
};

//--------------------------------------//

#endif
