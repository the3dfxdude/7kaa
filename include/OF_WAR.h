//Filename    : OF_WAR.H
//Description : Header of FirmWar war factory.

#ifndef __OF_WAR_H
#define __OF_WAR_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//-------- Define constant ---------//

#define MAX_BUILD_QUEUE 	20

//------- Define class FirmWar --------//

class FirmWar : public Firm
{
public:
	short build_unit_id;
	DWORD last_process_build_frame_no;
	float build_progress_days;

	char  build_queue_array[MAX_BUILD_QUEUE];		// it stores the unit id.
	char  build_queue_count;

	int	is_operating()		{ return productivity > 0 && build_unit_id; }

public:
	FirmWar();
	~FirmWar();

	void 	put_info(int refreshFlag);
	void 	detect_info();

	void 	disp_main_menu(int refreshFlag);
	void 	detect_main_menu();

	void 	disp_build_menu(int refreshFlag);
	void 	detect_build_menu();

	void	next_day();
	void	process_ai();

	virtual	FirmWar* cast_to_FirmWar() { return this; };
	void	add_queue(int unitId);
	void	remove_queue(int unitId);
	void	cancel_build_unit();

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

private:
	void 	disp_war_info(int dispY1, int refreshFlag);
	void  disp_build_button(int y, int unitId, int buttonUp);
	void  disp_queue_button(int y, int unitId, int buttonUp);

	void 	process_build();
	void	process_queue();

	//-------- AI actions ---------//

	void	think_new_production();
	int 	should_build_new_weapon();
	int	think_del();
};

//--------------------------------------//

#endif
