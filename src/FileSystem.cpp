#include <FileSystem.h>
#include <OMISC.h>
#include <OERROR.h>
#include <FilePath.h>
#include <string.h>

#ifdef USE_WINDOWS
#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#define chdir _chdir
#define unlink _unlink
#elif defined (USE_POSIX)
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif


bool FileSystem::set_current_directory(const char* directory) {
	return chdir(directory) == 0;
}

//------- Begin of function FileSystem::is_file_exist ---------//
//
// Check whether the given file exists in the current directory or not
//
// <const char*> fileName = the name of the file
//
// return : true  - the file exists
//          false - doesn't exist
//
bool FileSystem::is_file_exist(const char* fileName)
{
#ifdef USE_WINDOWS
	DWORD attrib = GetFileAttributes(fileName);
	return (attrib != INVALID_FILE_ATTRIBUTES) && !(attrib & FILE_ATTRIBUTE_DIRECTORY);
#elif defined (USE_POSIX)
	return !access (fileName, F_OK);
#endif
	err.msg ("FileSystem::is_file_exist: stub");
	return false;
}
//---------- End of function FileSystem::is_file_exist ---------//


bool FileSystem::delete_file(const char* file) {
	return unlink(file) == 0;
}


namespace {
	// mkpath_mkdir -- helper function to mkpath
	int mkpath_mkdir(char* path)
	{
	#ifdef USE_WINDOWS
		if (!path[2] && path[1] == ':' && isalpha(path[0]))
		{
			// don't try to make a drive letter path
			// this actually works on windows, but not on Wine
			return 1;
		}
		return !CreateDirectory(path, NULL) ?
			GetLastError() == ERROR_ALREADY_EXISTS : 1;
	#else
		return mkdir(path, 0777) == -1 ? errno == EEXIST : 1;
	#endif
	}
}


//------- Begin of function FileSystem::mkpath ---------//
// Given an absolute path to a directory, create the
// directory, and all intermediate directories if
// necessary.
bool FileSystem::mkpath(char *abs_path)
{
	char path_copy[FilePath::MAX_FILE_PATH];
	int count;

	if( strlen(abs_path) >= FilePath::MAX_FILE_PATH )
		return false;

	count = 0;
	while (count < FilePath::MAX_FILE_PATH) {
		if (!abs_path[count]) {
			if (count > 0) {
				path_copy[count] = 0;
				if (!mkpath_mkdir(path_copy))
					return false;
			}
			return true;
		} else if (abs_path[count] == PATH_DELIM[0] && count > 0) {
			path_copy[count] = 0;
			if (!mkpath_mkdir(path_copy))
				return false;
		}

		path_copy[count] = abs_path[count];
		count++;
	}
	return true;
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
void FileSystem::extract_file_name(char* desFileName, const char* srcFileName)
{
	int i;

	for( i=strlen(srcFileName); i>=0 ; i-- )
	{
		if( srcFileName[i]==PATH_DELIM[0] )			// get last '\' before the file name
			break;
	}

	const char *p = srcFileName+i+1;
	size_t fileNameLen = strlen(p);
	if( fileNameLen >= FilePath::MAX_FILE_PATH )
	{
		desFileName[0] = 0;
		return;
	}

	strncpy(desFileName, p, FilePath::MAX_FILE_PATH);
}
//---------- End of function FileSystem::extract_file_name ---------//
