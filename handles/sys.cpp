#include "sys.h"

sys::Dll::Dll(char const * path) : handle(nullptr) {
	handle = LoadLibrary(path);
	if(!handle)
		throw LoadError("Dll: unable to load library");
}

sys::Dll::~Dll() {
	if(handle) {
		FreeLibrary(handle);
		handle = nullptr;
	}
}

sys::Timestamp::Timestamp(unsigned long date, char const * format) {
	tm * temp = localtime((time_t *)&date);
	memcpy(&t, temp, sizeof(tm));
	
	if(format) {
		s.reserve(64);
		strftime(const_cast<char*>(s.data()),
			s.capacity(), format, &t);
	}
}

char const * sys::Timestamp::GetString() const {
	return s.c_str();
}

void sys::JoinStrings(typ::string & out,
		typ::strings const & strings, char const * delim) {

	out = "";
	for(typ::strings::const_iterator it=strings.begin();
			it != strings.end(); ++it) {

		if(it != strings.begin())
			out += delim;
		out += it->data();
	}
}
