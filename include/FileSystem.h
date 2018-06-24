#pragma once


// Miscellaneous file/directory handling functions.
class FileSystem
{
public:
	static const char* get_current_directory();
	static bool set_current_directory(const char*);
	static bool is_file_exist(const char*);
	static bool path_cat(char *dest, const char *src1, const char *src2, int max_len);
	static int mkpath(char *abs_path);
	static void change_file_ext(char* dest, const char* src, const char* newExt);
	static const char* get_file_name(const char*);

	// Returns the configuration/home path, if available, terminated with a path separator, or else an empty string (i.e. the current working directory).
	static const char* get_home_directory(const char* organisationName, const char* applicationName);

	static const char PATH_DELIMITER;
};
