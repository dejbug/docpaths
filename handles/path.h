#ifndef _PATH_H_
#define _PATH_H_

#include <windows.h>

#include "typ.h"
#include "ex.h"

NAMESPACE_BEGIN(path)

DEFINE_EXCEPTION(std::runtime_error, PathError)


struct SplitPath {

	SplitPath(char const * fullpath);
	virtual ~SplitPath();

	char const * GetFullPath() const {
		return fullpath;
	}
	
	char const * GetPath() const {
		return path;
	}
	
	char const * GetName() const {
		return name;
	}
	
	char const * GetExt() const {
		return ext;
	}

private:
	char * fullpath, * path, * name, * ext;
};


struct FileInfo {

	FileInfo(char const * path);
	virtual ~FileInfo();

	bool Exists() const {
		return exists;
	}

	size_t GetSize() const {
		return size;
	}
	
	unsigned long GetCreationTime() const {
		return ctime;
	}
	
	unsigned long GetLastAccessTime() const {
		return atime;
	}

private:
	bool exists;
	size_t size;
	unsigned long ctime, atime;
};


struct DosPathTranslator {

	DosPathTranslator();

	void Print() const;
	bool Translate(typ::string & path);

	void EnumerateDrives();
	void QueryMapping();

protected:
	typ::strings drives;
	typ::stringmap drivemap;
};


struct FileFinder {

	FileFinder(char const * pattern);
	virtual ~FileFinder();

	bool Next();
	char const * GetFileName() const;

protected:
	HANDLE handle;
	WIN32_FIND_DATA data;
	typ::string search;
};


template <size_t MAX_LINE>
struct LineReader {

	LineReader(FILE * handle) : handle(handle) {
	}

	std::string GetLine() const {
		return std::string(buffer);
	}
	
	bool ReadNextLine() {
		if(feof(handle)) {
			buffer[0] = '\0';
			return false;
		}
		
		int offset = 0;
		
		for(int c=fgetc(handle); EOF != c; c=fgetc(handle)) {
			if('\r' == c) continue;
			if('\n' == c || offset >= MAX_LINE)
				break;
			buffer[offset++] = c;
		}
		
		buffer[offset] = '\0';
		
		return true;
	}

private:
	char buffer[MAX_LINE + 1];
	FILE * handle;
};


size_t GetFileSize(char const * path);

DWORD GetDeviceBusTypeId(LPCSTR szPath);
LPCSTR GetBusTypeString(DWORD busTypeId);
DWORD GetDeviceBusTypeIdByDriveLetter(CHAR driveLetter);
DWORD GetDeviceBusTypeIdByPhysicalDiskPath(LPCSTR physicalPath);
DWORD GetDeviceBusTypeIdByVolumeGUIDString(LPCSTR guidString);


NAMESPACE_END

#endif
