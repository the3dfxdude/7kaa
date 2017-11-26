#pragma once


// Miscellaneous file/directory handling functions.
class FileSystem
{
public:
	static const char* get_current_directory();
	static bool set_current_directory(const char*);
	static bool is_file_exist(const char*);
	static int mkpath(char *abs_path);
	static void change_file_ext(char* dest, const char* src, const char* newExt);
	static const char* get_file_name(const char*);

	static const char PATH_DELIMITER;
};
