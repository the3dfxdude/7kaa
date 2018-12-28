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

// Filename    : WebService.cpp
// Description : A curl implementation to access web services

#include <OSYS.h>
#include <WebService.h>
#include <FilePath.h>

static size_t WriteMemoryCallback(char *contents, size_t size, size_t nmemb, std::string *buffer)
{
	buffer->append(contents, size*nmemb);
	return size*nmemb;
}

WebService::WebService()
{
	init_flag = 0;
}

WebService::~WebService()
{
	deinit();
}

void WebService::init()
{
	if( init_flag )
		return;
	if( curl_global_init(CURL_GLOBAL_DEFAULT) )
		return;
	curl = curl_easy_init();
	if( !curl )
		return;

	FilePath cookie_file(sys.dir_config);
	cookie_file += "cookies.txt";
	if( !cookie_file.error_flag )
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, (char*)cookie_file);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
	// Use a hard timeout at 10 seconds. This is not foolproof, but it is
	// somewhat necessary while we are using the blocking API.
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
#ifndef ENABLE_IPV6
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
#endif

	init_flag = 1;
}

void WebService::deinit()
{
	if( !init_flag )
		return;
	if( curl )
		curl_easy_cleanup(curl);
	curl_global_cleanup();
	init_flag = 0;
}

int WebService::refresh(char *user)
{
	CURLcode res;

	curl_easy_setopt(curl, CURLOPT_POST, 0);
	curl_easy_setopt(curl, CURLOPT_URL, "https://7kfans.com/forums/index.php");
	buffer = "";

	res = curl_easy_perform(curl);
	return res == CURLE_OK;
}

int WebService::login(char *user, char *pass)
{
	CURLcode res;

	/* the fields given by the user must be url encoded */
	std::string login = "login=Login&username=";
	char *encoded;

	encoded = curl_easy_escape(curl, user, 0);
	login += encoded;
	curl_free(encoded);
	login += "&password=";
	encoded = curl_easy_escape(curl, pass, 0);
	login += encoded;
	curl_free(encoded);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, login.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_URL, "https://7kfans.com/forums/ucp.php?mode=login");
	buffer = "";

	res = curl_easy_perform(curl);
	return res == CURLE_OK;
}
