#pragma once


// Miscellaneous file/directory handling functions.
class FileSystem
{
public:
	static bool set_current_directory(const char* directory);
	static bool is_file_exist(const char*);
	static bool delete_file(const char*);
	static bool mkpath(char *abs_path);
	static void change_file_ext(char* dest, const char* src, const char* newExt);
	static void extract_file_name(char* dest, const char* src);
};
