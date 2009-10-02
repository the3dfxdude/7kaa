//Filename    : OF_RESE.H
//Description : Header of FirmResearch

#ifndef __OF_RESE_H
#define __OF_RESE_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif


//------- Define class FirmResearch --------//

#pragma pack(1)
class FirmResearch : public Firm
{
public:
	short tech_id;				   // the id. of the tech this firm is currently researching
	float complete_percent;		// percent completed on researching the current technology

	int	is_operating()		{ return productivity > 0 && tech_id; }

public:
	FirmResearch();
	~FirmResearch();

	void  init_derived();

	void 	put_info(int refreshFlag);
	void 	detect_info();

	void 	disp_main_menu(int refreshFlag);
	void 	detect_main_menu();

	void 	disp_research_menu(int refreshFlag);
	void 	detect_research_menu();

	void 	start_research(int techId, char remoteAction);
	void 	process_research();
	void 	terminate_research();
	void  research_complete();

	void	next_day();
	void	process_ai();

	void 	change_nation(int newNationRecno);

	virtual FirmResearch* cast_to_FirmResearch() { return this; };

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

private:
	void	disp_research_info(int dispY1, int refreshFlag);
	void 	disp_research_button(int y, int techId, int buttonUp);

	//-------- AI actions ---------//

	void		think_new_research();
	int		think_del();
};
#pragma pack()

//--------------------------------------//

#endif
