/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2017 Jesse Allen
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

// Filename    : WebService.h
// Description : A curl implementation to access web services


#ifndef __WEBSERVICE_H
#define __WEBSERVICE_H

#include <string>
#include <curl/curl.h>

class WebService
{
private:
	bool init_flag;
	CURL *curl;
	std::string buffer;

public:

	WebService();
	~WebService();

	void init();
	void deinit();
	int refresh(char *user);
	int login(char *user, char *pass);
};

extern WebService ws;

#endif
