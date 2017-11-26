#include <FileSystem.h>
#include <ODIR.h>
#include <string.h>

#ifdef NO_WINDOWS
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#else
#include <Windows.h>
#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd
#endif

const char FileSystem::PATH_DELIMITER = PATH_DELIM[0];


const char* FileSystem::get_current_directory()
{
	static char current_directory[MAX_PATH];
	if (getcwd(current_directory, MAX_PATH) == nullptr) {
		current_directory[0] = '\0';
	}
	return current_directory;
}

bool FileSystem::set_current_directory(const char* directory)
{
	return chdir(directory) == 0;
}


//------- Begin of function FileSystem::is_file_exist ---------//
//
// Check whether the given file exists in the current directory or not
//
// <char*> fileName = the name of the file
//
// return : <int> 1 - the file exists
//                0 - doesn't exist
//
bool FileSystem::is_file_exist(const char* fileName)
{
	Directory dir;
	return dir.read(fileName, 0) == 1;
}
//---------- End of function FileSystem::is_file_exist ---------//


namespace {
	// Attempts to perform string += concat, but ensures the string is zero-terminated whilst not exceeding the string capacity.
	// If targetWithinString is supplied, which should be a pointer within string, then writing starts at targetWithinString, else writing starts at string+strlen(string).
	// string must be properly intialised as a zero-terminated string, unless targetWithinString is supplied.
	// Returns a pointer to end of the concatenated string (pointing to the null termination character), or nullptr if target capacity was insufficient.
	char* concat_string(char* const string, const int targetCapacity, const char* const concat, char* const targetWithinString = nullptr)
	{
		const char* source = concat;
		char* target = targetWithinString ? targetWithinString : string + strlen(string);
		int usedCapacity = target - string;

		for ( ; *source != '\0' && usedCapacity < targetCapacity - 1; ++usedCapacity) {
			*target++ = *source++;
		}
		*target++ = '\0'; ++usedCapacity;

		return (*source == '\0' ? target - 1 : nullptr);
	}
}
//------- Begin of function FileSystem::path_cat ---------//
//
// Copies src1 to dest, then src2, ensuring the two strings together don't
// exceed max_len-1. The destination string is always null terminated.
//
// The return is 1 on success. If length of src1+src2 is greater than
// max_len-1, then the return is 0, and the dest string contents is null.
//
bool FileSystem::path_cat(char *dest, const char *src1, const char *src2, int max_len)
{
	if (max_len <= 0) return false;

	char* destPtr = dest;
	destPtr = concat_string(dest, max_len, src1, destPtr);
	// Insert path delimiter if neither or src1 and src2 supplies it and both are non-empty.
	if (destPtr && *src1 != '\0' && *src2 != '\0' && destPtr[-1] != PATH_DELIMITER && src2[0] != PATH_DELIMITER) {
		destPtr = concat_string(dest, max_len, PATH_DELIM, destPtr);
	}
	destPtr = concat_string(dest, max_len, src2, destPtr);
		
	if (!destPtr) {
		destPtr = dest;
		*destPtr = '\0';
		return false;
	}
	else {
		return true;
	}
}
//---------- End of function FileSystem::path_cat ---------//


// misc_mkdir -- helper function to mkpath
int misc_mkdir(char *path)
{
#ifdef NO_WINDOWS
	return mkdir(path, 0777) == -1 ? errno == EEXIST : 1;
#else // WINDOWS
	if (!path[2] && path[1] == ':' && isalpha(path[0]))
	{
		// don't try to make a drive letter path
		// this actually works on windows, but not on Wine
		return 1;
	}
	return !CreateDirectory(path, NULL) ?
		GetLastError() == ERROR_ALREADY_EXISTS : 1;
#endif
}


//------- Begin of function FileSystem::mkpath ---------//
// Given an absolute path to a directory, create the
// directory, and all intermediate directories if
// necessary.
int FileSystem::mkpath(char *abs_path)
{
	char path_copy[MAX_PATH+1];
	int count;

	count = 0;
	while (count < MAX_PATH) {
		if (!abs_path[count]) {
			if (count > 0) {
				path_copy[count] = 0;
				if (!misc_mkdir(path_copy))
					return 0;
			}
			return 1;
		} else if (abs_path[count] == PATH_DELIMITER && count > 0) {
			path_copy[count] = 0;
			if (!misc_mkdir(path_copy))
				return 0;
		}

		path_copy[count] = abs_path[count];
		count++;
	}
	return 0;
}
//-------- End of function FileSystem::mkpath ----------//


//------- Begin of function FileSystem::change_file_ext ---------//
//
// Change file extension.
//
// <char*> desFileName = the destination file name to be written
// <char*> srcFileName = the source file name
// <char*> newExt      = the new extension.
//
void FileSystem::change_file_ext(char* desFileName, const char* srcFileName, const char* newExt)
{
	int nameLen = misc.str_chr(srcFileName, '.');	// include the '.' in the nameLen

	err_when( nameLen<1 || nameLen>9 || strlen(newExt)>3 );

	memcpy( desFileName, srcFileName, nameLen );
	strcpy( desFileName+nameLen, newExt );        // extension for scenarion text file
}
//---------- End of function FileSystem::change_file_ext ---------//


//------- Begin of function FileSystem::extract_file_name ---------//
//
// Extract the file name from a full file path.
//
// <char*> desFileName = the destination buffer to be written
// <char*> srcFileName = the source file name
//
const char* FileSystem::get_file_name(const char* srcFileName)
{
	int i;
	for( i=strlen(srcFileName) ; i>=0 ; i-- )
	{
		if( srcFileName[i]=='\\' )			// get last '\' before the file name
			break;
	}

	return srcFileName+i+1;
}
//---------- End of function FileSystem::extract_file_name ---------//
