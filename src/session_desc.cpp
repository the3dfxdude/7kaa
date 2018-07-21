/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2016 Jesse Allen
* Copyright 2018 Richard Dijk <microvirus.multiplying@gmail.com>
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


#include <session_desc.h>
#include <OMISC.h>
#include <GAMEDEF.h>
#include <cstring>


SessionDesc::SessionDesc(const char* session_name, const guuid_t& session_id, uint32_t flags, const ENetAddress& address)
	: /*session_name*/
	  password{},
	  /*session_id*/
	  flags(flags),
	  max_players(MAX_NATION),
	  player_count(1),
	  address(address)
{
	std::strncpy(this->session_name, session_name, MP_FRIENDLY_NAME_LEN);
	this->session_name[MP_FRIENDLY_NAME_LEN] = 0;
	misc.uuid_copy(this->session_id, session_id);
}
