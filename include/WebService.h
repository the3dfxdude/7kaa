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
	void refresh(char *user);
	void login(char *user, char *pass);
};

extern WebService ws;
