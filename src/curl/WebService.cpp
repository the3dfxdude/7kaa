#include <OSYS.h>
#include <WebService.h>

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

	std::string cookie_file = sys.dir_config;
	cookie_file += "cookies.txt";
	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_file.c_str());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);

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

void WebService::refresh(char *user)
{
	CURLcode res;

	curl_easy_setopt(curl, CURLOPT_POST, 0);
	curl_easy_setopt(curl, CURLOPT_URL, "https://7kfans.com/forums/index.php");
	buffer = "";

	res = curl_easy_perform(curl);
	if( res != CURLE_OK )
		return;

	return;
}

void WebService::login(char *user, char *pass)
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
	if( res != CURLE_OK )
		return;

	return;
}
