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

//Filename    : OF_RESE.H
//Description : Header of FirmResearch

#ifndef __OF_RESE_H
#define __OF_RESE_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif


//------- Define class FirmResearch --------//

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

	virtual void accept_file_visitor(FileReaderVisitor* v) override;
	virtual void accept_file_visitor(FileWriterVisitor* v) override;

	virtual FirmResearch* cast_to_FirmResearch() { return this; }

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();

private:
	void	disp_research_info(int dispY1, int refreshFlag);
	void 	disp_research_button(int y, int techId, int buttonUp);

	//-------- AI actions ---------//

	void		think_new_research();
	int		think_del();
};

//--------------------------------------//

#endif
