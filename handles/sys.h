#ifndef _SYS_H_
#define _SYS_H_

#include <windows.h>
#include <time.h>
#include <string>
#include <vector>

#include "typ.h"
#include "ex.h"

NAMESPACE_BEGIN(sys)

struct Dll {
	DEFINE_EXCEPTION(ex::Error, LoadError)
	DEFINE_EXCEPTION(ex::Error, FindError)

	HMODULE handle;

	Dll(char const * path);
	virtual ~Dll();

	template<class T>
	T find(char const * name) const {
		FARPROC func = GetProcAddress(handle, name);
		if(!func)
			throw FindError("Dll: no such proc in library");
		return reinterpret_cast<T>(func);
	}
};

struct Timestamp {
	tm t;
	std::string s;

	Timestamp(unsigned long date, char const * format=NULL);

	char const * GetString() const;
};

void JoinStrings(typ::string & out,
	typ::strings const & strings, char const * delim="\r\n");


NAMESPACE_END

#endif
