#ifndef _HANDLES_H_
#define _HANDLES_H_

#include <windows.h>
#include <memory>
#include <algorithm>
#include <stdio.h>

#include "typ.h"
#include "ex.h"
#include "nt.h"
#include "sys.h"
#include "path.h"
#include "text.h"

namespace handles {

DEFINE_EXCEPTION(ex::Error, HandlesError)
DEFINE_EXCEPTION(HandlesError, QueryError)

struct ProcessIdFinder{
	using PID = unsigned long;
	using PIDNAME = std::pair<PID, std::string>;
	using PIDNAMES = std::vector<PIDNAME>;

	ProcessIdFinder();

	void Find(PIDNAMES & pidnames, char const * text);
	virtual bool Matches(char const * p, int plen,
		char const * s, int slen);

private:
	ULONG GetProcessInformationMinimalOutputBufferSize();

protected:
	NTSTATUS result;
	std::unique_ptr<char> buffer;
};


struct RemoteHandleLocalizer {

	RemoteHandleLocalizer();
	virtual ~RemoteHandleLocalizer();

	HANDLE Localize(nt::SYSTEM_HANDLE_ENTRY const & h);
	HANDLE Localize(ULONG ownerPid, USHORT handleValue);
	HANDLE Localize(ULONG ownerPid, HANDLE remoteHandle);

protected:
	HANDLE myProcess;
	DWORD myPid;
};


struct FileHandlePathGetter {

	FileHandlePathGetter();

	void GetPath(std::string & path,
		nt::SYSTEM_HANDLE_ENTRY const & h);
	
	void GetPath(std::string & path, HANDLE handle,
		bool closeHandle=false);

protected:
	sys::Dll ntdll;
	nt::NtQueryObjectType _QueryObject;
};


struct HandleInformationPrinter {

	HandleInformationPrinter();

	void Print(nt::SYSTEM_HANDLE_ENTRY const & h);

protected:
	RemoteHandleLocalizer rhl;
	sys::Dll ntdll;
	nt::NtQueryObjectType _QueryObject;
};


struct HandleEnumerator {
	using DATA = std::map<unsigned long, std::string>;

	HandleEnumerator(size_t initialBufferSize=1000000);

	void GetFilePaths(typ::strings & paths);
	void GetFilePathsForProcess(typ::strings & paths);
	void GetFilePathsForProcess(typ::strings & paths,
		unsigned long pid);
	void GetFilePathsForProcess(typ::strings & paths,
		char const * name);

private:
	void EnumerateHandles();

protected:
	unsigned long GetHandleEntriesCount() const;
	nt::SYSTEM_HANDLE_ENTRY * GetHandleEntries() const;

protected:
	NTSTATUS result;
	sys::Dll ntdll;
	size_t size;
	std::unique_ptr<char> buffer;
	DATA data;
};


void PrintHandleInformation(nt::SYSTEM_HANDLE_ENTRY const & h);

void FilterPaths(typ::strings & paths,
	char const * ext);

void FilterPaths(typ::strings & out,
	typ::strings const & paths,
	char const * ext);

bool ListHandlesForProcess(char const * name,
	char const * ext=NULL);

void GetHandlesForProcess(typ::strings & paths,
	char const * name,
	char const * ext=NULL);

}

#endif
